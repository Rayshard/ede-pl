from enum import Enum, auto
from typing import Dict, Optional

class InstructionType(Enum):
    PUSHI = auto()
    PUSHD = auto()
    ADDI = auto()
    ADDD = auto()
    POP = auto()
    JUMP = auto()

Constant = int | float
OperandTypes = Constant | str

class Instruction:
    @staticmethod
    def __Create(type: InstructionType, operands: Optional[Dict[str, OperandTypes]]):
        return {"type": type, "operands": operands}

    @staticmethod
    def CreatePushI(constant: int): return Instruction.__Create(InstructionType.PUSHI, {"constant": constant})

    @staticmethod
    def CreatePushF(constant: float): return Instruction.__Create(InstructionType.PUSHD, {"constant": constant})

    @staticmethod
    def CreatePop(): return Instruction.__Create(InstructionType.POP, None)

    @staticmethod
    def CreateAddI(): return Instruction.__Create(InstructionType.ADDI, None)

    @staticmethod
    def CreateAddD(): return Instruction.__Create(InstructionType.ADDD, None)

    @staticmethod
    def CreateJump(label: str): return Instruction.__Create(InstructionType.PUSHI, {"label": label})
