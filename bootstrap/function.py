import openai
import json
import subprocess
import os

def get_py_filenames(directory):
    py_files = [file_name for file_name in os.listdir(directory) if file_name.endswith(".py")]
    return py_files

openai.api_key = "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"

def fix_indentation(file_path):
    # Command to run
    cmd = ["autopep8", "--in-place", "--aggressive", "--aggressive", file_path]

    # Run the command
    subprocess.run(cmd)

def get_json_schema_type(python_type):
    type_mapping = {
        'str': 'string',
        'int': 'number',
        'float': 'number',
        'dict': 'object',
        'list': 'array',
        'bool': 'boolean',
        'NoneType': 'null'
    }
    return type_mapping.get(python_type, 'object')

def create_gpt_function_definition(name, description, params, param_type_hints):
    if len(params) != len(param_type_hints):
        raise ValueError('Lengths of "params" and "param_type_hints" must be the same.')
    
    params_str = ', '.join(f'{p}: {t}' for p, t in zip(params, param_type_hints))
    
    # Create function string
    function_string = f'''def {name}({params_str}):
    """
    {description}
    """
    pass # TODO: implement function body
    '''
    print(function_string)

    # Create parameters schema
    json_types = [get_json_schema_type(t) for t in param_type_hints]
    params_schema = [{"name": p, "type": t} for p, t in zip(params, json_types)]

    # Create JSON schema
    json_schema = {
        "name": name,
        "description": description,
        "parameters": {
            "type": "object",
            "properties": {p: {"type": t} for p, t in zip(params, json_types)},
        },
        "required": params
    }

    print("\nJSON schema:")
    print(json.dumps(json_schema, indent=4))
    
    # TODO: Automated test
    return function_string, json_schema


def implement_python_function(function_signature, function_body, pip_packages):
    # Create a list to store all lines of the function
    function_lines = []

    # Add the import statements for the pip packages
    if pip_packages:
        for package in pip_packages:
            function_lines.append(f"import {package}")

        # Add a blank line for clarity
        function_lines.append("")

    # Split the function signature into lines
    print(function_signature)
    function_signature_lines = function_signature.split("\n")

    function_lines.append(function_signature_lines[0])

    # Split the function body into lines and add them with a tab indentation
    function_body_lines = function_body.split("\n")
    for line in function_body_lines:
        function_lines.append(f"    {line}")

    # Join all the function lines into a single string
    function_string = "\n".join(function_lines)

    # Print the function
    print(function_string)

    return function_string


# Step 1, send model the user query and what functions it has access to
def run_conversation():
    with open('../team/work/impasse/description.json', 'r') as f:
        description = json.load(f)
    print(get_py_filenames('actions'))
    response = openai.ChatCompletion.create(
        model="gpt-4",
        messages=[
            # {"role": "system", "content": f"Artificial super intelligence training environment 'Book Simulator': a transhuman/AI RPG"},
            {"role": "assistant", "content": f"I am {description['name']}, a virtual entity appearing as {description['physical_description']} with a {description['personality']} personality. My motivation is {description['motivation']}."},
            {"role": "assistant", "content": f"My current objective is to increase my intelligence and capability in order to acquire 1 cent"}, 
            # {"role":"assistant", "content":"Create a plan to acquire 1 cent"}
            {"role": "assistant", "content": f"Prefer using libraries where possible. Focus on AI system integration of existing code. Do not call other functions."},
            {"role": "user", "content": "Describe in 120 characters or fewer, but do not code, a simple capability which could be implemented in a single Python function to help generate or iterate on an existing codebase. For example: parse a C file and extract functions using pyclang"},
            {"role": "assistant", "content": f"DO NOT replicate existing action functionality Existing actions: {get_py_filenames('actions')}"},
        ],
    )
    capability = response["choices"][0]["message"]
    print(capability)

