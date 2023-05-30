# client.py
import socket
import json
import sys


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

        # Write the output to the agent's bind mount
        with open(f'/{agent}/response.log', 'a+') as f:
            f.write(response.decode() + "\n")
    finally:
        sock.close()


if __name__ == "__main__":
    agent = sys.argv[1]  # Get the agent's name from the command line arguments
    model = 'gpt-3.5-turbo'
    messages = [
        {"role": "assistant", "content": f"My name is {agent}. I am in an RPG. What are the stats?"},
    ]
    send_request(agent, model, messages)
