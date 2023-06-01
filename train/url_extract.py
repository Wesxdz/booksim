import requests
from bs4 import BeautifulSoup

# Request the webpage
response = requests.get('https://www.flecs.dev/flecs/md_docs_Docs.html')

# Parse the webpage content
soup = BeautifulSoup(response.content, 'html.parser')

# Find and extract all URLs
urls = []
for link in soup.find_all('a'):
    print(link)
    href = link.get('href')
    if href.startswith('http'):  # This will ensure we're getting absolute urls, not relative ones
        urls.append(href)

# Write URLs to a text file
with open('urls.txt', 'w') as file:
    for url in urls:
        file.write(f'{url}\n')
