# type: ignore
import sys
import subprocess
import os
import glob
import platform
import pkg_resources
import click
import pathlib

EXECUTABLE_FILE_NAME = "evm" + \
    (".exe" if platform.system() == "Windows" else "")


def run():
    sys.exit(subprocess.call(["bin/" + EXECUTABLE_FILE_NAME] + sys.argv[1:]))


@click.command()
@click.option('-o', '--output', type=click.Path(exists=False, file_okay=False, resolve_path=True), default='bin/', help="Sets the output directory for the executable.")
@click.option('-t', '--tests', is_flag=True, help="Whether or not the test suite should be built.")
@click.option('-d', '--debug', is_flag=True, help="Whether or not to build with debug features.")
@click.option('--debug-heap', is_flag=True, help="Whether or not to build with debugging code for heap.cpp.")
def build(output: str, tests: bool, debug: bool, debug_heap: bool):
    pathlib.Path(output).mkdir(parents=True, exist_ok=True)

    resources = {}
    cpp_file_names = [os.path.basename(file)
                      for file in glob.glob("src/evm/src/*.cpp")]

    try:
        resources = {
            "build.h": pkg_resources.resource_filename('evm', 'build.h'),
            "cpps": {file_name: pkg_resources.resource_filename('evm', os.path.join('src/', file_name))
                     for file_name in cpp_file_names}
        }
    except:
        resources = {
            "build.h": "src/evm/build.h",
            "cpps": {file_name: os.path.join('src/evm/src/', file_name) for file_name in cpp_file_names}
        }

    if not tests:
        del resources.get("cpps")["tests.cpp"]

    # Set build flags
    with open(resources["build.h"], 'w') as f:
        flags = {
            "BUILD_WITH_TESTS": tests,
            "BUILD_DEBUG": debug,
            "BUILD_DEBUG_HEAP": debug_heap
        }

        for flag, present in flags.items():
            if present:
                f.write("#define " + flag + "\n")

        if flags["BUILD_DEBUG"]:
            f.write("#ifdef BUILD_DEBUG\n")
            f.write("\textern bool PRINT_STACK_AFTER_THREAD_END;\n")
            f.write("\textern bool PRINT_STACK_AFTER_INSTR_EXECUTION;\n")
            f.write("\textern bool PRINT_INSTR_BEFORE_EXECUTION;\n")
            f.write("\textern bool PRINT_HEAP_AFTER_PROGRAM_END;\n")
            f.write("#endif\n")

    # Build executable
    build_call_args = ["g++", "-std=c++17", "-fdiagnostics-color=always", "-g"]
    input_files = [path for name, path in resources.get("cpps").items()]
    evm_output_path = os.path.join(
        output, EXECUTABLE_FILE_NAME)
    subprocess.call(build_call_args + ["-o", evm_output_path] + input_files)
