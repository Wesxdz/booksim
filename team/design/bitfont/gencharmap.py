# Python script to generate charMap
charMap = {}

# Grid size
grid_size = 8

# Start at ASCII 'SP' (Space), which is ASCII 32.
for i in range(32, 127):
    char = chr(i)
    
    # Calculate position of character in grid.
    grid_x = (i - 32) % grid_size
    grid_y = (i - 32) // grid_size
    
    # Each character is an 8x8 rectangle.
    rect = [grid_x * 8, grid_y * 8, 8, 8]
    
    charMap[char] = rect

# Write charMap to char-map.lua
with open('char-map.lua', 'w') as file:
    file.write('charMap = {\n')
    for char, rect in charMap.items():
        file.write(f"  ['{char}'] = {{{', '.join(map(str, rect))}}},\n")
    file.write('}\n')
