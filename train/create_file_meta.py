import os

def create_file(directory, filename, content):
    try:
        filepath = os.path.join(directory, filename)
        with open(filepath, 'w') as f:
            f.write(content)
        print(f'Successfully created file at {filepath}')
    except PermissionError:
        print(f'Permission denied for creating file at {directory}')