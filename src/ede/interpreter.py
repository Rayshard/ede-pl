from enum import Enum, auto
from typing import Dict, List, NamedTuple, Optional, cast, get_args
from ede_ast.ede_context import Context, CtxEntryType
from ede_ast.ede_ir import InstructionType, OperandTypes
from ede_ast.ede_literal import UnitLiteral
from ede_ast.ede_stmt import ExprStmt, Statement
from ede_ast.ede_typesystem import EdeArray, EdeFunc, EdeObject, EdePrimitive, EdeTuple, EdeType, TCCtxEntry, TSPrimitiveType
from ede_utils import Position, char, float_from_bytes, float_to_bytes, int_from_bytes, int_to_bytes, unit

class ExecExceptionType(Enum):
    '''Enumeration of execution exception types'''

    DIV_BY_ZERO = auto()

class ExecException(NamedTuple):
    '''Execution Exception'''

    type: ExecExceptionType
    name: str
    pos: Position
    msg: str

    @staticmethod
    def DivisionByZero(pos: Position) -> 'ExecException':
        return ExecException(ExecExceptionType.DIV_BY_ZERO, 'Division By Zero', pos, "Unable to divide by 0")

class ArrayValue(NamedTuple):
    '''Array value'''

    values: List['ExecValue']

    def __str__(self) -> str:
        return f"[{', '.join([str(value) for value in self.values])}]"

class TupleValue(NamedTuple):
    '''Tuple value'''

    values: List['ExecValue']

    def __str__(self) -> str:
        return f"({', '.join([str(value) for value in self.values])})"

class ObjectValue(NamedTuple):
    '''Object value'''

    name: str
    members: Dict[str, 'ExecValue']

    def __str__(self) -> str:
        return self.name + ' { ' + f"{', '.join([name + ' = ' + str(value) for name, value in self.members.items()])}" + ' }'

FuncPtrValue = Statement
ExecValueTypes = int | str | bool | char | unit | ArrayValue | TupleValue | ObjectValue | FuncPtrValue | ExecException

class ExecValue:
    '''Representation of the possible values return on execution of an AST node'''

    def __init__(self, value: ExecValueTypes) -> None:
        '''Create a typed execution value'''

        self.value = value

    def __eq__(self, o: object) -> bool:
        if isinstance(o, ExecValue):
            return type(o.value) == type(self.value) and o.value == self.value
        elif isinstance(o, get_args(ExecValueTypes)):
            return type(o) == type(self.value) and o == self.value
        
        return False 

    def __str__(self) -> str:
        if isinstance(self.value, char):
            return "'" + str(self.value) + "'"
        elif isinstance(self.value, str):
            return '"' + str(self.value) + '"'
        
        return str(self.value)

    def is_exception(self) -> bool:
        return isinstance(self.value, ExecException)

    def to_int(self) -> int:
        assert type(self.value) == int
        return cast(int, self.value)

    def to_str(self) -> str:
        assert type(self.value) == str
        return cast(str, self.value)

    def to_bool(self) -> bool:
        assert type(self.value) == bool
        return cast(bool, self.value)

    def to_char(self) -> char:
        assert type(self.value) == char
        return cast(char, self.value)

    def to_unit(self) -> unit:
        assert type(self.value) == unit
        return cast(unit, self.value)

    def to_array(self) -> ArrayValue:
        assert type(self.value) == ArrayValue
        return cast(ArrayValue, self.value)

    def to_tuple(self) -> TupleValue:
        assert type(self.value) == TupleValue
        return cast(TupleValue, self.value)

    def to_object(self) -> ObjectValue:
        assert type(self.value) == ObjectValue
        return cast(ObjectValue, self.value)

    def to_func_ptr(self) -> FuncPtrValue:
        assert isinstance(self.value, FuncPtrValue)
        return self.value

    def to_exception(self) -> ExecException:
        assert type(self.value) == ExecException
        return cast(ExecException, self.value)
 
    @staticmethod
    def UNIT() -> 'ExecValue':
        return ExecValue(unit())

    @staticmethod
    def get_default_value(type: EdeType) -> 'ExecValue':
        match type:
            case EdePrimitive(prim_type=TSPrimitiveType.UNIT):
                return ExecValue(unit())
            case EdePrimitive(prim_type=TSPrimitiveType.INT):
                return ExecValue(0)
            case EdePrimitive(prim_type=TSPrimitiveType.STR):
                return ExecValue("")
            case EdePrimitive(prim_type=TSPrimitiveType.CHAR):
                return ExecValue('\0')
            case EdePrimitive(prim_type=TSPrimitiveType.BOOL):
                return ExecValue(False)
            case EdeTuple():
                return ExecValue(TupleValue(list(map(ExecValue.get_default_value, type.get_inner_types()))))
            case EdeArray():
                return ExecValue(ArrayValue([]))
            case EdeObject():
                return ExecValue(ObjectValue(type.get_name(), {id: ExecValue.get_default_value(mem_type) for id, mem_type in type.get_members().items()}))
            case EdeFunc():
                return ExecValue(ExprStmt(UnitLiteral(Position())))
            case _:
                raise Exception('Case not handled')

