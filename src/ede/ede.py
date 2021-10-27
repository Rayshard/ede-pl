import os, subprocess
from typing import List
import click, json
from .ede_ast.ede_ir import Instruction, OpCode
from .ede_ast.ede_module import Module
from .ede_ast.ede_visitors.ede_cfg_visitor import CFG, CFGVisitor
from .ede_ast.ede_visitors.ede_execution_visitor import ExecutionVisitor
from .ede_ast.ede_visitors.ede_ir_visitor import IRVisitor, ModuleIRBuilder
from .ede_ast.ede_visitors.ede_typecheck_visitor import TypecheckVisitor
from .ede_ast.ede_typesystem import TCContext
from .ede_ast.ede_visitors.ede_json_visitor import JsonVisitor
from . import ede_parser
from .interpreter import ExecContext

# TODO: Comment File

def print_exit(msg: str, code: int):
    click.echo(msg) # type: ignore
    exit(code)

@click.command()
@click.option('-s', '--simulate', is_flag=True, help="Simulates execution of the input files.")
@click.option('--ast', is_flag=True, help="Outputs the AST and saves to a file.")
@click.option('--cfg', is_flag=True, help="Outputs the CFG and saves to a file.")
@click.option('--ec', is_flag=True, help="Prints the final execution context to the standard output.")
@click.argument('file_paths', type=click.Path(exists=True, resolve_path=True), required=True, nargs=-1)
def cli(simulate: bool, ast: bool, cfg: bool, ec: bool, file_paths: List[str]):
    if not all([os.path.splitext(file_path)[1] == '.ede' for file_path in file_paths]):
            print_exit('Only EDE files are allowed!', 1)

    modules : List[Module] = []

    for file_path in file_paths:
        parse_result = ede_parser.parse_file(file_path)
        if parse_result.is_error():
            print_exit(parse_result.error().get_output_msg(file_path), 1)

        module = parse_result.get()

        if ast:
            with open(file_path + '.json', 'w+') as f_ast:
                json.dump({
                    "source": file_path,
                    "ast": JsonVisitor.visit(module)
                }, f_ast, indent=4, sort_keys=False)

        if cfg:
            with open(file_path + '.dot', 'w+') as f_cfg:
                output_cfg = CFG()
                CFGVisitor.visit(module, output_cfg, None)

                f_cfg.write(output_cfg.to_dot().to_string())

        tc_res = TypecheckVisitor.visit(module, TCContext())
        if tc_res.is_error():
            print_exit(tc_res.error().get_output_msg(modules[0].file_path), 1)

        modules.append(module)

    if simulate:                   
        exec_ctx = ExecContext()
        exec_res = ExecutionVisitor.visit(modules[0], exec_ctx)
        
        if ec:
            click.echo("=================== Execution Context ====================") # type: ignore
            click.echo(json.dumps(JsonVisitor.visit(exec_ctx), indent=4, sort_keys=False)) # type: ignore
            click.echo("=================== ================= ====================") # type: ignore
        
        print_exit(str(exec_res), exec_res.is_exception())
    else:
        elements: List[str | Instruction] = []
        
        for module in modules:
            ir_builder = ModuleIRBuilder()
            IRVisitor.visit(module, ir_builder)
            module_ir = ir_builder.get_ir()
            
            for elem in module_ir["code"]:
                match elem:
                    case label if isinstance(label, str):
                        elements.append("@" + label + ":")
                    case [op_code] if isinstance(op_code, str):
                        elements.append(Instruction(OpCode[op_code], []))
                    case [op_code, *operands] if isinstance(op_code, str):
                        elements.append(Instruction(OpCode[op_code], list(operands)))
                    case _:
                        print_exit("Unknown Bytecode Element: {elem}", 1)

        bytecode_file_path = modules[0].file_path + 'asm'
        with open(bytecode_file_path, 'w+') as f_bc:
            for elem in elements:
                f_bc.write(("" if isinstance(elem, str) else "\t") + str(elem) + '\n')

        subprocess.call(["bin/evm.exe", bytecode_file_path])
