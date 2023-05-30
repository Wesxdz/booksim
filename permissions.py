import docker
import datetime
import os
import json

# Create a Docker client
client = docker.from_env()

# RPG party containing the agents
party = "party"

# Load agent descriptions from JSON file
with open("/home/aeri/ant/dojo/pasta/team/desc.json", "r") as f:
    agent_descriptions = json.load(f)

# Define base path
base_path = "/home/aeri/ant/dojo/pasta/team/work/"

# Define log directory
log_dir = os.path.join(base_path, 'party')

# Clear the existing logs
for filename in os.listdir(log_dir):
    if filename.endswith(".log"):
        os.remove(os.path.join(log_dir, filename))

# Prepare and run the Python script in each container
for agent in agent_descriptions.keys():
    # Get the container
    container = client.containers.get(agent)

    # Define agent path
    agent_path = os.path.join(base_path, agent)

    # Write agent description to the agent's bind mount
    with open(os.path.join(agent_path, 'description.json'), 'w') as f:
        json.dump(agent_descriptions[agent], f, indent=4)

    # Create a Python script in the agent's bind mount for the permission tests
    with open(f"{agent_path}/permissions_test.py", 'w') as f:
        f.write("""
import os
print("Hello")

# Create a file
with open('/{}/testfile.txt', 'w') as f:
    f.write('This is a test file.')

# Rename the file
os.rename('/{}/testfile.txt', '/{}/renamedfile.txt')

# Remove the file
os.remove('/{}/renamedfile.txt')

print('File operations completed successfully.')
""".format(agent, agent, agent, agent))

    # Run the Python script
    result = container.exec_run("python3 /{}/permissions_test.py".format(agent))

    # Write the output to the shared volume
    with open(os.path.join(log_dir, 'shared.log'), 'a') as f:
        f.write("{} - {}\n".format(datetime.datetime.now().isoformat(), result.output.decode().strip()))

print("Python scripts executed in each container and logs collected in the shared volume.")
