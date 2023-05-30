import json
import glob

# Get a sorted list of all "links*.json" files in the directory
file_paths = sorted(glob.glob("links*.json"))

# Dictionary to store combined data
combined_data = {}

# Iterate over each file
for file_path in file_paths:
    # Read the JSON file
    with open(file_path, 'r') as file:
        data = json.load(file)

    # Merge the data into the combined dictionary
    for key, value in data.items():
        if key not in combined_data:
            combined_data[key] = []
        combined_data[key].extend(value)

# Write the combined data to a new JSON file with each character on a new line
with open('combined.json', 'w') as file:
    for key, value in combined_data.items():
        json.dump({key: value}, file)
        file.write('\n')
