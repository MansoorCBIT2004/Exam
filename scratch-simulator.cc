#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"
#include "ns3/seq-ts-header.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SelectiveArqExample");

// ---------------- Sender Application ----------------
class SelectiveSender : public Application {
public:
  SelectiveSender();
  virtual ~SelectiveSender();
  void Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t totalPackets, uint32_t windowSize, Time timeout);

private:
  virtual void StartApplication();
  virtual void StopApplication();
  void SendWindow();
  void SendPacket(uint32_t seq);
  void Timeout(uint32_t seq);
  void HandleAck(Ptr<Socket> socket);

  Ptr<Socket> m_socket;
  Address m_peerAddress;
  uint32_t m_packetSize;
  uint32_t m_totalPackets;
  uint32_t m_windowSize;
  Time m_timeout;
  std::map<uint32_t, EventId> m_timers;
  std::set<uint32_t> m_ackedPackets;
  uint32_t m_base;
};

SelectiveSender::SelectiveSender()
    : m_socket(0), m_packetSize(0), m_totalPackets(0), m_windowSize(0), m_base(0) {}

SelectiveSender::~SelectiveSender() {
  m_socket = 0;
}

void SelectiveSender::Setup(Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t totalPackets, uint32_t windowSize, Time timeout) {
  m_socket = socket;
  m_peerAddress = address;
  m_packetSize = packetSize;
  m_totalPackets = totalPackets;
  m_windowSize = windowSize;
  m_timeout = timeout;
}

void SelectiveSender::StartApplication() {
  m_socket->Connect(m_peerAddress);
  m_socket->SetRecvCallback(MakeCallback(&SelectiveSender::HandleAck, this));
  SendWindow();
}

void SelectiveSender::StopApplication() {
  if (m_socket) {
    m_socket->Close();
  }
  for (auto &t : m_timers) {
    Simulator::Cancel(t.second);
  }
}

void SelectiveSender::SendWindow() {
  for (uint32_t i = m_base; i < m_base + m_windowSize && i < m_totalPackets; ++i) {
    if (m_ackedPackets.find(i) == m_ackedPackets.end() && m_timers.find(i) == m_timers.end()) {
      SendPacket(i);
    }
  }
}

void SelectiveSender::SendPacket(uint32_t seq) {
  Ptr<Packet> packet = Create<Packet>(m_packetSize);

  SeqTsHeader seqHeader;
  seqHeader.SetSeq(seq);  // ✅ Correct usage
  packet->AddHeader(seqHeader);

  m_socket->Send(packet);
  NS_LOG_INFO("Sent packet " << seq);
  m_timers[seq] = Simulator::Schedule(m_timeout, &SelectiveSender::Timeout, this, seq);
}

void SelectiveSender::Timeout(uint32_t seq) {
  if (m_ackedPackets.find(seq) == m_ackedPackets.end()) {
    NS_LOG_INFO("Timeout for packet " << seq << ", retransmitting");
    SendPacket(seq);
  }
}

void SelectiveSender::HandleAck(Ptr<Socket> socket) {
  Ptr<Packet> packet = socket->Recv();

  SeqTsHeader seqHeader;
  packet->RemoveHeader(seqHeader);
  uint32_t ackSeq = seqHeader.GetSeq();

  NS_LOG_INFO("Received ACK for packet " << ackSeq);
  m_ackedPackets.insert(ackSeq);

  if (m_timers.find(ackSeq) != m_timers.end()) {
    Simulator::Cancel(m_timers[ackSeq]);
    m_timers.erase(ackSeq);
  }

  while (m_ackedPackets.find(m_base) != m_ackedPackets.end()) {
    m_base++;
  }

  SendWindow();
}

// ---------------- Receiver Application ----------------
class SelectiveReceiver : public Application {
public:
  SelectiveReceiver();
  virtual ~SelectiveReceiver();
  void Setup(Ptr<Socket> socket);

private:
  virtual void StartApplication();
  virtual void StopApplication();
  void HandleRead(Ptr<Socket> socket);

  Ptr<Socket> m_socket;
};

SelectiveReceiver::SelectiveReceiver() : m_socket(0) {}

SelectiveReceiver::~SelectiveReceiver() {
  m_socket = 0;
}

void SelectiveReceiver::Setup(Ptr<Socket> socket) {
  m_socket = socket;
}

void SelectiveReceiver::StartApplication() {
  m_socket->SetRecvCallback(MakeCallback(&SelectiveReceiver::HandleRead, this));
}

void SelectiveReceiver::StopApplication() {
  if (m_socket) {
    m_socket->Close();
  }
}

void SelectiveReceiver::HandleRead(Ptr<Socket> socket) {
  Ptr<Packet> packet = socket->Recv();

  SeqTsHeader seqHeader;
  packet->RemoveHeader(seqHeader);
  uint32_t seq = seqHeader.GetSeq();

  NS_LOG_INFO("Received packet " << seq << ", sending ACK");

  Ptr<Packet> ack = Create<Packet>(10);
  SeqTsHeader ackHeader;
  ackHeader.SetSeq(seq);
  ack->AddHeader(ackHeader);
  socket->Send(ack);
}

// ---------------- Main Simulation ----------------
int main(int argc, char *argv[]) {
  LogComponentEnable("SelectiveArqExample", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create(2);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("1Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("10ms"));

  NetDeviceContainer devices = pointToPoint.Install(nodes);

  InternetStackHelper stack;
  stack.Install(nodes);

  Ipv4AddressHelper address;
  address.SetBase("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign(devices);

  TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");

  // Receiver
  Ptr<Socket> recvSocket = Socket::CreateSocket(nodes.Get(1), tid);
  InetSocketAddress recvAddr = InetSocketAddress(Ipv4Address::GetAny(), 8080);
  recvSocket->Bind(recvAddr);
  recvSocket->Connect(InetSocketAddress(interfaces.GetAddress(0), 8081));

  Ptr<SelectiveReceiver> receiverApp = CreateObject<SelectiveReceiver>();
  receiverApp->Setup(recvSocket);
  nodes.Get(1)->AddApplication(receiverApp);
  receiverApp->SetStartTime(Seconds(0.0));
  receiverApp->SetStopTime(Seconds(30.0));

  // Sender
  Ptr<Socket> sendSocket = Socket::CreateSocket(nodes.Get(0), tid);
  InetSocketAddress senderAddr = InetSocketAddress(Ipv4Address::GetAny(), 8081);
  sendSocket->Bind(senderAddr);

  Ptr<SelectiveSender> senderApp = CreateObject<SelectiveSender>();
  senderApp->Setup(sendSocket, InetSocketAddress(interfaces.GetAddress(1), 8080), 1024, 10, 4, Seconds(2.0));
  nodes.Get(0)->AddApplication(senderApp);
  senderApp->SetStartTime(Seconds(1.0));
  senderApp->SetStopTime(Seconds(30.0));

  // NetAnim Setup
  AnimationInterface anim("selective-arq.xml");
  anim.SetConstantPosition(nodes.Get(0), 10, 20);
  anim.SetConstantPosition(nodes.Get(1), 50, 20);
  anim.UpdateNodeDescription(nodes.Get(0), "Sender");
  anim.UpdateNodeDescription(nodes.Get(1), "Receiver");
  anim.EnablePacketMetadata(true);

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

