from enum import IntEnum, auto
import struct
from typing import List, Type

OP_CODE_SIZE = 1
WORD_SIZE = 8

double = float

class Word:
    def __init__(self, value: int | double = 0) -> None:
        self.__bytes = bytes(WORD_SIZE)

        match value:
            case int(): self.set_int(value)
            case double(): self.set_double(value)
            case _: raise Exception("Case not handled!")

    def set_int(self, value: int) -> None:
        self.__bytes = int_to_bytes(value)

    def set_double(self, value: double) -> None:
        self.__bytes = double_to_bytes(value)

    def as_bytes(self) -> bytes:
        return self.__bytes

    def as_int(self) -> int:
        return int_from_bytes(self.__bytes)

    def as_double(self) -> double:
        return double_from_bytes(self.__bytes)

Operand = Word | int | double | str

def int_to_bytes(value: int) -> bytes:
    return value.to_bytes(WORD_SIZE, 'little', signed=True)

def double_to_bytes(value: double) -> bytes:
    return struct.pack("d", value)

def int_from_bytes(bytes: bytes) -> int:
    return int.from_bytes(bytes, 'little', signed=True)

def double_from_bytes(bytes: bytes) -> double:
    return struct.unpack('d', bytes)[0]

def get_operand_size(operand: Operand) -> int:
    match operand:
        case Word(): return WORD_SIZE
        case int(): return WORD_SIZE
        case double(): return WORD_SIZE
        case str(): raise Exception("Cannot get the size of a string")
        case _: raise Exception("Case not handled")

def get_operand_bytes(operand: Operand) -> bytes:
    match operand:
        case Word(): return operand.as_bytes()
        case int(): return int_to_bytes(operand)
        case double(): return double_to_bytes(operand)
        case str(): raise Exception("Cannot get the bytes of a string")
        case _: raise Exception("Case not handled")

class OpCode(IntEnum):
    PUSH = 0
    IADD = auto()
    ISUB = auto()
    IMUL = auto()
    IDIV = auto()
    DADD = auto()
    DSUB = auto()
    DMUL = auto()
    DDIV = auto()
    POP = auto()
    JUMP = auto()
    EXIT = auto()

    # Placeholder op codes
    PUSH_I = auto()
    PUSH_D = auto()
    JUMP_ = auto()

    def get_bytes(self) -> bytes:
        if self.is_placeholder():
            raise Exception("Could not get the bytes for a placeholder op code!")

        return self.value.to_bytes(OP_CODE_SIZE, 'big', signed=False)

    def is_placeholder(self) -> bool:
        return self.name.endswith('_')

    def get_operand_types(self) -> List[Type[Operand]]:
        match self:
            case OpCode.PUSH: return [Word]
            case OpCode.PUSH_I: return [int]
            case OpCode.PUSH_D: return [double]
            case OpCode.IADD: return []
            case OpCode.ISUB: return []
            case OpCode.IMUL: return []
            case OpCode.IDIV: return []
            case OpCode.DADD: return []
            case OpCode.DSUB: return []
            case OpCode.DMUL: return []
            case OpCode.DDIV: return []
            case OpCode.POP: return []
            case OpCode.EXIT: return []
            case OpCode.JUMP: return [int]
            case OpCode.JUMP_: return [str]
            case _: raise Exception(f"Case not handled '{self.name}'")

    def get_instr_byte_size(self) -> int:
        if self.is_placeholder():
            return OpCode[self.name.split('_')[0]].get_instr_byte_size()

        return OP_CODE_SIZE + sum([get_operand_size(operand()) for operand in self.get_operand_types()])

class Instruction:

    def __init__(self, op_code: OpCode, operands: List[Operand]) -> None:
        assert [type(op) for op in operands] == op_code.get_operand_types(), f"Invalid operands for op code '{op_code.name}'"
        
        self.__op_code = op_code
        self.__operands = operands

    def get_ir(self) -> List[str | Operand]:
        return [self.__op_code.name] + self.__operands # type: ignore

    def __str__(self) -> str:
        return ' '.join([str(elem) for elem in self.get_ir()])

    def is_placeholder(self) -> bool:
        return self.__op_code.is_placeholder()

    def get_byte_size(self) -> int:
        return self.__op_code.get_instr_byte_size()

    def get_bytes(self) -> bytes:
        return self.__op_code.get_bytes() + b''.join([get_operand_bytes(operand) for operand in self.__operands])
         
    def get_op_code(self) -> OpCode:
        return self.__op_code

    def get_operands(self) -> List[Operand]:
        return self.__operands

    @staticmethod
    def PUSHI(constant: int): return Instruction(OpCode.PUSH_I, [constant])

    @staticmethod
    def PUSHD(constant: double): return Instruction(OpCode.PUSH_D, [constant])

    @staticmethod
    def POP(): return Instruction(OpCode.POP, [])

    @staticmethod
    def IADD(): return Instruction(OpCode.IADD, [])

    @staticmethod
    def ISUB(): return Instruction(OpCode.ISUB, [])

    @staticmethod
    def IMUL(): return Instruction(OpCode.IMUL, [])

    @staticmethod
    def IDIV(): return Instruction(OpCode.IDIV, [])

    @staticmethod
    def DADD(): return Instruction(OpCode.DADD, [])

    @staticmethod
    def DSUB(): return Instruction(OpCode.DSUB, [])

    @staticmethod
    def DMUL(): return Instruction(OpCode.DMUL, [])

    @staticmethod
    def DDIV(): return Instruction(OpCode.DDIV, [])

    @staticmethod
    def EXIT(): return Instruction(OpCode.EXIT, [])

    @staticmethod
    def JUMP(label: str): return Instruction(OpCode.JUMP_, [label])
