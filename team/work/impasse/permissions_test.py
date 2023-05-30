
import os
print("Hello")

# Create a file
with open('/impasse/testfile.txt', 'w') as f:
    f.write('This is a test file.')

# Rename the file
os.rename('/impasse/testfile.txt', '/impasse/renamedfile.txt')

# Remove the file
os.remove('/impasse/renamedfile.txt')

print('File operations completed successfully.')
