from typing import Any, Callable, Dict, Type, cast
from ede_ast.ede_ast import Node
from ede_ast.ede_ir import Instruction
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, UnitLiteral

class IRVisitor:
    '''Constructs the IR representation of an ast node'''

    @staticmethod
    def visit(node: Node) -> Any:
        return VISITORS[type(node)](node)

VISITORS : Dict[Type[Node], Callable[[Node], Any]] = {
    UnitLiteral: lambda _: Instruction.CreatePushI(0),
    IntLiteral: lambda i: Instruction.CreatePushI(cast(IntLiteral, i).value),
    CharLiteral: lambda c: Instruction.CreatePushI(ord(cast(CharLiteral, c).value)),
    BoolLiteral: lambda b: Instruction.CreatePushI(int(cast(BoolLiteral, b).value)),
}