import openai
import json

openai.api_key = "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"

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
    pass # TODO: implement function logic
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
    
    return function_string, json_schema


def implement_function(language, function):
    print("Implement Function")
    messages=[{"role": "user", "content": f"Use {language} programming language. Implement the function\n{function}"}]
    response = openai.ChatCompletion.create(
        model="gpt-4",
        messages=messages,
    )
    message = response["choices"][0]["message"]
    print(message['content'])
    return message

# Step 1, send model the user query and what functions it has access to
def run_conversation():
    messages=[{"role": "assistant", "content": "To create a GPT function, first call create_gpt_function_definition, then with the fuction result, call implement_function in Python"}, {"role": "user", "content": "Keep the function name concise and minimalist, DRY. Create a GPT function that gets all function names from a C file."}]
    functions=[
    {
        "name": "create_gpt_function_definition",
        "description": """Outputs a Python function def callable by GPT. Includes type hints and matching JSON schema.""",
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
        # "name": "implement_function",
        # "description": """Implements logical and correct code for an existing function placeholder. Outputs the entire function as a string. Make sure the function placeholder is generated first before calling this function.""", # TODO: Line insert vs rewrite
        # "parameters": {
        #     "type": "object",
        #     "properties": {
        #         "language":{"type":"string"},
        #         "function":{ "type": "string" }, 
        #     },
        #     "required": ["function"],
        # },
    }]
    response = openai.ChatCompletion.create(
        model="gpt-3.5-turbo-0613",
        messages=messages,
        functions = functions,
        function_call="auto",
    )

    message = response["choices"][0]["message"]

    # Step 2, check if the model wants to call a function
    if message.get("function_call"):
        available_functions = {
            "create_gpt_function_definition": create_gpt_function_definition,
            "implement_function":implement_function
        }  # only one function in this example, but you can have multiple
        function_name = message["function_call"]["name"]
        print(function_name)
        function_to_call = available_functions[function_name]
        function_args = json.loads(message["function_call"]["arguments"])
        function_response = None
        if function_name == "create_gpt_function_definition":
            function_response = create_gpt_function_definition(
                name=function_args.get("name"),
                description=function_args.get("description"),
                params=function_args.get("params"),
                param_type_hints=function_args.get("param_type_hints"),
            )
            # implement_function("Python 3.9", f"{function_response}")
        # elif function_name == "implement_function":
        #     function_response = implement_function(
        #         language=function_args.get("language"),
        #         function=function_args.get("function"),
        #     )


        # # Step 3, call the function
        # # Note: the JSON response from the model may not be valid JSON
        # function_response = create_gpt_function(
        #     name=function_args.get("name"),
        #     description=function_args.get("description"),
        #     params=function_args.get("params"),
        # )

        # # Step 4, send model the info on the function call and function response
        # second_response = openai.ChatCompletion.create(
        #     model="gpt-3.5-turbo-0613",
        #     messages=[
        #         {"role": "user", "content": "What is the weather like in boston?"},
        #         message,
        #         {
        #             "role": "function",
        #             "name": function_name,
        #             "content": function_response,
        #         },
        #     ],
        # )
        # return second_response

print(run_conversation())