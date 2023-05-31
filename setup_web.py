import os
import tarfile
import requests

def download_and_extract_firefox(dest_dir):
    # URL of the Firefox version for Linux 64-bit
    firefox_url = "https://ftp.mozilla.org/pub/firefox/releases/113.0.2/linux-x86_64/en-US/firefox-113.0.2.tar.bz2"
    response = requests.get(firefox_url, stream=True)
    file = tarfile.open(fileobj=response.raw, mode="r|bz2")
    file.extractall(path=dest_dir)
    file.close()

def download_and_extract_geckodriver(dest_dir):
    # URL of the latest geckodriver version for Linux 64-bit
    geckodriver_url = "https://github.com/mozilla/geckodriver/releases/download/v0.33.0/geckodriver-v0.33.0-linux64.tar.gz"
    response = requests.get(geckodriver_url, stream=True)
    file = tarfile.open(fileobj=response.raw, mode="r|gz")
    file.extractall(path=dest_dir)
    file.close()

# Set the directory where you want to store the binaries
bind_mount_dir = "/home/aeri/ant/dojo/booksim/team/work/party"

download_and_extract_firefox(bind_mount_dir)
download_and_extract_geckodriver(bind_mount_dir)

print("Firefox and geckodriver binaries have been downloaded and extracted.")
