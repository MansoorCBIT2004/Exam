def a_star(graph, start, goal, h):
    open_list = {start}
    closed_list = set()

    g = {start: 0}
    parent = {start: start}

    while open_list:
        n = min(open_list, key=lambda x: g[x] + h[x])

        if n == goal:
            path = []
            while parent[n] != n:
                path.append(n)
                n = parent[n]
            path.append(start)
            path.reverse()
            return path

        for (m, cost) in graph[n]:
            if m not in open_list and m not in closed_list:
                open_list.add(m)
                parent[m] = n
                g[m] = g[n] + cost
            else:
                if g[m] > g[n] + cost:
                    g[m] = g[n] + cost
                    parent[m] = n
                    if m in closed_list:
                        closed_list.remove(m)
                        open_list.add(m)

        open_list.remove(n)
        closed_list.add(n)

    return None


graph = {
    'A': [('B',1), ('C',3)],
    'B': [('D',3), ('E',1)],
    'C': [('F',5)],
    'D': [],
    'E': [('F',2)],
    'F': []
}

h = {
    'A':5,
    'B':3,
    'C':4,
    'D':2,
    'E':1,
    'F':0
}

print("Path:", a_star(graph, 'A', 'F', h))