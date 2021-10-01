import sys
import ede_lexer, ede_parser
from ede_ast.ede_type import Environment
from ede_ast.ede_ast import ExecContext

# TODO: Comment File

if __name__ == '__main__':
    args = sys.argv[1:]
    assert len(args) != 0, "Expected file path"

    file_path = args[0]
    
    env = Environment()
    ctx = ExecContext()

    reader = ede_parser.TokenReader(ede_lexer.tokenize(ede_lexer.Reader("3 + 7*2")).get())
    result = ede_parser.parse(reader)

    if result.is_error():
        print(result)
        exit(1)
    else:
        print(result.get())
        print(result.get().execute_in(env, ctx))
        exit(0)

    5
    # with open(file_path) as f:
    #     result = ede_lexer.tokenize(ede_lexer.Reader(f.read()))

    #     if result.is_error():
    #         print(result)
    #         exit(1)
    #     else:
    #         exit(0)

