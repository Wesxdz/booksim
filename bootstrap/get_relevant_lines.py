import json
import openai
import os
from tenacity import (
    retry,
    stop_after_attempt,
    wait_random_exponential,
)  

@retry(wait=wait_random_exponential(min=1, max=60), stop=stop_after_attempt(6))
def completion_with_backoff(**kwargs):
    return openai.ChatCompletion.create(**kwargs)

def file_to_string(file_path):
    with open(file_path, 'r') as file:
        content = file.read()
    return content

file_path = 'flecs_manual.txt'
file_content = file_to_string(file_path)

def output_relevant_linespans(linespans, prompt_gist, prompt):
    file_name = file_path.split('.')[0]
    directory = f"lq/{file_name}"
    if not os.path.exists(directory):
        os.makedirs(directory, exist_ok=True)
    data = {
        "file_path": file_path,
        "prompt": prompt,
        "linespans": linespans
    }
    output_json = json.dumps(data, indent=4)
    with open(f'{directory}/{prompt_gist}.json', "w") as f:
        f.write(output_json)
    print(linespans)
    lines = file_content.split('\n')
    for span in linespans:
        start = span['start']
        stop = span['stop']
        relevant_lines = lines[start - 1:stop]  # Assuming line numbers start at 1
        for line in relevant_lines:
            print(line)

def merge_linespans(linespans):
    # Sort linespans by 'start'
    linespans.sort(key=lambda x: x['start'])

    merged = []
    for span in linespans:
        # If 'stop' does not exist, set it equal to 'start'
        if 'stop' not in span:
            span['stop'] = span['start']

        # If list of merged spans is empty or current span does not overlap with last span in merged
        if not merged or merged[-1]['stop'] < span['start'] - 1:  
            merged.append(span)
        else:
            # If current span overlaps with or is adjacent to last span in merged, merge them
            merged[-1]['stop'] = max(merged[-1]['stop'], span['stop'])

    return merged



def get_relevant_lines(prompt: str, document: str):
    """
    Returns an array of objects representing lines of a txt document that are relevant to a prompt.
    """
    lines = document.split('\n')
    all_linespans = []

    chunk_size = 100
    overlap = 10  # Modify as needed
    global prompt_gist
    prompt_gist = None

    for chunk_start in range(0, len(lines), chunk_size - overlap):
        
        chunk = lines[chunk_start : chunk_start + chunk_size]
        numbered_lines = []

        for idx, line in enumerate(chunk, start=chunk_start + 1):
            numbered_line = f"{idx} {line}"
            numbered_lines.append(numbered_line)

        messages=[{"role": "user", "content": f"Use decodable JSON. Output linespans of the document that match the prompt: {prompt}"},
                    {"role": "user", "content": f"DOCUMENT_BEGIN:\n{numbered_lines[:100]}"}] #TODO: Parse document in chunks
        functions=[
        {
            "name": "output_relevant_linespans",
            "description": """Inputs an array of linespans. prompt_gist is a three word snake_case prompt summary""",
            "parameters": {
                "type": "object",
                "properties": {
                    "linespans": {
                        "type": "array",
                        "items": {
                            "type": "object",
                            "properties": {
                                "start": {"type": "integer"},
                                "stop": {"type": "integer"}
                            },
                            "required": ["start", "stop"]
                        }
                    },
                    "prompt_gist": {
                        "type":"string"
                    }
                },
                "required": ["linespans"]
            },
        }]
        response = completion_with_backoff(
            model="gpt-3.5-turbo-0613",
            # model="gpt-4-0613",
            messages=messages,
            functions = functions,
            function_call="auto",
        )

        message = response["choices"][0]["message"]

        # Check if the model wants to call a function
        if message.get("function_call"):
            function_args = json.loads(message["function_call"]["arguments"])
            linespans = function_args.get("linespans")
            print(linespans)
            if prompt_gist is None:
                prompt_gist=function_args.get("prompt_gist"),
            all_linespans.extend(linespans)

    # Merge all overlapping linespans
    print(all_linespans)
    merged_linespans = merge_linespans(all_linespans)

    output_relevant_linespans(
                linespans=merged_linespans,
                prompt_gist=prompt_gist,
                prompt=prompt
            )

    return merged_linespans

get_relevant_lines("Only include lines that explain or demonstrate how to access a specifically named entity within a SYSTEM C macro definition (without using get_mut).", file_content)