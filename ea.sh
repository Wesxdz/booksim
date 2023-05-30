#!/bin/bash

# Check if container name is provided as a command-line argument
if [ -z "$1" ]; then
  echo "Error: Container name not provided."
  echo "Usage: ./enter_container_bash.sh <container_name>"
  exit 1
fi

container_name="$1"

# Get the container ID based on the container name
container_id=$(docker ps -qf "name=$container_name")

if [ -z "$container_id" ]; then
  echo "Error: No such container found with name: $container_name"
  exit 1
fi

# Execute the command with the obtained container ID
docker exec -it "$container_id" /bin/bash
