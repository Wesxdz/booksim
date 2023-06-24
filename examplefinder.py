import os
import json

def parse_directory(base_dir, exclude_dir='build'):
    result = []
    subdirs = []
    for name in os.listdir(base_dir):
        path = os.path.join(base_dir, name)
        if os.path.isdir(path) and name != exclude_dir:
            if name in ['src', 'include']:
                subdirs.append(name)
            else:
                subdir_content = parse_directory(path)
                if subdir_content:
                    subdirs.append({name: subdir_content})
    if subdirs:
        result.append({
            os.path.basename(base_dir): subdirs
        })
    return result

def write_to_json_file(data, filename):
    with open(filename, 'w') as f:
        json.dump(data, f, indent=4)

base_dir = './game/flecs-3.2.3/examples/c/'  # replace with your directory
output_file = 'examples.json'  # replace with your output file
max_depth = 1
exclude_dir = 'build'

parsed_data = parse_directory(base_dir, exclude_dir)
write_to_json_file(parsed_data, output_file)
