import os
import random
import string

def generate_random_string(length):
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for _ in range(length))

def create_directory(dir_name):
    os.makedirs(dir_name, exist_ok=True)
    print(f"Directory {dir_name} created")

def create_file(dir_name, file_name, message):
    with open(f"{dir_name}/{file_name}", 'w') as file:
        file.write(message)
    print(f"File {file_name} created with message: {message}")

def read_file(dir_name, file_name):
    with open(f"{dir_name}/{file_name}", 'r') as file:
        print(f"Read from file {file_name}: {file.read()}")

def rename_file(dir_name, old_file_name, new_file_name):
    os.rename(f"{dir_name}/{old_file_name}", f"{dir_name}/{new_file_name}")
    print(f"File {old_file_name} renamed to {new_file_name}")

def delete_file(dir_name, file_name):
    os.remove(f"{dir_name}/{file_name}")
    print(f"File {file_name} deleted")

def delete_directory(dir_name):
    os.rmdir(dir_name)
    print(f"Directory {dir_name} deleted")

def execute_quest_1():
    dir_name = generate_random_string(5)
    file_name = generate_random_string(5) + ".txt"
    new_file_name = generate_random_string(5) + ".txt"
    message = generate_random_string(20)
    
    create_directory(dir_name)
    create_file(dir_name, file_name, message)
    read_file(dir_name, file_name)
    rename_file(dir_name, file_name, new_file_name)
    delete_file(dir_name, new_file_name)
    delete_directory(dir_name)

execute_quest_1()
