import docker
import os
from pathlib import Path
import subprocess

os.system("docker build -t my_ubuntu:latest .")
os.system("docker build -t my_interface:latest -f dockerfile_interface .")

# Create a Docker client
client = docker.from_env()

# network = client.networks.create("zone_one") # TODO: Make zones that allow multiple distinct parties to interact
network = client.networks.create("alpha_online")

# Define the agent names
agents = ["impasse", "cascade", "ember", "vortice", "ivory", "bulwark"]

# Define memory limit
memory_limit = "4g"  # This sets a limit of 4 gigabytes

# Define base directory
base_path = "/home/aeri/ant/dojo/pasta/team/work/"

# Create the directory if it doesn't exist already
Path(base_path).mkdir(parents=True, exist_ok=True)

# Define shared directory
game_world_path = os.path.join(base_path, "game_world")

# Create the shared directory if it doesn't exist already
Path(game_world_path).mkdir(parents=True, exist_ok=True)

# Set owner and permissions on the host
subprocess.run(['sudo', 'chown', '-R', '0:0', game_world_path])
subprocess.run(['sudo', 'chmod', '-R', '777', game_world_path])

# Define shared directory
shared_path = os.path.join(base_path, "party")

# Create the shared directory if it doesn't exist already
Path(shared_path).mkdir(parents=True, exist_ok=True)

# Create a directory for each agent and adjust ownership and permissions
for agent in agents:
    agent_path = os.path.join(base_path, agent)
    Path(agent_path).mkdir(parents=True, exist_ok=True)

    # Bind both the agent-specific directory and the shared directory
    volumes = {
        agent_path: {'bind': f'/{agent}', 'mode': 'rw'},
        shared_path: {'bind': '/party', 'mode': 'rw'},
        game_world_path: {'bind': '/game_world', 'mode': 'ro'}  # Read-only access for agents
    }

    # Here we use 'root' user inside the container for simplicity
    client.containers.run(
        "my_ubuntu:latest",
        name=agent,
        volumes=volumes,
        mem_limit=memory_limit,
        detach=True,
        tty=True,
        network="alpha_online",
    )

    # Change the owner of the directories on the host to 'root'.
    # Note: 'root' is usually UID 0 and GID 0.
    subprocess.run(['sudo', 'chown', '-R', '0:0', agent_path])
    subprocess.run(['sudo', 'chown', '-R', '0:0', shared_path])

    # Set permissions of the directories on the host to 777 (read/write/execute for all)
    subprocess.run(['sudo', 'chmod', '-R', '777', agent_path])
    subprocess.run(['sudo', 'chmod', '-R', '777', shared_path])


# Define shared directory
rpg_path = os.path.join(base_path, "RPG")

# Create the shared directory if it doesn't exist already
Path(rpg_path).mkdir(parents=True, exist_ok=True)

interface_container = client.containers.run(
    "my_interface:latest",
    name="RPG",
    volumes={
        shared_path: {'bind': '/vacuum_shield', 'mode': 'rw'}, 
        rpg_path: {'bind': '/rpg', 'mode': 'rw'},
        game_world_path: {'bind': '/game_world', 'mode': 'rw'}  # Read-write access for RPG
    },
    mem_limit=memory_limit,
    detach=True,
    tty=True,
    network="alpha_online",
)


subprocess.run(['sudo', 'chown', '-R', '0:0', rpg_path])
subprocess.run(['sudo', 'chmod', '-R', '777', rpg_path])


print("Containers created successfully.")
