
import os
from typing import List
import click, json
from ede_ast.ede_visitors.ede_execution_visitor import ExecutionVisitor
import ede_lexer, ede_parser
from ede_ast.ede_typesystem import Environment
from ede_ast.ede_visitors.ede_json_visitor import JsonVisitor
from interpreter import ExecContext

# TODO: Comment File

@click.command()
@click.option('-s', '--simulate', is_flag=True, help="Simulates execution of the input files.")
@click.option('--ast', is_flag=True, help="Outputs the AST and saves to a file.")
@click.argument('file_paths', type=click.Path(exists=True, resolve_path=True), required=True, nargs=-1)
def cli(simulate: bool, ast: bool, file_paths: List[str]):
    max_file_path_len = max([len(path) for path in file_paths]) + 1
    if simulate:
        click.echo("=" * max_file_path_len) # type: ignore
        click.echo(" " * (max_file_path_len // 3) + "Simulating files:\n\n" + '\n'.join(file_paths))# type: ignore
        click.echo("=" * max_file_path_len + "\n")# type: ignore

        if not all([os.path.splitext(file_path)[1] == '.ede' for file_path in file_paths]):
            click.echo('Only EDE files are allowed!') # type: ignore
            exit(1)

        for file_path in file_paths:
            file_name = os.path.splitext(file_path)[0]
            
            with open(file_path) as f:
                lex_result = ede_lexer.tokenize(ede_lexer.Reader(f.read()))
                if lex_result.is_error():
                    print(lex_result.error().get_output_msg(file_path))
                    exit(1)
                
                reader = ede_parser.TokenReader(lex_result.get())
                if reader.is_eof():
                    continue
                
                parse_result = ede_parser.parse_module(file_name, reader)
                if parse_result.is_error():
                    print(parse_result.error().get_output_msg(file_path))
                    exit(1)
                
                if ast:
                    with open(file_path + '.json', 'w+') as f_ast:
                        json.dump({
                            "source": file_path,
                            #"ast": parse_result.get().to_json()
                            "ast": JsonVisitor.visit(parse_result.get())
                        }, f_ast, indent=4, sort_keys=False)
                        
                env = Environment()
                ctx = ExecContext()

                # env.declare('int', EnvEntry(EnvEntryType.TYPENAME, EdeInt, Position()), False)
                # env.declare('string', EnvEntry(EnvEntryType.TYPENAME, EdeString, Position()), False)
                # env.declare('char', EnvEntry(EnvEntryType.TYPENAME, EdeChar, Position()), False)
                # env.declare('bool', EnvEntry(EnvEntryType.TYPENAME, EdeBool, Position()), False)
                # env.declare('unit', EnvEntry(EnvEntryType.TYPENAME, EdeUnit, Position()), False)

                exec_res = ExecutionVisitor.visit_in(parse_result.get(), env, ctx)
                if exec_res.is_error():
                    print(exec_res.error().get_output_msg(file_path))
                
                print(exec_res.get())

                print("=================== Execution Context ====================")
                print(ctx)
                print("=================== ================= ====================")
    else:
        click.echo(f"Compiling files: {file_paths}") # type: ignore

if __name__ == '__main__':
    cli()
