import sys
import lexer, ede_parser
from ede_ast.ede_type import Environment
from ede_ast.ede_ast import ExecContext

# TODO: Comment File

if __name__ == '__main__':
    args = sys.argv[1:]
    assert len(args) != 0, "Expected file path"

    file_path = args[0]
    
    env = Environment()
    ctx = ExecContext()

    reader = ede_parser.TokenReader(lexer.tokenize(lexer.Reader("3 + 7*2")).get())
    result = ede_parser.parse(reader)

    if result.is_error():
        print(result)
        exit(1)
    else:
        print(result.get().to_string(0))
        print(result.get().execute_in(env, ctx))
        exit(0)

    # with open(file_path) as f:
    #     result = lexer.tokenize(lexer.Reader(f.read()))

    #     if result.is_error():
    #         print(result)
    #         exit(1)
    #     else:
    #         exit(0)

