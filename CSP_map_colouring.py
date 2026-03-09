graph = {
    'A': ['B','C'],
    'B': ['A','C','D'],
    'C': ['A','B','D'],
    'D': ['B','C']
}

colors = ['Red','Green','Blue']

assignment = {}


def is_valid(node, color):
    for neighbour in graph[node]:
        if neighbour in assignment and assignment[neighbour] == color:
            return False
    return True


def backtrack():
    if len(assignment) == len(graph):
        return True

    node = list(graph.keys())[len(assignment)]

    for color in colors:
        if is_valid(node, color):
            assignment[node] = color

            if backtrack():
                return True

            del assignment[node]

    return False


if backtrack():
    print("Solution:")
    for region in assignment:
        print(region, "->", assignment[region])
else:
    print("No solution")