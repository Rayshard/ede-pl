# type: ignore
import sys
import subprocess
import os
import platform
import pkg_resources
import click
import pathlib

EXECUTABLE_FILE_NAME = "evm" + (".exe" if platform.system() == "Windows" else "")

def run():
    sys.exit(subprocess.call(["bin/" + EXECUTABLE_FILE_NAME] + sys.argv[1:]))


@click.command()
@click.option('-o', '--output', type=click.Path(exists=False, file_okay=False, resolve_path=True), default='bin/', help="Sets the output directory for the executable.")
@click.option('-t', '--tests', is_flag=True, help="Whether or not the test suite should be built.")
@click.option('-d', '--debug', is_flag=True, help="Whether or not to build with debug features.")
def build(output: str, tests: bool, debug: bool):
    pathlib.Path(output).mkdir(parents=True, exist_ok=True)

    resources = {}

    try:
        resources = {
            "build.h": pkg_resources.resource_filename('evm', 'build.h'),
            "cpps": {
                "main.cpp": pkg_resources.resource_filename('evm', 'src/main.cpp'),
                "program.cpp": pkg_resources.resource_filename('evm', 'src/program.cpp'),
                "thread.cpp": pkg_resources.resource_filename('evm', 'src/thread.cpp'),
                "instructions.cpp": pkg_resources.resource_filename('evm', 'src/instructions.cpp'),
                "vm.cpp": pkg_resources.resource_filename('evm', 'src/vm.cpp'),
            }
        }
    except:
        resources = {
            "build.h": "src/evm/build.h",
            "cpps": {
                "main.cpp": 'src/evm/src/main.cpp',
                "program.cpp": 'src/evm/src/program.cpp',
                "thread.cpp": 'src/evm/src/thread.cpp',
                "instructions.cpp": 'src/evm/src/instructions.cpp',
                "vm.cpp": 'src/evm/src/vm.cpp',
            }
        }

    if tests:
        resources.get("cpps")["tests.cpp"] = 'src/evm/src/tests.cpp'

    # Set build flags
    with open(resources["build.h"], 'w') as f:
        flags = {
            "BUILD_WITH_TESTS": tests,
            "BUILD_DEBUG": debug
        }

        for flag, present in flags.items():
            if present:
                f.write("#define " + flag + "\n")

        if flags["BUILD_DEBUG"]:
            f.write("#ifdef BUILD_DEBUG\n")
            f.write("\textern bool PRINT_STACK_AFTER_THREAD_END;\n")
            f.write("\textern bool PRINT_STACK_AFTER_INSTR_EXECUTION;\n")
            f.write("\textern bool PRINT_INSTR_BEFORE_EXECUTION;\n")
            f.write("#endif\n")

    # Build executable
    build_call_args = ["g++", "-std=c++17", "-fdiagnostics-color=always", "-g"]
    input_files = [path for name, path in resources.get("cpps").items()]
    evm_output_path = os.path.join(
        output, EXECUTABLE_FILE_NAME)
    subprocess.call(build_call_args + ["-o", evm_output_path] + input_files)
