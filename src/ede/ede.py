# type: ignore

import sys, click, json
import ede_lexer, ede_parser
from ede_ast.ede_typesystem import Environment
from ede_ast.ede_ast import ExecContext
from ede_token import TokenType

# TODO: Comment File

@click.command()
@click.option('-s', '--simulate', is_flag=True, help="Simulates execution of the input files.")
@click.option('--ast', is_flag=True, help="Outputs the AST and saves to a file.")
@click.argument('file_paths', type=click.Path(exists=True, resolve_path=True), required=True, nargs=-1)
def cli(simulate, ast, file_paths):
    max_file_path_len = max([len(path) for path in file_paths]) + 1
    if simulate:
        click.echo("=" * max_file_path_len)
        click.echo(" " * (max_file_path_len // 3) + "Simulating files:\n\n" + '\n'.join(file_paths))
        click.echo("=" * max_file_path_len + "\n")

        for file_path in file_paths:
            with open(file_path) as f:
                lex_result = ede_lexer.tokenize(ede_lexer.Reader(f.read()))
                if lex_result.is_error():
                    print(lex_result)
                    exit(1)
                
                reader = ede_parser.TokenReader(lex_result.get())
                if reader.peek().type == TokenType.EOF:
                    continue
                
                parse_result = ede_parser.parse(reader)
                if parse_result.is_error():
                    print(parse_result)
                    exit(1)
                
                if ast:
                    with open(file_path + '.ast', 'w+') as f_ast:
                        json.dump({
                            "source": file_path,
                            "ast": parse_result.get().to_json()
                        }, f_ast, indent=4, sort_keys=False)
                        
                env = Environment()
                ctx = ExecContext()
                print(parse_result.get().execute_in(env, ctx))
    else:
        click.echo(f"Compiling files: {file_paths}")

if __name__ == '__main__':
    cli()
