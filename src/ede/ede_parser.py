from enum import Enum, auto
from typing import Dict, List, Optional, cast
from ede_ast.ede_binop import BinopExpr, BinopType
from ede_ast.ede_expr import ExprType, Expression, IdentifierExpr
from ede_ast.ede_stmt import ExprStmt, Statement, VarDeclStmt
from ede_ast.ede_typesystem import EdeBool, EdeChar, EdeInt, EdeString, EdeUnit, TSPrimitiveType
from ede_ast.ede_type_symbol import ArrayTypeSymbol, NameTypeSymbol, PrimitiveTypeSymbol, RecordTypeSymbol, TupleTypeSymbol, TypeSymbol
from ede_token import Token, TokenType
from ede_utils import Error, ErrorType, Position, Result, Success, char
from ede_ast.ede_ast import Node
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral

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

    def peek_read(self, type: TokenType) -> Optional[Token]:
        '''Determines if the next token is of the specified type, and if so, reads it.'''
        return self.read() if self.peek().type == type else None

    def prev(self) -> Token:
        '''Retrieve the prev token in the stream without consuming'''
        return self.stream[max(0, self.ptr - 1)]

    def unread(self) -> None:
        '''Moves stream pointer back by 1'''
        self.ptr = max(0, self.ptr - 1)

    def get_position(self):
        '''Returns the current position of token reader'''
        return self.peek().position

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

# TODO: convert to match expression
# Map between intrinsic type symbols and their ede types
INTRINSIC_TYPE_SYMBOLS_DICT = {
    TSPrimitiveType.UNIT.get_type_symbol(): EdeUnit,
    TSPrimitiveType.INT.get_type_symbol(): EdeInt,
    TSPrimitiveType.STR.get_type_symbol(): EdeString,
    TSPrimitiveType.BOOL.get_type_symbol(): EdeBool,
    TSPrimitiveType.CHAR.get_type_symbol(): EdeChar,
}

# Ensure all primitive type symbols are covered
assert len(INTRINSIC_TYPE_SYMBOLS_DICT) == len(TSPrimitiveType)

def is_intrinsic_type_symbol(name: str) -> bool:
    '''Determines if the given name is an intrinsic type symbol'''
    return name in INTRINSIC_TYPE_SYMBOLS_DICT

def is_operator(type: TokenType) -> bool:
    '''Determines if the given token type is an operator'''
    return type in OPERATOR_DICT

def get_op_prec(op: OperatorType) -> int:
    '''Returns the precedence of the given operator'''
    return OPERATOR_PREC_DICT[op]

def is_op_right_assoc(op: OperatorType) -> bool:
    '''Determines if the operator is right associative'''
    return op in RIGHT_ASSOC_OPERATORS

