import socket
import openai
import json
import logging
import psutil
import os
import threading

openai.api_key = "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"

# Set up logging
logging.basicConfig(filename='/rpg/server.log', level=logging.DEBUG)

def handle_request(connection):
    try:
        # Receive the data in small chunks and retransmit it
        data = connection.recv(4096)
        if data:
            request = json.loads(data)
            agent = request['agent']  # Assuming agent's id is in the request
            model = request['model']
            messages = request['messages']

            logging.info(f"Received request: {request}")

            agent_stats_dir = "/game_world" # TODO: Potentially consider distinct game worlds containing 'virtual' data
            # Check the agent's budget
            stats_file = os.path.join(agent_stats_dir, f'{agent}_stats.json')
            with open(stats_file, 'r') as f:
                stats = json.load(f)
            budget = stats['budget']

            if budget <= 0:
                logging.error(f"Agent {agent} has insufficient budget")
                connection.sendall(json.dumps({"error": "Insufficient budget"}).encode())
            else:
                # Make the OpenAI API call
                response = openai.ChatCompletion.create(
                    model="gpt-4",  # for now we limit model use
                    messages=messages
                )
                output = response.choices[0].message.content
                # if model = "gpt-3.5-turbo"
                # cost = response.usage.total_tokens / 1000.0 * 0.002
                cost = response.usage.total_tokens / 1000.0 * 0.03
                logging.info(f"Sending response: {response}")

                # Deduct the cost from the agent's budget
                budget -= cost
                stats['budget'] = budget
                with open(stats_file, 'w') as f:
                    json.dump(stats, f, indent=4)

                connection.sendall(json.dumps({"output": output, "cost": cost}).encode())
    finally:
        # Clean up the connection
        connection.close()


def kill_existing_server(port):
    for proc in psutil.process_iter(['pid', 'name']):
        for conns in proc.connections(kind='inet'):
            if conns.laddr.port == port:
                proc.kill()
                logging.info(f"Killed existing server process {proc.pid}")

def main():
    # Kill existing server if it's already running
    kill_existing_server(5000)
    
    # Wait until the old server is properly terminated
    while True:
        if not any((conns.laddr.port == 5000 for proc in psutil.process_iter(['pid', 'name']) for conns in proc.connections(kind='inet'))):
            break

    # Create a TCP/IP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Bind the socket to the port
    server_address = ('0.0.0.0', 5000)  # changed from 'localhost' to '0.0.0.0'
    sock.bind(server_address)

    logging.info("Server started, listening on port 5000")

    # Listen for incoming connections
    sock.listen(1)

    with open('/party/server_ready.txt', 'w') as f:
        f.write("Server is ready")

    while True:
        # Wait for a connection
        connection, client_address = sock.accept()
        logging.info(f"Accepted connection from {client_address}")
        
        thread = threading.Thread(target=handle_request, args=(connection,))
        thread.start()

if __name__ == "__main__":
    main()
