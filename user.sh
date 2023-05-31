#!/bin/bash

# Define the agents
agents=("impasse" "cascade" "ember" "vortice" "ivory" "bulwark")

# Create the 'booksim' group if it doesn't exist
if ! getent group booksim > /dev/null; then
    sudo groupadd booksim
fi

# Remove the existing users, create them again with no home directories, and add them to the 'booksim' group
for agent in ${agents[@]}; do
    sudo deluser --remove-home $agent
    sudo adduser --system --no-create-home --ingroup booksim $agent

    # Edit /etc/subuid and /etc/subgid to map the users to user namespaces
    echo "$agent:231072:65536" | sudo tee -a /etc/subuid
    echo "$agent:231072:65536" | sudo tee -a /etc/subgid

    # Give recursive file permissions to the agent within their own directory
    sudo setfacl -Rm "u:$agent:rwx" "/home/aeri/ant/dojo/booksim/team/work/$agent"

    # Give recursive file permissions to the agent within the 'party' directory
    sudo setfacl -Rm "u:$agent:rwx" "/home/aeri/ant/dojo/booksim/team/work/party"
done
