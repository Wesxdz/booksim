#!/bin/bash

# Stop all running containers
docker stop $(docker ps -aq)

# Delete all stopped containers
docker rm $(docker ps -aq)

# Delete the network
docker network rm alpha_online

docker network prune --force