
import os
print("Hello")

# Create a file
with open('/ivory/testfile.txt', 'w') as f:
    f.write('This is a test file.')

# Rename the file
os.rename('/ivory/testfile.txt', '/ivory/renamedfile.txt')

# Remove the file
os.remove('/ivory/renamedfile.txt')

print('File operations completed successfully.')
