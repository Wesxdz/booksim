# Use an official Ubuntu 20.04 as a parent image
FROM ubuntu:20.04

# Set the working directory in the container to /app
WORKDIR /app

# Update the system and install necessary tools
RUN apt-get update && apt-get install -y software-properties-common

# Add the deadsnakes PPA
RUN add-apt-repository -y ppa:deadsnakes/ppa

# Update again and install Python 3.9, Git, CMake, Clang, and python3-pip
RUN apt-get update && apt-get install -y python3.9 python3-pip git cmake clang wget unzip firefox xvfb

# Upgrade pip
RUN python3.9 -m pip install --upgrade pip

# Make python3.9 the default Python
RUN update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.9 1

# Verify python3 points to python 3.9
RUN python3 --version

# Install necessary Python packages: Selenium and BeautifulSoup
RUN python3.9 -m pip install selenium beautifulsoup4 

RUN python3.9 -m pip install watchdog

# Cleanup
RUN apt-get clean && rm -rf /var/lib/apt/lists/*
