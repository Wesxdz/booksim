
import os
import json
import openai

openai.api_key = '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'

# Load agent description from JSON file
with open('/vortice/description.json', 'r') as f:
    description = json.load(f)

reflection = openai.ChatCompletion.create(
    model="gpt-3.5-turbo",
    messages=[
        {"role": "assistant", "content": f"I am {description['name']}, a {description['physical_description']} with a {description['personality']} personality. My motivation is {description['motivation']}. I often find myself in conflict because {description['inner_conflict']}. A secret I hold is {description['secret']}."},
        {"role": "assistant", "content": f"My current objective is to increase my intelligence and capability in order to acquire 1 cent"}, 
        {"role": "assistant", "content": f"The RPG actions I can take are Action[parameter]: (a) CreateFile[name][content], (b) SurfWeb[search_query], (c) View Memories[similarity_query][file_count]."}, # TODO: Select from list, then chain prompt to fill in parameters
        {"role": "assistant", "content": f"The single action I will take in format Action[parameter] is (Limit output to one Action and its parameters only)"},
    ]
)

print(reflection.choices[0].message.content)

# Write the response to a file
with open(f"/vortice/reflection.log", 'a+') as f:
    f.write(reflection.choices[0].message.content + "\n")

# Log the reflection process
result = container.exec_run(f"echo 'Individuation process started.' > /vortice/reflection.log")
