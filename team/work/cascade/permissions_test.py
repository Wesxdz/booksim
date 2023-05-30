
import os
print("Hello")

# Create a file
with open('/cascade/testfile.txt', 'w') as f:
    f.write('This is a test file.')

# Rename the file
os.rename('/cascade/testfile.txt', '/cascade/renamedfile.txt')

# Remove the file
os.remove('/cascade/renamedfile.txt')

print('File operations completed successfully.')
