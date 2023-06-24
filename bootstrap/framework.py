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


# Step 1, send model the user query and what functions it has access to
def run_conversation():
    capabilities = [
        "return entities created prior to world start/progress/loop",
        "return component struct data for component type string",
        "return all component types in an array of strings",
        "return all systems function names in an array of strings",
        "return all system function names in a pipeline phase",
        "return code snippet for system name as string",
        "return all observer function names in an array of strings",
        "create a relationship type",
        "create a system of a given name with array of component type strings",
        "modify an existing system/observer function to add or remove component from signature",
        "create a system",
        "add an existing relationship type between two entities",
    ]
    # capabilities = [
    #     "return an array of an objects with two ints representing lines of a txt document that are relevant to a prompt"
    # ]
    for capability in capabilities:
        messages=[{"role": "user", "content": f"Keep the function name concise and minimalist, DRY. Create a GPT function to {capability}"}]
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

        with open(f"actions/{created_function_name}.py", 'w') as f:
            f.write(function_string)

        fix_indentation(f"actions/{created_function_name}.py")

        with open(f"actions/{created_function_name}_schema.json", 'w') as f:
            f.write(json.dumps(json_schema))

print(run_conversation())