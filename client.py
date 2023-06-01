# client.py
import socket
import json
import sys
import subprocess

def load_message_from_file(file_name):
    with open(file_name, 'r') as file:
        content = json.load(file)
    return content

def send_request(agent, model, messages):
    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Connect the socket to the port where the server is listening
    server_address = ('RPG', 5000)
    sock.connect(server_address)

    try:
        # Send data
        request = {
            'agent':agent,
            'model': model,
            'messages': messages
        }
        sock.sendall(json.dumps(request).encode())

        # Receive the response
        response = sock.recv(4096)
        print('Received:', response.decode())
        print(eval(response.decode()).output)
        subprocess.run(["python", "script.py", eval(response.decode()).output])

        # Write the output to the agent's bind mount
        with open(f'/{agent}/response.log', 'a+') as f:
            f.write(response.decode() + "\n")
    finally:
        sock.close()


if __name__ == "__main__":
    agent = sys.argv[1]  # Get the agent's name from the command line arguments
    model = 'gpt-4'
    personality = load_message_from_file('description.json')
    messages = [
        {"role": "assistant", "content": f"{personality} Roleplay as {agent}. Output web search query that I should research."},
    ]
    send_request(agent, model, messages)
