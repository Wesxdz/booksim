import os
import subprocess
import time
import sys
import logging

from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

class FileCreationHandler(FileSystemEventHandler):
    def __init__(self, script_path, script_args):
        super().__init__()
        self.script_path = script_path
        self.script_args = script_args

    def on_created(self, event):
        if event.src_path == '/party/server_ready.txt':
            # Wait a moment for the file to be ready
            time.sleep(1)

            # Execute the client script
            subprocess.run(['python3', self.script_path] + self.script_args)

if __name__ == "__main__":
    agent_name = sys.argv[1]
    script_path = f'/{agent_name}/client.py'
    script_args = [agent_name]

    # Initialize logging
    logging.basicConfig(filename=f'/{agent_name}/observer.log', level=logging.DEBUG)
    logging.info('Observer started.')

    # Check if server_ready.txt already exists
    if os.path.exists('/party/server_ready.txt'):
        # Execute the client script immediately
        logging.info('server_ready.txt exists. Executing client script.')
        subprocess.run(['python3', script_path] + script_args)
    else:
        # Create an observer and event handler
        observer = Observer()
        event_handler = FileCreationHandler(script_path, script_args)

        # Set the path to monitor for file creation
        path_to_monitor = '/party'

        # Start the observer
        observer.schedule(event_handler, path_to_monitor, recursive=False)
        observer.start()

        logging.info('Observer scheduled. Waiting for server_ready.txt.')

        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            observer.stop()

        observer.join()
