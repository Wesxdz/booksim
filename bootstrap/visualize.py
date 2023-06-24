import json

query = "lq/flecs_manual/create_prefab.json"

def txt_to_html_with_highlight(txt_file, html_file, linespans):
    with open(txt_file, 'r') as f:
        lines = f.readlines()

    with open(html_file, 'w') as f:
        f.write('<html><body>')

        for i, line in enumerate(lines):
            # Check if this line is within any of the linespans
            highlight = any(span['start'] <= i+1 <= span['stop'] for span in linespans)

            if highlight:
                f.write(f'<p style="background-color:lightgreen">{line.strip()}</p>')
            else:
                f.write(f'<p>{line.strip()}</p>')

        f.write('</body></html>')

# Load json file
with open(query) as f:
    data = json.load(f)

# Extract file path and linespans
file_path = data['file_path']
linespans = data['linespans']

# Convert .txt to .html with highlight
txt_to_html_with_highlight(file_path, f"{query.split('/')[-1].split('.')[0]}_highlighted.html", linespans)
