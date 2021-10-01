from enum import Enum, auto
from typing import Dict, List, Union, cast
from ede_ast.ede_binop import BinopExpr, BinopType
from ede_ast.ede_expr import ExprType, Expression, IdentifierExpr
from ede_token import Token, TokenType
from ede_utils import Error, ErrorType, Position, Result, Success
from ede_ast.ede_ast import Node
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral, char

class TokenReader:
    '''Token Reader class that reads a stream of tokens'''

    def __init__(self, stream: List[Token]) -> None:
        '''Creates a token reader'''

        assert len(stream) != 0 and stream[-1].type == TokenType.EOF
        self.stream = stream
        self.ptr = 0

    def peek(self) -> Token:
        '''Retrieve the next token in the stream without consuming'''
        return self.stream[self.ptr]

    def read(self) -> Token:
        '''Retrieve and consume the next token in the stream'''

        tok = self.peek()
        
        if tok.type != TokenType.EOF:
            self.ptr = self.ptr + 1

        return tok

    def unread(self) -> None:
        '''Moves stream pointer back by 1'''
        self.ptr = min(0, self.ptr - 1)

    def get_position(self):
        '''Returns the current position of token reader'''
        return self.peek().position

def parse_atom(reader: TokenReader) -> Result[Expression]:
    'Parse an atom node'

    tok = reader.read()

    # TODO: Convert to match expression
    if tok.type == TokenType.INTEGER:
        return Success(IntLiteral(tok.position, cast(int, tok.value)))
    elif tok.type == TokenType.STRING:
        return Success(StringLiteral(tok.position, cast(str, tok.value)))
    elif tok.type == TokenType.CHAR:
        return Success(CharLiteral(tok.position, cast(char, tok.value)))
    elif tok.type == TokenType.KW_TRUE:
        return Success(BoolLiteral(tok.position, True))
    elif tok.type == TokenType.KW_FALSE:
        return Success(BoolLiteral(tok.position, False))
    elif tok.type == TokenType.IDENTIFIER:
        return Success(IdentifierExpr(tok.position, cast(str, tok.value)))

    reader.unread()
    return ParseError.UnexpectedToken(tok, [
        TokenType.INTEGER,
        TokenType.STRING,
        TokenType.CHAR,
        TokenType.IDENTIFIER,
        TokenType.KW_TRUE,
        TokenType.KW_FALSE],
        tok.position)        

class OperatorType(Enum):
    '''Enumeration of operators'''

    ADD = auto()
    SUB = auto()
    MUL = auto()
    DIV = auto()
    ASSIGN = auto()

# Map of token types to operator types 
OPERATOR_DICT : Dict[TokenType, OperatorType] = {
    TokenType.SYM_PLUS: OperatorType.ADD,
    TokenType.SYM_DASH: OperatorType.SUB,
    TokenType.SYM_ASTERISK: OperatorType.MUL,
    TokenType.SYM_FWD_SLASH: OperatorType.DIV,
    TokenType.SYM_EQUALS: OperatorType.ASSIGN
}

# Map of operator types to their precedences 
OPERATOR_PREC_DICT : Dict[OperatorType, int] = {
    item[0]: item[1] for sublist in [dict.fromkeys(ops, prec) for prec, ops in enumerate([
      [OperatorType.ADD, OperatorType.SUB],
      [OperatorType.MUL, OperatorType.DIV],
      [OperatorType.ASSIGN]
  ])] for item in sublist.items()
}

# List of right associative operators
RIGHT_ASSOC_OPERATORS : List[OperatorType] = [OperatorType.ASSIGN]

# Ensures each operator has a precedence
assert len(OperatorType) == len(OPERATOR_PREC_DICT)

def is_operator(type: TokenType) -> bool:
    '''Determines if the given token type is an operator'''
    return type in OPERATOR_DICT

def get_op_prec(op: OperatorType) -> int:
    '''Returns the precedence of the given operator'''
    return OPERATOR_PREC_DICT[op]

def is_op_right_assoc(op: OperatorType) -> bool:
    '''Determines if the operator is right associative'''
    return op in RIGHT_ASSOC_OPERATORS

def parse_expr(reader: TokenReader, cur_precedence: int = 0) -> Result[Expression]:
    '''Parse an expression'''

    atom_res = parse_atom(reader)
    if atom_res.is_error():
        return atom_res

    expr = atom_res.get()

    while is_operator(reader.peek().type):
        op_tok = reader.read()
        op = OPERATOR_DICT[op_tok.type]
        op_prec = OPERATOR_PREC_DICT[op]

        if op_prec < cur_precedence:
            break

        rhs = parse_expr(reader, op_prec if is_op_right_assoc(op) else (op_prec + 1))  
        if rhs.is_error():
            return rhs

        # TODO: use match instead
        if op == OperatorType.ADD:
            expr = BinopExpr(op_tok.position, expr, rhs.get(), BinopType.ADD)
        elif op == OperatorType.SUB:
            expr = BinopExpr(op_tok.position, expr, rhs.get(), BinopType.SUB)
        elif op == OperatorType.MUL:
            expr = BinopExpr(op_tok.position, expr, rhs.get(), BinopType.MUL)
        elif op == OperatorType.DIV:
            expr = BinopExpr(op_tok.position, expr, rhs.get(), BinopType.DIV)
        elif op == OperatorType.ASSIGN:
            # Ensrue that assignments have ids as their LHS
            if expr.get_expr_type() != ExprType.ID:
                return ParseError.InvalidOperator(op, op_tok.position)

            expr = BinopExpr(op_tok.position, expr, rhs.get(), BinopType.ASSIGN)

    return Success(expr)

def parse(reader: TokenReader) -> Result[Node]:
    'Parses a stream of tokens and returns the AST'
    node = parse_expr(reader)
    return Success(cast(Node, node.get())) if node.is_success() else cast(Error, node)

class ParseError:
    '''Wrapper for parsing errors'''

    @staticmethod
    def UnexpectedToken(found: Token, expected: Union[TokenType, List[TokenType]], pos: Position) -> Error:
        return Error(ErrorType.PARSING_UNEXPECTED_TOKEN, pos, f"Encountered unexpected token {found} but expected {expected}")

    @staticmethod
    def InvalidOperator(op: OperatorType, pos: Position) -> Error:
        return Error(ErrorType.PARSING_INVALID_OPERATOR, pos, f"Invalid operator {op}")

