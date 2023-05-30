
import os
print("Hello")

# Create a file
with open('/vortice/testfile.txt', 'w') as f:
    f.write('This is a test file.')

# Rename the file
os.rename('/vortice/testfile.txt', '/vortice/renamedfile.txt')

# Remove the file
os.remove('/vortice/renamedfile.txt')

print('File operations completed successfully.')
