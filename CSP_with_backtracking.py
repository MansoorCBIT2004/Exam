def backtrack(assignment, variables, domains, constraints):

    if len(assignment) == len(variables):
        return assignment

    var = [v for v in variables if v not in assignment][0]

    for value in domains[var]:

        valid = True
        for constraint in constraints:
            if not constraint(assignment, var, value):
                valid = False
                break

        if valid:
            assignment[var] = value

            result = backtrack(assignment, variables, domains, constraints)
            if result:
                return result

            del assignment[var]

    return None


def example_constraint(assignment, variable, value):
    # All variables must have different values
    for var, val in assignment.items():
        if val == value:
            return False
    return True


if __name__ == "__main__":

    # Variables
    variables = ['X1', 'X2', 'X3']

    # Domains
    domains = {
        'X1': [1, 2, 3],
        'X2': [1, 2, 3],
        'X3': [1, 2, 3]
    }

    # Constraints
    constraints = [example_constraint]

    # Solve CSP
    assignment = {}

    solution = backtrack(assignment, variables, domains, constraints)

    if solution:
        print("Solution found:", solution)
    else:
        print("No solution exists")