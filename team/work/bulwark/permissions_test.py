
import os
print("Hello")

# Create a file
with open('/bulwark/testfile.txt', 'w') as f:
    f.write('This is a test file.')

# Rename the file
os.rename('/bulwark/testfile.txt', '/bulwark/renamedfile.txt')

# Remove the file
os.remove('/bulwark/renamedfile.txt')

print('File operations completed successfully.')
