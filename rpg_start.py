# rpg_start.py
import docker
import datetime
import os
import json
import pytz

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

# STATS
# Define agent stats directory
agent_stats_dir = os.path.join(base_path, 'game_world')

# Daily budget for each agent
daily_budget = 1.0  # in dollars

# Clear the existing logs
for filename in os.listdir(log_dir):
    if filename.endswith(".log"):
        os.remove(os.path.join(log_dir, filename))

# Get the interface container
interface_container = client.containers.get("RPG")

# Define interface container path
interface_path = os.path.join(base_path, "RPG")

# Get the current date and time in MST
now = datetime.datetime.now(pytz.timezone('MST'))

# Load and reset agent stats, if it's past 6am
for agent in agent_descriptions.keys():
    stats_file = os.path.join(agent_stats_dir, f'{agent}_stats.json')
    if os.path.exists(stats_file):
        with open(stats_file, 'r') as f:
            stats = json.load(f)
        last_reset = datetime.datetime.fromisoformat(stats.get('last_reset'))
        if now.hour >= 6 and last_reset.day < now.day:
            stats['budget'] = daily_budget
            stats['last_reset'] = now.isoformat()
        with open(stats_file, 'w') as f:
            json.dump(stats, f, indent=4)
    else:  # Create the stats file if it doesn't exist
        stats = {
            'budget': daily_budget,
            'last_reset': now.isoformat(),
        }
        with open(stats_file, 'w') as f:
            json.dump(stats, f, indent=4)
# STATS

# Clear the existing logs
for filename in os.listdir(log_dir):
    if filename.endswith(".log"):
        os.remove(os.path.join(log_dir, filename))

# Get the interface container
interface_container = client.containers.get("RPG")

# Define interface container path
interface_path = os.path.join(base_path, "RPG")

# TODO: Refactor to load dict array of files to RPG/agent bind mounts
# TODO: Reorganize development deployment paradigm
# Load server script from file
with open('server.py', 'r') as file:
    server_script = file.read()

server_ready = os.path.join(log_dir, "server_ready.txt")
if os.path.exists(server_ready):
    os.remove(server_ready)
# Write server script to the interface container path
with open(os.path.join(interface_path, 'server.py'), 'w') as f:
    f.write(server_script)

# Run the server script in the interface container
result = interface_container.exec_run("python3 /rpg/server.py")

# Load client script from file
with open('client.py', 'r') as file:
    client_script = file.read()

# Load client script from file
with open('observer.py', 'r') as file:
    observer_script = file.read()

# Prepare and run the Python script in each container
print(agent_descriptions.keys())
for agent in agent_descriptions.keys():
    # Get the container
    container = client.containers.get(agent)

    # Define agent path
    agent_path = os.path.join(base_path, agent)

    # Write agent description to the agent's bind mount
    with open(os.path.join(agent_path, 'description.json'), 'w') as f:
        json.dump(agent_descriptions[agent], f, indent=4)

    # Write client script to the agent path
    with open(os.path.join(agent_path, 'client.py'), 'w') as f:
        f.write(client_script)

        # Write client script to the agent path
    with open(os.path.join(agent_path, 'observer.py'), 'w') as f:
        f.write(observer_script)

    # Run the client script
    print("aha")
    print("python3 /{}/observer.py {}".format(agent, agent))
    result = container.exec_run("python3 /{}/observer.py {}".format(agent, agent), detach=True)
    print(result)
    # print("STDOUT:\n", result.output.decode())
    # print("STDERR:\n", result.error)  # This should display error if any.


    # Write the output to the shared volume
    # with open(os.path.join(log_dir, 'shared.log'), 'a') as f:
    #     f.write("{} - {}\n".format(datetime.datetime.now().isoformat(), result.output.decode().strip()))

print("Python scripts executed in each container and logs collected in the shared volume.")
