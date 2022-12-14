

def convert_cp_to_prob(cp):
    return 1/(1 + 10**(-cp/400))

print(convert_cp_to_prob(-381))