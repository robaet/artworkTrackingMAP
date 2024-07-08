def sort_dict_by_keys(d):
    # Sort the dictionary by keys and return a new dictionary
    return dict(sorted(d.items()))

# Example usage
my_dict = {
    "banana": 3,
    "apple": 4,
    "cherry": 2,
    "date": 1
}

sorted_dict = sort_dict_by_keys(my_dict)
print(sorted_dict)
