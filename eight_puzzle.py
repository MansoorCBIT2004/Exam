import heapq

goal = [[1,2,3],
        [4,5,6],
        [7,8,0]]

moves = [(0,1),(1,0),(0,-1),(-1,0)]


def heuristic(state):
    count = 0
    for i in range(3):
        for j in range(3):
            if state[i][j] != 0 and state[i][j] != goal[i][j]:
                count += 1
    return count


def find_zero(state):
    for i in range(3):
        for j in range(3):
            if state[i][j] == 0:
                return i,j


def neighbours(state):
    x,y = find_zero(state)
    result = []

    for dx,dy in moves:
        nx,ny = x+dx,y+dy

        if 0<=nx<3 and 0<=ny<3:
            new_state = [row[:] for row in state]
            new_state[x][y],new_state[nx][ny] = new_state[nx][ny],new_state[x][y]
            result.append(new_state)

    return result


def a_star(start):

    pq=[]
    heapq.heappush(pq,(heuristic(start),start))

    parent={}
    parent[str(start)] = None

    visited=set()

    while pq:

        cost,state = heapq.heappop(pq)

        if state == goal:
            path=[]
            while state is not None:
                path.append(state)
                state = parent[str(state)]
            return path[::-1]

        visited.add(str(state))

        for next_state in neighbours(state):

            if str(next_state) not in visited:

                heapq.heappush(pq,(heuristic(next_state),next_state))

                if str(next_state) not in parent:
                    parent[str(next_state)] = state

    return None


start = [[1,2,3],
         [0,4,6],
         [7,5,8]]

solution = a_star(start)

print("Steps to reach goal:\n")

step=0
for state in solution:
    print("Step",step)
    for row in state:
        print(row)
    print()
    step+=1