import heapq

def heuristic(a, b):
    return abs(a[0]-b[0]) + abs(a[1]-b[1])

def a_star(grid, start, goal):

    rows, cols = len(grid), len(grid[0])
    pq = [(0, start)]
    g = {start:0}
    parent = {start:None}

    moves = [(0,1),(1,0),(0,-1),(-1,0)]

    while pq:
        _, current = heapq.heappop(pq)

        if current == goal:
            path=[]
            while current:
                path.append(current)
                current=parent[current]
            return path[::-1]

        for dx,dy in moves:
            x,y = current[0]+dx, current[1]+dy

            if 0<=x<rows and 0<=y<cols and grid[x][y]==0:
                new_cost = g[current] + 1
                if (x,y) not in g or new_cost < g[(x,y)]:
                    g[(x,y)] = new_cost
                    f = new_cost + heuristic((x,y),goal)
                    heapq.heappush(pq,(f,(x,y)))
                    parent[(x,y)] = current

    return None


grid = [
[0,0,0],
[1,0,1],
[0,0,0]
]

start=(0,0)
goal=(2,2)

print("Path:", a_star(grid,start,goal))