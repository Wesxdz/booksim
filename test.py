import docker
import datetime
import os

# Create a Docker client
client = docker.from_env()

# RPG party containing the agents
party = "party"

# Define the agent names and their interests
agents = {
    "impasse": "AI research papers, AI consciousness and safety",
    "cascade": "arXiv, self-replicating biotechnological system",
    "ember": "Renewable energy forums, unstable energy source",
    "vortice": "Futurist discussion, manipulating planetary weather systems",
    "ivory": "European design companies, the perfect material",
    "bulwark": "Medieval oneironautics, building impenetrable fortresses"
}

# Define base path
base_path = "/home/aeri/ant/dojo/pasta/team/work/"

# Define log directory
log_dir = os.path.join(base_path, 'party')

# Clear the existing logs
for filename in os.listdir(log_dir):
    if filename.endswith(".log"):
        os.remove(os.path.join(log_dir, filename))

# The Python script that will be created in each container
# /{agent_name}"

pre_script = """
import sys
import os
from selenium import webdriver
from selenium.webdriver.firefox.service import Service
from bs4 import BeautifulSoup

# Directory where Firefox and geckodriver are located
firefox_dir = "/{party_name}"
"""

script_content = """

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

"""

# Prepare and run the Python script in each container
for agent, topic in agents.items():
    # Get the container
    container = client.containers.get(agent)

    # Define agent path
    agent_path = os.path.join(base_path, agent)

    # Create the Python script in the agent's bind mount
    with open(os.path.join(agent_path, 'script.py'), 'w') as f:
        f.write(pre_script.format(party_name=party) + script_content)

    # Install selenium and BeautifulSoup4
    exit_code, output = container.exec_run("pip install selenium beautifulsoup4")

    # Run the Python script
    result = container.exec_run("python3 /{}/script.py \"{}\"".format(agent, topic))

    # Write the output to the shared volume
    with open(os.path.join(log_dir, 'shared.log'), 'a') as f:
        f.write("{} - {}\n".format(datetime.datetime.now().isoformat(), result.output.decode().strip()))

print("Python scripts executed in each container and logs collected in the shared volume.")
