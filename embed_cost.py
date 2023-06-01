import sys
import tiktoken

def encode_file(filename):

    # Get the encoding for a specific model in the OpenAI API
    enc = tiktoken.encoding_for_model("gpt-4-0314")

    # Read the file
    with open(filename, 'r') as file:
        text = file.read()

    # Encode the text
    encoded_text = enc.encode(text)

    # Decode the text
    decoded_text = enc.decode(encoded_text)

    return encoded_text

if __name__ == "__main__":
    filename = sys.argv[1]
    token_count = len(encode_file(filename))
    print(f"Token count: {token_count}")

    # Calculate cost
    cost_per_token = 0.002 / 1000  # $0.002 per 1K tokens
    total_cost = cost_per_token * token_count
    print(f"Total cost: ${total_cost}")
