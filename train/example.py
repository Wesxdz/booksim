from create_file_meta import create_file
import openai
import subprocess
openai.api_key = "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"

# prepass = openai.ChatCompletion.create(
#     model="gpt-3.5-turbo",
#     messages=[
#         {"role": "user", "content": f"""To create a file, output Python code 

#         from create_file_meta import create_file
#         create_file('/home/aeri/ant/dojo/booksim/team/work/, file_name.type)
        
#         Output Python codeblock that creates a file named test.py and prints Hello, booksim
#         Only output syntactically correct Python code that will run directly in Python interpreter. Do not include a tab to indicate Python code.""" },
#     ]
# )

# # Create file in /home/aeri/ant/dojo/booksim/team/work/
# create_file('/home/aeri/ant/dojo/booksim/team/work/', 'open_ai_output.py', prepass.choices[0].message.content)
file_path = '/home/aeri/ant/dojo/booksim/team/work/open_ai_output.py'
with open(file_path, 'r') as file:
    file_contents = file.read()
exec(file_contents)