def parse_type_symbol(reader: TokenReader) -> Result[TypeSymbol]:
    '''Parse an type symbol'''

    position = reader.get_position()

    # TODO: convert to match?
    if reader.peek_read(TokenType.IDENTIFIER) is not None:
        value = reader.prev().value
        return Success(PrimitiveTypeSymbol(INTRINSIC_TYPE_SYMBOLS_DICT[value], position) if is_intrinsic_type_symbol(value) else NameTypeSymbol(value, position))
    elif reader.peek_read(TokenType.SYM_LEFT_SBRACKET) is not None: # Array Type Symbol
        # Get inner type symbol
        inner_type_symbol = parse_type_symbol(reader)
        if inner_type_symbol.is_error():
            return inner_type_symbol

        # Read right bracket
        if reader.peek_read(TokenType.SYM_RIGHT_SBRACKET) is not None:
            return Success(ArrayTypeSymbol(inner_type_symbol.get(), position))

        return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_RIGHT_SBRACKET])
    elif reader.peek_read(TokenType.SYM_LPAREN) is not None: # Tuple Type Symbol
        # Get inner type symbols
        type_symbols : List[TypeSymbol] = []

        while True:
            type_symbol = parse_type_symbol(reader)
            if type_symbol.is_error():
                return type_symbol

            type_symbols.append(type_symbol.get())

            # Try to read a comma
            if reader.peek_read(TokenType.SYM_COMMA) is None:
                break

        # Ensure there are at least two inner type symbols
        if len(type_symbols) == 1:
            return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_COMMA])

        # Read right parenthesis
        if reader.peek_read(TokenType.SYM_RPAREN) is not None:
            return Success(TupleTypeSymbol(type_symbols, position))

        return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_RPAREN])      
    elif reader.peek_read(TokenType.SYM_LEFT_CBRACKET) is not None: # Record Type Symbol
        # Get items
        items : Dict[str, TypeSymbol] = {}

        while True:
            # Read item id
            if reader.peek_read(TokenType.IDENTIFIER) is None:
                return ParseError.UnexpectedToken(reader.peek(), [TokenType.IDENTIFIER])

            name = reader.prev().value

            # Ensure there are no duplicate names
            if name in items:
                return ParseError.DuplicateRecordItemName(name, reader.prev().position)

            # Try to read a colon
            if reader.peek_read(TokenType.SYM_COLON) is None:
                return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_COLON])

            # Try item type symbol
            type_symbol = parse_type_symbol(reader)
            if type_symbol.is_error():
                return type_symbol

            items[name] = type_symbol.get()

            # Try to read a comma
            if reader.peek_read(TokenType.SYM_COMMA) is None:
                break

        # Read right curly bracket
        if reader.peek_read(TokenType.SYM_RIGHT_CBRACKET) is not None:
            return Success(RecordTypeSymbol(items, position))

        return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_RIGHT_CBRACKET])

    return ParseError.UnexpectedToken(reader.peek(), [
        TokenType.IDENTIFIER,
        TokenType.SYM_LEFT_SBRACKET,
        TokenType.SYM_LEFT_CBRACKET,
        TokenType.SYM_LPAREN]) 

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
        TokenType.KW_FALSE]) 

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

def parse_declaration(reader: TokenReader) -> Result[Statement]:
    '''Parse a declaration'''

    position = reader.get_position()

    if reader.peek_read(TokenType.KW_LET) is None:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.KW_LET])
    
    if reader.peek_read(TokenType.IDENTIFIER) is None:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.IDENTIFIER])

    id = reader.prev().value
    type_symbol: Optional[TypeSymbol] = None
    expr : Optional[Expression] = None

    if reader.peek_read(TokenType.SYM_COLON) is not None:
        ts_res = parse_type_symbol(reader)
        if ts_res.is_error():
            return cast(Error, ts_res)

        type_symbol = ts_res.get()
    elif reader.peek().type != TokenType.SYM_EQUALS:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_EQUALS])
    
    if reader.peek_read(TokenType.SYM_EQUALS) is not None:
        expr_res = parse_expr(reader)
        if expr_res.is_error():
            return cast(Error, expr_res)

        expr = expr_res.get()

    return Success(VarDeclStmt(position, id, type_symbol, expr))     

def parse_stmt(reader: TokenReader) -> Result[Statement]:
    '''Parse a statement'''

    if reader.peek().type == TokenType.KW_LET:
        return parse_declaration(reader)
        
    node = parse_expr(reader)
    if node.is_success():
        return Success(ExprStmt(node.get()))

    return cast(Error, node)

def parse(reader: TokenReader) -> Result[Node]:
    'Parses a stream of tokens and returns the AST'

    node = parse_stmt(reader)
    return Success(cast(Node, node.get())) if node.is_success() else cast(Error, node)

class ParseError:
    '''Wrapper for parsing errors'''

    @staticmethod
    def DuplicateRecordItemName(name: str, pos: Position) -> Error:
        return Error(ErrorType.PARSING_DUP_RECORD_ITEM_NAME, pos, f"Duplicate record item name '{name}' found")

    @staticmethod
    def UnexpectedToken(found: Token, expected: List[TokenType]) -> Error:
        return Error(ErrorType.PARSING_UNEXPECTED_TOKEN, found.position, f"Encountered unexpected token '{found.type.name}' but expected {[item.name for item in expected]}")

    @staticmethod
    def InvalidOperator(op: OperatorType, pos: Position) -> Error:
        return Error(ErrorType.PARSING_INVALID_OPERATOR, pos, f"Invalid operator {op}")