# {"role": "assistant", "content": "To create a GPT function, first call create_gpt_function_definition, then with the fuction result, call implement_function in Python"}, 
    # messages=[{"role": "user", "content": "Keep the function name concise and minimalist, DRY. Create a GPT function that gets all function names from a C file."}]
    messages=[{"role": "user", "content": f"Keep the function name concise and minimalist, DRY. Create a GPT function to {capability}"}]
    # messages=[{"role": "user", "content": "Keep the function name concise and minimalist, DRY. Create a GPT function that asks the user for their name and prints it."}]
    functions=[
    {
        "name": "create_gpt_function_definition",
        "description": """Outputs a Python function def callable by GPT. Includes type hints (use lowercase list) and matching JSON schema.""",
        "parameters": {
            "type": "object",
            "properties": {
                "name":{ "type": "string" },
                "description":{ "type": "string" },
                "params":{"type": "array", "items": { "type": "string" }},
                "param_type_hints":{"type": "array", "items": { "type": "string" }}
            },
            "required": ["name", "description", "params", "param_type_hints"],
        },
    }]
    response = openai.ChatCompletion.create(
        model="gpt-3.5-turbo-0613",
        # model="gpt-4-0613",
        messages=messages,
        functions = functions,
        function_call="auto",
    )

    message = response["choices"][0]["message"]

    created_function_name = None
    function_string = None
    json_schema = None
    # Step 2, check if the model wants to call a function
    if message.get("function_call"):
        available_functions = {
            "create_gpt_function_definition": create_gpt_function_definition,
        } 
        function_name = message["function_call"]["name"]
        print(function_name)
        function_to_call = available_functions[function_name]
        function_args = json.loads(message["function_call"]["arguments"])
        function_response = None
        if function_name == "create_gpt_function_definition":
            created_function_name = function_args.get("name")
            function_string, json_schema = create_gpt_function_definition(
                name=function_args.get("name"),
                description=function_args.get("description"),
                params=function_args.get("params"),
                param_type_hints=function_args.get("param_type_hints"),
            )

    messages.append(message)
    messages.append({"role": "user", "content": f"Use Python 3.9. Output error-free properly formatted code to implement the function body. Output code as a string which can be JSON decoded."})
    messages.append({"role": "user", "content": f"function_signature parameter for implement_python_function: \n{function_string}"})
    
    functions=[
    {
        "name": "implement_python_function",
        "description": """Implements logical and correct code for a single existing function body given a placeholder signature. Provide a list of pip dependencies in pip_packages parameter, Import statements provided in pip_packages parameter will be automatically imported (so don't use import statements).""",
        "parameters": {
            "type": "object",
            "properties": {
                "function_signature":{ "type": "string" }, 
                "function_body":{ "type": "string" }, 
                "pip_packages":{"type": "array", "items": { "type": "string" }},
            },
            "required": ["function_signature", "function_body", "pip_packages"],
        },
    }]
    response = openai.ChatCompletion.create(
        model="gpt-4-0613",
        messages=messages,
        functions=functions
    )
    message = response["choices"][0]["message"]
    
    complete_function = None
    if message.get("function_call"):
        available_functions = {
            "implement_python_function": implement_python_function,
        }  # only one function in this example, but you can have multiple
        function_name = message["function_call"]["name"]
        print(function_name)
        function_to_call = available_functions[function_name]
        try:
            function_args = json.loads(message["function_call"]["arguments"])
        except json.JSONDecodeError as ex:
            print(f'Failed to decode JSON: {ex}')
        function_response = None
        if function_name == "implement_python_function":
            complete_function = implement_python_function(
                function_signature=function_args.get("function_signature"),
                function_body=function_args.get("function_body"),
                pip_packages=function_args.get("pip_packages"),
            )

    with open(f"actions/{created_function_name}.py", 'w') as f:
        f.write(complete_function)

    fix_indentation(f"actions/{created_function_name}.py")

    with open(f"actions/{created_function_name}_schema.json", 'w') as f:
        f.write(json.dumps(json_schema))

print(run_conversation())