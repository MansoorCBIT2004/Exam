from collections import deque

def water_jug(cap1, cap2, goal):

    visited = set()
    queue = deque([(0,0)])

    while queue:

        x,y = queue.popleft()

        if (x,y) in visited:
            continue

        visited.add((x,y))

        print((x,y))

        if x == goal or y == goal:
            print("Goal reached")
            return

        next_states = [
            (cap1,y),   # fill jug1
            (x,cap2),   # fill jug2
            (0,y),      # empty jug1
            (x,0),      # empty jug2
        ]

        # pour jug1 → jug2
        transfer = min(x, cap2-y)
        next_states.append((x-transfer, y+transfer))

        # pour jug2 → jug1
        transfer = min(y, cap1-x)
        next_states.append((x+transfer, y-transfer))

        for state in next_states:
            if state not in visited:
                queue.append(state)


water_jug(4,3,2)