class ExecEntry(TCCtxEntry):
    '''Execution context entry'''
    
    def __init__(self, type: CtxEntryType, ede_type: EdeType, value: ExecValue, pos: Position) -> None:
        '''Create entry'''

        super().__init__(type, ede_type, pos)
        self.value = value

class ExecContext(Context[ExecEntry]):
    def __init__(self, parent: Optional['Context[ExecEntry]'] = None) -> None:
        super().__init__(parent=parent)

        self.ret_stack : List[ExecValue] = [] if parent is None else parent.ret_stack # type: ignore

Instruction = Dict[str, InstructionType | Dict[str, OperandTypes]]
Code = List[str | Instruction]

class Interpreter:
    def __init__(self) -> None:
        self.stack : List[bytes] = []
        self.ip : int = 0

    def run(self, code: Code) -> int | ExecException:
        # Get labels
        labels = {elem: i for i, elem in enumerate(code) if isinstance(elem, str)}

        self.ip = labels['__start']

        # Execute instructions
        while 0 <= self.ip < len(code):
            elem = code[self.ip]

            if not isinstance(elem, str):
                print(elem)

                exec_result = self.execute_instr(elem, labels)
                if exec_result is not None:
                    return exec_result

                self.print_stack()

            self.ip += 1    

        return 0

    def execute_instr(self, instr: Instruction, labels: Dict[str, int]) -> None | int | ExecException:
        match instr:
            case {"type": InstructionType.PUSHI.name, "operands": {"constant": c}}: 
                self.stack.append(int_to_bytes(cast(int, c)))
            case {"type": InstructionType.PUSHD.name, "operands": {"constant": c}}: 
                self.stack.append(float_to_bytes(cast(float, c)))
            case {"type": InstructionType.POP.name, "operands": None}: 
                self.stack.pop()
            case {"type": InstructionType.ADDI.name, "operands": None}: 
                self.stack.append(int_to_bytes(int_from_bytes(self.stack.pop()) + int_from_bytes(self.stack.pop())))
            case {"type": InstructionType.ADDD.name, "operands": None}: 
                self.stack.append(float_to_bytes(float_from_bytes(self.stack.pop()) + float_from_bytes(self.stack.pop())))
            case {"type": InstructionType.JUMP.name, "operands": {"label": l}}: 
                self.ip = labels[cast(str, l)] - 1
            case _: raise Exception(f"Unknown Instruction: {instr}")

    def print_stack(self):
        print('=' * 90)
        
        for i, word in enumerate(self.stack):
            print("0x{:016x}".format(i * 8) + f":\t0x{word.hex()}, int({int_from_bytes(word)}), double({float_from_bytes(word)})")
        
        print('=' * 90)
        