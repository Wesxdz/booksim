#!/bin/bash

# Stop Docker daemon
sudo systemctl stop docker

# Create or modify /etc/docker/daemon.json for user namespace remap
sudo bash -c 'echo -e "{\n\t\"userns-remap\": \"default\"\n}" > /etc/docker/daemon.json'

# Start Docker daemon
sudo systemctl start docker
