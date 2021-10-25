from typing import Any, Callable, Dict, List, Type, cast
from ..ede_ast import Node
from ..ede_definition import DefType
from ..ede_expr import BinopExpr, BinopType
from ..ede_ir import Instruction, double
from ..ede_literal import BoolLiteral, CharLiteral, IntLiteral, UnitLiteral
from ..ede_module import Module
from ..ede_stmt import ExprStmt
from ..ede_typesystem import EdePrimitive, TSPrimitiveType

ModuleConstant = str | int | double

class ModuleIRBuilder:
    def __init__(self) -> None:
        self.__constants: Dict[str, ModuleConstant] = {}
        self.__code: List[str | Instruction] = []

    def add_constant(self, id: str, const: ModuleConstant) -> None:
        assert id not in self.__constants, "Duplicate constant ID encountered"
        self.__constants[id] = const

    def add_code(self, elem: str | Instruction) -> None:
        assert not isinstance(elem, str) or elem not in self.__code, "Duplicate label encountered"
        self.__code.append(elem)

    def get_ir(self):
        return {
            "constants": self.__constants,
            "code": [elem if isinstance(elem, str) else elem.get_elements() for elem in self.__code]
        }

class IRVisitor:
    '''Constructs the IR representation of an ast node'''

    @staticmethod
    def visit(node: Node, builder: ModuleIRBuilder) -> None:
        return VISITORS[type(node)](node, builder)

def visit_Module(m: Module, builder: ModuleIRBuilder) -> None:
    builder.add_code("__start")
    
    for definition in m.defs:
        if definition.get_def_type() != DefType.FUNC:
            continue

        IRVisitor.visit(definition, builder)

    for stmt in m.stmts:
        IRVisitor.visit(stmt, builder)

    builder.add_code(Instruction.PUSHI(0))
    builder.add_code(Instruction.EXIT())

def visit_BinopExpr(b: BinopExpr, builder: ModuleIRBuilder) -> None:
    if b.op == BinopType.ASSIGN:
        pass
    else:
        IRVisitor.visit(b.right, builder)
        IRVisitor.visit(b.left, builder)
        
        match b.type_pattern:
            case (EdePrimitive(prim_type=TSPrimitiveType.INT), EdePrimitive(prim_type=TSPrimitiveType.INT), BinopType.ADD):
                builder.add_code(Instruction.IADD())
            case (EdePrimitive(prim_type=TSPrimitiveType.INT), EdePrimitive(prim_type=TSPrimitiveType.INT), BinopType.SUB):
                builder.add_code(Instruction.ISUB())
            case (EdePrimitive(prim_type=TSPrimitiveType.INT), EdePrimitive(prim_type=TSPrimitiveType.INT), BinopType.MUL):
                builder.add_code(Instruction.IMUL())
            case (EdePrimitive(prim_type=TSPrimitiveType.INT), EdePrimitive(prim_type=TSPrimitiveType.INT), BinopType.DIV):
                builder.add_code(Instruction.IDIV())
            case _:
                raise Exception("Case not handled")

def visit_ExprStmt(e: ExprStmt, builder: ModuleIRBuilder) -> None:
    IRVisitor.visit(e.expr, builder)
    builder.add_code(Instruction.POP())

VISITORS : Dict[Type[Node], Callable[[Any, ModuleIRBuilder], None]] = {
    UnitLiteral: lambda _, builder: builder.add_code(Instruction.PUSHI(0)),
    IntLiteral: lambda i, builder: builder.add_code(Instruction.PUSHI(cast(IntLiteral, i).value)),
    CharLiteral: lambda c, builder: builder.add_code(Instruction.PUSHI(ord(cast(CharLiteral, c).value))),
    BoolLiteral: lambda b, builder: builder.add_code(Instruction.PUSHI(int(cast(BoolLiteral, b).value))),
    Module: visit_Module,
    ExprStmt: visit_ExprStmt,
    BinopExpr: visit_BinopExpr
}