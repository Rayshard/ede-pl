import sys
import lexer
from ede_utils import Position
from ede_ast.ede_literal import IntLiteral, StringLiteral
from ede_ast.ede_binop import BinopExpr, BinopType
from ede_ast.ede_type import Environment
from ede_ast.ede_ast import ExecContext

if __name__ == '__main__':
    args = sys.argv[1:]
    assert len(args) != 0, "Expected file path"

    file_path = args[0]
    
    env = Environment()
    ctx = ExecContext()

    left = IntLiteral(Position(1, 1), 10)
    right = IntLiteral(Position(1, 1), 15)
    binop = BinopExpr(Position(1, 1), left, right, BinopType.ADD)

    lefts = StringLiteral(Position(1, 1), "Hello")
    rights = StringLiteral(Position(1, 1), "World")
    binops = BinopExpr(Position(1, 1), lefts, rights, BinopType.ADD)

    print(binops.execute_in(env, ctx))

    with open(file_path) as f:
        result = lexer.tokenize(lexer.Reader(f.read()))

        if result.is_error():
            print(result)
            exit(1)
        else:
            exit(0)

