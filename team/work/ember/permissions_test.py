
import os
print("Hello")

# Create a file
with open('/ember/testfile.txt', 'w') as f:
    f.write('This is a test file.')

# Rename the file
os.rename('/ember/testfile.txt', '/ember/renamedfile.txt')

# Remove the file
os.remove('/ember/renamedfile.txt')

print('File operations completed successfully.')
