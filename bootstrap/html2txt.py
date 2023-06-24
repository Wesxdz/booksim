from bs4 import BeautifulSoup

def html_to_txt(html_file, txt_file):
    # Read the HTML file
    with open(html_file, 'r') as f:
        html_content = f.read()

    # Parse the HTML using BeautifulSoup
    soup = BeautifulSoup(html_content, 'html.parser')

    # Extract text from the HTML
    text = soup.get_text()

    # Remove blank lines
    lines = text.split("\n")
    non_empty_lines = [line for line in lines if line.strip() != ""]
    text_without_blank_lines = "\n".join(non_empty_lines)

    # Indicate headers in Markdown format
    headers = soup.find_all(['h1', 'h2', 'h3', 'h4', 'h5', 'h6'])
    for header in headers:
        level = int(header.name[1])
        header_text = header.get_text().strip()
        header_line = '#' * level + ' ' + header_text
        text_without_blank_lines = text_without_blank_lines.replace(header_text, header_line)

    # Write the text to a plain text file
    with open(txt_file, 'w') as f:
        f.write(text_without_blank_lines)
# Example usage
html_file = 'flecs_manual.html'
txt_file = 'flecs_manual.txt'
html_to_txt(html_file, txt_file)
