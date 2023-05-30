#!/bin/bash

# Execute cleanup.sh script
./cleanup.sh

# Execute sandbox.py script
python sandbox.py

# Execute rpg_start.py script with sudo
sudo python rpg_start.py
