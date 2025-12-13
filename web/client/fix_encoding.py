
input_file = 'd:/esp_test_code/project/sample_project/web/client/index.html'
output_file = input_file

with open(input_file, 'r', encoding='utf-8') as f:
    lines = f.readlines()

# Line 16 is index 15
# Double check content (optional, but good for safety)
print(f"Old line 16: {lines[15]}")

# Replace with clean version, keeping indentation
lines[15] = '            <h1>小月智能家居系统</h1>\n'

with open(output_file, 'w', encoding='utf-8') as f:
    f.writelines(lines)

print("Line 16 replaced successfully.")
