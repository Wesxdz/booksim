import random
import string
import time
import os

agents = ["impasse", "cascade", "ember", "vortice", "ivory", "bulwark"]

while True:
    # Generate a random delay
    delay = random.uniform(0.5, 3)
    time.sleep(delay)

    # Choose a random agent
    chosen_agent = random.choice(agents)

    # Generate a random string of 20 ASCII characters
    random_string = ''.join(random.choices(string.ascii_letters + string.digits, k=20))

    # Count the existing dialog files for this agent
    n = len([f for f in os.listdir('.') if os.path.isfile(f) and f.startswith(f'dialogue_{chosen_agent}_')])

    # Create a new file
    with open(f'dialogue_{chosen_agent}_{n+1}.txt', 'w') as f:
        f.write(random_string)
