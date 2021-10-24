import os, subprocess, platform, pkg_resources, click, pathlib
from ede.ede_ast.ede_ir import OP_CODE_SIZE, WORD_SIZE, OpCode

@click.command()
@click.option('-d', '--directory', type=click.Path(exists=False, file_okay=False, resolve_path=True), default='bin/', help="Sets the output directory for the executable.")
@click.option('-t', '--tests', is_flag=True, help="Whether or not the test suite should be built.")
def cli(directory: str, tests: bool):
    pathlib.Path(directory).mkdir(parents=True, exist_ok=True)

    macros = {
        "OP_CODES": ', '.join([op_code.name for op_code in OpCode if '_' not in op_code.name]),
        "INSTRUCTION_SIZES": ', '.join([str(op_code.get_instr_byte_size()) for op_code in OpCode if '_' not in op_code.name]),
        "WORD_SIZE": WORD_SIZE,
        "OP_CODE_SIZE": OP_CODE_SIZE
    }

    resources = {
        "main.cpp": pkg_resources.resource_filename('evm', 'src/main.cpp'),
        "template.h": pkg_resources.resource_filename('evm', 'src/template.h'),
        "thread.cpp": pkg_resources.resource_filename('evm', 'src/thread.cpp'),
        "instructions.cpp": pkg_resources.resource_filename('evm', 'src/instructions.cpp'),
        "vm.cpp": pkg_resources.resource_filename('evm', 'src/vm.cpp'),
        "tests.cpp": pkg_resources.resource_filename('evm', 'src/tests.cpp'),
    }

    with open(resources["template.h"], "w+") as output:
        output.write("#pragma once\n")
        output.writelines([f"\n#define {macro} {value}" for macro, value in macros.items()])

    input_files = [resources["main.cpp"], resources["thread.cpp"], resources["vm.cpp"], resources["instructions.cpp"]]
    evm_output_path = os.path.join(directory, "evm" + (".exe" if platform.system() == "Windows" else "")) 
    subprocess.call(["g++", "-std=c++17", "-fdiagnostics-color=always", "-g"] + input_files + ["-o", evm_output_path])

    if tests:
        input_files.remove(resources["main.cpp"])
        input_files.append(resources["tests.cpp"])
        tests_output_path = os.path.join(directory, "evm_tests" + (".exe" if platform.system() == "Windows" else "")) 
        subprocess.call(["g++", "-std=c++17", "-fdiagnostics-color=always", "-g"] + input_files + ["-o", tests_output_path])