
import sys
import os
from selenium import webdriver
from selenium.webdriver.firefox.service import Service
from bs4 import BeautifulSoup

# Directory where Firefox and geckodriver are located
firefox_dir = "/party"


print(f"Directory exists: {os.path.exists(firefox_dir)}")
print(f"Directory contents: {os.listdir(firefox_dir)}")

# Paths to Firefox and geckodriver
firefox_binary_path = os.path.join(firefox_dir, "firefox", "firefox")

print(f"Directory exists: {os.path.exists(firefox_dir)}")
print(f"Directory contents: {os.listdir(firefox_dir)}")

print(f"Firefox binary path: {firefox_binary_path}")
print(f"Firefox binary exists: {os.path.exists(firefox_binary_path)}")

topic = sys.argv[1]

# Setting up Selenium to use Firefox headlessly
options = webdriver.FirefoxOptions()
options.add_argument('-headless')
options.binary_location = firefox_binary_path

# Creating a new Firefox webdriver
try:
    driver = webdriver.Firefox(options=options)
except Exception as e:
    print(f"Error while creating WebDriver: {e}")

# Making a Google search
try:
    driver.get('https://www.google.com/search?q=' + topic)
except Exception as e:
    print(f"Error while making Google search: {e}")

# Getting the source of the page and quitting the driver
try:
    page_source = driver.page_source
    driver.quit()
except Exception as e:
    print(f"Error while getting page source and quitting driver: {e}")

# Parsing the source with BeautifulSoup and finding the first link title
try:
    # Parsing the source with BeautifulSoup and finding the first link URL
    soup = BeautifulSoup(page_source, 'html.parser')
    link_url = soup.find('div', class_='yuRUbf').a['href']
    print(link_url)
except Exception as e:
    print(f"Error while parsing source and printing link title: {e}")

