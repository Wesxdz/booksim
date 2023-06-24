import json

query = "lq/flecs_manual/create_prefab.json"

def extract_highlighted_lines(txt_file, output_txt_file, linespans):
    with open(txt_file, 'r') as f:
        lines = f.readlines()

    highlighted_lines = []
    for i, line in enumerate(lines):
        # Check if this line is within any of the linespans
        highlight = any(span['start'] <= i+1 <= span['stop'] for span in linespans)
        if highlight:
            highlighted_lines.append(line.strip())

    # Write highlighted lines to output_txt_file
    with open(output_txt_file, 'w') as f:
        f.write("\n".join(highlighted_lines))
    
    # Return highlighted lines as a single string (joined by spaces)
    return " ".join(highlighted_lines)

# Load json file
with open(query) as f:
    data = json.load(f)

# Extract file path and linespans
file_path = data['file_path']
linespans = data['linespans']

# Extract highlighted lines to a new .txt file and get the highlighted lines as a string
highlighted_string = extract_highlighted_lines(file_path, f"{query.split('/')[-1].split('.')[0]}_highlighted.txt", linespans)

# Print the string of highlighted lines
print(highlighted_string)
