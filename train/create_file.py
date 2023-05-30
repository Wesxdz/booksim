import os

def create_file(directory, filename):
    try:
        filepath = os.path.join(directory, filename)
        with open(filepath, 'w') as f:
            f.write('Hello, World!')
        print(f'Successfully created file at {filepath}')
    except PermissionError:
        print(f'Permission denied for creating file at {directory}')

# Create file in /home/aeri/ant/dojo/pasta/team/work/
create_file('/home/aeri/ant/dojo/pasta/team/work/', 'file1.txt')

# Try to create file in /home/aeri/ant/dojo/pasta/team/
create_file('/home/aeri/ant/dojo/pasta/team/', 'file2.txt')
