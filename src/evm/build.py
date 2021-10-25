import os, subprocess, platform, pkg_resources, click, pathlib

@click.command()
@click.option('-d', '--directory', type=click.Path(exists=False, file_okay=False, resolve_path=True), default='bin/', help="Sets the output directory for the executable.")
@click.option('-t', '--tests', is_flag=True, help="Whether or not the test suite should be built.")
def cli(directory: str, tests: bool):
    pathlib.Path(directory).mkdir(parents=True, exist_ok=True)

    resources = {
        "main.cpp": pkg_resources.resource_filename('evm', 'src/main.cpp'),
        "program.cpp": pkg_resources.resource_filename('evm', 'src/program.cpp'),
        "thread.cpp": pkg_resources.resource_filename('evm', 'src/thread.cpp'),
        "instructions.cpp": pkg_resources.resource_filename('evm', 'src/instructions.cpp'),
        "vm.cpp": pkg_resources.resource_filename('evm', 'src/vm.cpp'),
        "tests.cpp": pkg_resources.resource_filename('evm', 'src/tests.cpp'),
    }

    input_files = [path for name, path in resources.items() if name != "tests.cpp"]
    evm_output_path = os.path.join(directory, "evm" + (".exe" if platform.system() == "Windows" else "")) 
    subprocess.call(["g++", "-std=c++17", "-fdiagnostics-color=always", "-g"] + input_files + ["-o", evm_output_path])

    if tests:
        input_files.remove(resources["main.cpp"])
        input_files.append(resources["tests.cpp"])
        tests_output_path = os.path.join(directory, "evm_tests" + (".exe" if platform.system() == "Windows" else "")) 
        subprocess.call(["g++", "-std=c++17", "-fdiagnostics-color=always", "-g"] + input_files + ["-o", tests_output_path])