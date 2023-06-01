import subprocess
import logging
import os

# Configure logging
logging.basicConfig(filename='error_log.txt', level=logging.ERROR)

# Path to your project directory
project_dir = "/home/aeri/ant/dojo/booksim/game/"
project_build_dir = os.path.join(project_dir, "build")

def run_command(command):
    process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()

    if process.returncode != 0:
        logging.error(f'Error running command: {command} \n {stderr}')
        return False, stdout, stderr
    else:
        return True, stdout, stderr

def test_compile():
    success, stdout, stderr = run_command(f"cd {project_build_dir} && cmake .. && make")
    if not success:
        print(f"Compile stderr:\n{stderr.decode()}")
    else:
        print(f"Compile stdout:\n{stdout.decode()}")
    return success

def test_run():
    success, stdout, stderr = run_command(f"cd {project_build_dir} && ./agent")
    if not success:
        print(f"Run stderr:\n{stderr.decode()}")
    else:
        print(f"Run stdout:\n{stdout.decode()}")
    return success

def run_tests():
    if not test_compile():
        print("Compilation test failed. Check error_log.txt for details.")
        return

    if not test_run():
        print("Run test failed. Check error_log.txt for details.")
        return

    print("All tests passed!")

if __name__ == "__main__":
    run_tests()
