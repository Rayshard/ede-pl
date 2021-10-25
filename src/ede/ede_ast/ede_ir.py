from enum import Enum, auto
from typing import List

double = float

class SysCallCode(Enum):
    EXIT = 0
    PRINTC = auto()
    PRINTS = auto()
    MALLOC = auto()
    FREE = auto()

Operand = int | double | SysCallCode | str

class OpCode(Enum):
    NOOP = 0
    PUSHI = auto()
    PUSHD = auto()
    POP = auto()
    IADD = auto()
    ISUB = auto()
    IMUL = auto()
    IDIV = auto()
    DADD = auto()
    DSUB = auto()
    DMUL = auto()
    DDIV = auto()
    JUMP = auto()
    JUMPZ = auto()
    JUMPNZ = auto()
    SYSCALL = auto()
    EXIT = auto()

class Instruction:
    def __init__(self, op_code: OpCode, operands: List[Operand]) -> None:
        self.__op_code = op_code
        self.__operands = operands

    def get_elements(self) -> List[str | Operand]:
        return [self.__op_code.name] + self.__operands # type: ignore

    def __str__(self) -> str:
        return ' '.join([str(elem) for elem in self.get_elements()])
         
    def get_op_code(self) -> OpCode:
        return self.__op_code

    def get_operands(self) -> List[Operand]:
        return self.__operands

    @staticmethod
    def NOOP(): return Instruction(OpCode.NOOP, [])

    @staticmethod
    def PUSHI(constant: int): return Instruction(OpCode.PUSHI, [constant])

    @staticmethod
    def PUSHD(constant: double): return Instruction(OpCode.PUSHD, [constant])

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
    def JUMP(label: str): return Instruction(OpCode.JUMP, [label])

    @staticmethod
    def JUMPZ(label: str): return Instruction(OpCode.JUMPZ, [label])

    @staticmethod
    def JUMPNZ(label: str): return Instruction(OpCode.JUMPNZ, [label])

    @staticmethod
    def SYSCALL(code: SysCallCode): return Instruction(OpCode.SYSCALL, [code])
