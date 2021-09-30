import sys
import lexer
from ede_utils import Position
from ede_ast.ede_literal import IntLiteral, StringLiteral
from ede_ast.ede_expr import BinopExpr, BinopType
from ede_ast.ede_type import Environment

if __name__ == '__main__':
    args = sys.argv[1:]
    assert len(args) != 0, "Expected file path"

    file_path = args[0]
    
    env = Environment()
    left = IntLiteral(Position(1, 1), 10)
    right = IntLiteral(Position(1, 1), 15)
    binop = BinopExpr(Position(1, 1), left, right, BinopType.ADD)

    lefts = StringLiteral(Position(1, 1), "Hello")
    rights = StringLiteral(Position(1, 1), "World")
    binops = BinopExpr(Position(1, 1), lefts, rights, BinopType.ADD)

    print(binops.typecheck(env).get())
    print(binops.execute())

    with open(file_path) as f:
        result = lexer.tokenize(lexer.Reader(f.read()))

        if result.is_error():
            print(result)
        else:
            for token in result.get():
                print(token)

