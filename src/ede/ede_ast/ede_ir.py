from enum import Enum, auto
from typing import List, Type

class InstructionType(Enum):
    PUSHI = auto()
    PUSHD = auto()
    ADDI = auto()
    SUBI = auto()
    MULI = auto()
    DIVI = auto()
    ADDD = auto()
    POP = auto()
    JUMP = auto()

Constant = int | float
OperandTypes = Constant | str

class Instruction:

    def __init__(self, instr_type: InstructionType, operands: List[OperandTypes]) -> None:
        assert [type(op) for op in operands] == Instruction.get_operand_types(instr_type), f"Invalid Operands for Instruction '{instr_type.name}'"
        
        self.instr_type = instr_type
        self.operands = operands

    def get_ir(self) -> List[str | OperandTypes]:
        return [self.instr_type.name] + self.operands # type: ignore

    def __str__(self) -> str:
        return ' '.join([str(elem) for elem in self.get_ir()])

    @staticmethod
    def get_operand_types(instr: InstructionType) -> List[Type[OperandTypes]]:
        match instr:
            case InstructionType.PUSHI: return [int]
            case InstructionType.PUSHD: return [float]
            case InstructionType.ADDI: return []
            case InstructionType.ADDD: return []
            case InstructionType.POP: return []
            case InstructionType.JUMP: return [str]
            case _: raise Exception("Case not handled")

    @staticmethod
    def PUSHI(constant: int): return Instruction(InstructionType.PUSHI, [constant])

    @staticmethod
    def PUSHD(constant: float): return Instruction(InstructionType.PUSHD, [constant])

    @staticmethod
    def POP(): return Instruction(InstructionType.POP, [])

    @staticmethod
    def ADDI(): return Instruction(InstructionType.ADDI, [])

    @staticmethod
    def SUBI(): return Instruction(InstructionType.SUBI, [])

    @staticmethod
    def MULI(): return Instruction(InstructionType.MULI, [])

    @staticmethod
    def DIVI(): return Instruction(InstructionType.DIVI, [])

    @staticmethod
    def ADDD(): return Instruction(InstructionType.ADDD, [])

    @staticmethod
    def JUMP(label: str): return Instruction(InstructionType.JUMP, [label])
