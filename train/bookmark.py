from bs4 import BeautifulSoup

def convert_bookmarks(file_name):
    with open(file_name, 'r', encoding='utf-8') as f:
        contents = f.read()
        soup = BeautifulSoup(contents, 'html.parser')

    bookmarks = []

    for link in soup.find_all('a'):
        href = link.get('href')
        text = link.string
        bookmarks.append(f"{text} [{href}]")

    return bookmarks

bookmarks = convert_bookmarks('bookmarks.html')

# Saving the output to a text file
with open('output.txt', 'w') as f:
    for i,bookmark in enumerate(bookmarks):
        f.write(f"{i}. %s\n" % bookmark)
