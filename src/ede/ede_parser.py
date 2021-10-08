from enum import Enum, auto
from typing import Dict, List, Optional, cast
from ede_ast.ede_definition import Definition, ObjDef
from ede_ast.ede_expr import ArrayExpr, ExprType, Expression, IdentifierExpr, ObjInitExpr, TupleExpr, BinopExpr, BinopType
from ede_ast.ede_module import Module
from ede_ast.ede_stmt import Block, ExprStmt, IfElseStmt, Statement, VarDeclStmt
from ede_ast.ede_typesystem import EdeBool, EdeChar, EdeInt, EdeString, EdeUnit
from ede_ast.ede_type_symbol import ArrayTypeSymbol, NameTypeSymbol, PrimitiveTypeSymbol, TupleTypeSymbol, TypeSymbol
from ede_token import Token, TokenType
from ede_utils import Error, ErrorType, Position, Positioned, Result, Success, char
from ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral, UnitLiteral

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

    def is_eof(self) -> bool:
        return self.peek().type == TokenType.EOF

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
    "unit": EdeUnit(),
    "int": EdeInt(),
    "string": EdeString(),
    "bool": EdeBool(),
    "char": EdeChar(),
}

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

    return ParseError.UnexpectedToken(reader.peek(), [
        TokenType.IDENTIFIER,
        TokenType.SYM_LEFT_SBRACKET,
        TokenType.SYM_LPAREN]) 

def parse_obj_init_expr(reader: TokenReader) -> Result[Expression]:
    '''Parse object initializer expression'''

    position = reader.get_position()
    
    if reader.peek_read(TokenType.IDENTIFIER) is None:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.IDENTIFIER])

    obj_name = cast(str, reader.prev().value)

    if reader.peek_read(TokenType.SYM_LEFT_CBRACKET) is None:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_LEFT_CBRACKET])

    # Get members
    members : Dict[Positioned[str], Expression] = {}

    while True:
        # Read member id
        if reader.peek_read(TokenType.IDENTIFIER) is None:
            return ParseError.UnexpectedToken(reader.peek(), [TokenType.IDENTIFIER])

        mem_name = Positioned[str](reader.prev().value, reader.prev().position)

        # Ensure there are no duplicate names
        if mem_name.value in members:
            return ParseError.DuplicateMemberName(mem_name.value, reader.prev().position)

        # Try to read an equals
        if reader.peek_read(TokenType.SYM_EQUALS) is None:
            return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_EQUALS])

        # Try parse member expression
        expr = parse_expr(reader)
        if expr.is_error():
            return expr

        members[mem_name] = expr.get()

        # Try to read a comma
        if reader.peek_read(TokenType.SYM_COMMA) is None:
            break

    # Read right curly bracket
    if reader.peek_read(TokenType.SYM_RIGHT_CBRACKET) is None:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_RIGHT_CBRACKET])

    return Success(ObjInitExpr(obj_name, members, position))

def parse_atom(reader: TokenReader) -> Result[Expression]:
    '''Parse an atom node'''

    position = reader.get_position()

    # TODO: Convert to match expression
    if reader.peek_read(TokenType.INTEGER) is not None:
        return Success(IntLiteral(position, cast(int, reader.prev().value)))
    elif reader.peek_read(TokenType.STRING) is not None:
        return Success(StringLiteral(position, cast(str, reader.prev().value)))
    elif reader.peek_read(TokenType.CHAR) is not None:
        return Success(CharLiteral(position, cast(char, reader.prev().value)))
    elif reader.peek_read(TokenType.KW_TRUE) is not None:
        return Success(BoolLiteral(position, True))
    elif reader.peek_read(TokenType.KW_FALSE) is not None:
        return Success(BoolLiteral(position, False))
    elif reader.peek_read(TokenType.IDENTIFIER) is not None:
        if reader.peek().type == TokenType.SYM_LEFT_CBRACKET: # Object Initializer Expression
            reader.unread()
            return parse_obj_init_expr(reader)

        return Success(IdentifierExpr(position, cast(str, reader.prev().value)))
    elif reader.peek_read(TokenType.SYM_LEFT_SBRACKET) is not None: # Array
        # Get inner exprs
        exprs : List[Expression] = []

        while True:
            expr = parse_expr(reader)
            if expr.is_error():
                return expr

            exprs.append(expr.get())

            # Try to read a comma
            if reader.peek_read(TokenType.SYM_COMMA) is None:
                break

        # Read right bracket
        if reader.peek_read(TokenType.SYM_RIGHT_SBRACKET) is not None:
            return Success(ArrayExpr(exprs, position))

        return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_RIGHT_SBRACKET])
    elif reader.peek_read(TokenType.SYM_LPAREN) is not None:
        # Unit Literal
        if reader.peek_read(TokenType.SYM_RPAREN) is not None:
            return Success(UnitLiteral(position))

        initial_expr = parse_expr(reader)
        if initial_expr.is_error():
            return initial_expr

        # Parenthesized expression
        if reader.peek_read(TokenType.SYM_RPAREN) is not None:
            return initial_expr
        elif reader.peek_read(TokenType.SYM_COMMA) is None:
            return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_COMMA])

        # Tuple
        exprs : List[Expression] = [initial_expr.get()]

        while True:
            expr = parse_expr(reader)
            if expr.is_error():
                return expr

            exprs.append(expr.get())

            # Try to read a comma
            if reader.peek_read(TokenType.SYM_COMMA) is None:
                break

        # Read right parenthesis
        if reader.peek_read(TokenType.SYM_RPAREN) is not None:
            return Success(TupleExpr(exprs, position))

        return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_RPAREN])      
    
    return ParseError.UnexpectedToken(reader.peek(), [
        TokenType.INTEGER,
        TokenType.STRING,
        TokenType.CHAR,
        TokenType.IDENTIFIER,
        TokenType.KW_TRUE,
        TokenType.KW_FALSE,
        TokenType.SYM_LEFT_SBRACKET,
        TokenType.SYM_LPAREN]) 

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
            return ts_res.error()

        type_symbol = ts_res.get()
    elif reader.peek().type != TokenType.SYM_EQUALS:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_EQUALS])
    
    if reader.peek_read(TokenType.SYM_EQUALS) is not None:
        expr_res = parse_expr(reader)
        if expr_res.is_error():
            return expr_res.error()

        expr = expr_res.get()

    return Success(VarDeclStmt(position, id, type_symbol, expr))     

def parse_definition(reader: TokenReader) -> Result[Definition]:
    '''Parse a definition'''

    position = reader.get_position()

    if reader.peek_read(TokenType.KW_DEF) is None:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.KW_DEF])
    
    if reader.peek_read(TokenType.IDENTIFIER) is None:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.IDENTIFIER])

    def_id = reader.prev().value

    if reader.peek_read(TokenType.SYM_EQUALS) is None:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_EQUALS])

    # TODO: pattern match
    if reader.peek_read(TokenType.KW_OBJECT) is not None:
        if reader.peek_read(TokenType.SYM_LEFT_CBRACKET) is None:
            return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_LEFT_CBRACKET])

        # Get members
        members : Dict[Positioned[str], TypeSymbol] = {}

        while True:
            # Read member id
            if reader.peek_read(TokenType.IDENTIFIER) is None:
                return ParseError.UnexpectedToken(reader.peek(), [TokenType.IDENTIFIER])

            mem_name = Positioned[str](reader.prev().value, reader.prev().position)

            # Ensure there are no duplicate names
            if mem_name.value in members:
                return ParseError.DuplicateMemberName(mem_name.value, reader.prev().position)

            # Try to read an equals
            if reader.peek_read(TokenType.SYM_COLON) is None:
                return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_COLON])

            # Try parse member expression
            type_sym = parse_type_symbol(reader)
            if type_sym.is_error():
                return type_sym.error()

            members[mem_name] = type_sym.get()

            # Try to read a comma
            if reader.peek_read(TokenType.SYM_COMMA) is None:
                break

        # Read right curly bracket
        if reader.peek_read(TokenType.SYM_RIGHT_CBRACKET) is None:
            return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_RIGHT_CBRACKET])

        return Success(ObjDef(def_id, members, position))
    elif reader.peek_read(TokenType.KW_ENUM) is not None:
        raise Exception("Not Implemented")
    elif reader.peek_read(TokenType.KW_FUNC) is not None:
        raise Exception("Not Implemented")

    return ParseError.UnexpectedToken(reader.peek(), [
        TokenType.KW_OBJECT,
        TokenType.KW_FUNC,
        TokenType.KW_ENUM])     

def parse_block(reader: TokenReader) -> Result[Statement]:
    '''Parse block'''

    position = reader.get_position()

    if reader.peek_read(TokenType.SYM_LEFT_CBRACKET) is None:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_LEFT_CBRACKET])

    stmts : List[Statement] = []

    while True:
        # Try to read a closing bracket
        if reader.peek_read(TokenType.SYM_RIGHT_CBRACKET) is not None:
            break
        elif reader.is_eof():
            return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_RIGHT_CBRACKET])

        block_stmt = parse_stmt(reader)
        if block_stmt.is_error():
            return block_stmt

        stmts.append(block_stmt.get())

    return Success(Block(stmts, position))

def parse_if_else_stmt(reader: TokenReader) -> Result[Statement]:
    '''Parse if else statement'''

    position = reader.get_position()

    if reader.peek_read(TokenType.KW_IF) is None:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.KW_IF])

    # Parse the condition expression
    condExpr = parse_expr(reader)
    if condExpr.is_error():
        return condExpr.error()

    # Parse the then clause
    thenClause = parse_block(reader)
    if thenClause.is_error():
        return thenClause

    # Attempt to parse the else clause
    if reader.peek_read(TokenType.KW_ELSE) is None:
        return Success(IfElseStmt(condExpr.get(), thenClause.get(), None, position))

    # Parse the else clause
    elseClause = parse_block(reader)
    if elseClause.is_error():
        return elseClause

    return Success(IfElseStmt(condExpr.get(), thenClause.get(), elseClause.get(), position))

def parse_stmt(reader: TokenReader) -> Result[Statement]:
    '''Parse a statement'''
    stmt : Optional[Result[Statement]] = None

    if reader.peek().type == TokenType.KW_LET: # Declaration
        stmt = parse_declaration(reader)
        if stmt.is_error():
            return stmt
    elif reader.peek().type == TokenType.KW_IF: # Declaration
        return parse_if_else_stmt(reader)
    elif reader.peek().type == TokenType.SYM_LEFT_CBRACKET: # Parse Block
        return parse_block(reader)
    else: # Expression
        expr = parse_expr(reader)
        if expr.is_error():
            return expr.error()

        stmt = Success(ExprStmt(expr.get()))

    # Try to read a semicolon
    if reader.peek_read(TokenType.SYM_SEMICOLON) is None:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_SEMICOLON])

    return stmt

def parse_module(name: str, reader: TokenReader) -> Result[Module]:
    '''Parses a stream of tokens and returns the AST module'''

    defs : List[Definition] = []
    stmts : List[Statement] = []

    while not reader.is_eof():
        if reader.peek().type == TokenType.KW_DEF:
            definition = parse_definition(reader)
            if definition.is_error():
                return definition.error()

            defs.append(definition.get())
            continue

        stmt = parse_stmt(reader)
        if stmt.is_error():
            return stmt.error()

        stmts.append(stmt.get())

    return Success(Module(name, defs, stmts))

class ParseError:
    '''Wrapper for parsing errors'''

    @staticmethod
    def DuplicateMemberName(name: str, pos: Position) -> Error:
        return Error(ErrorType.PARSING_DUP_MEMBER_NAME, pos, f"Duplicate member name '{name}' found")

    @staticmethod
    def UnexpectedToken(found: Token, expected: List[TokenType]) -> Error:
        return Error(ErrorType.PARSING_UNEXPECTED_TOKEN, found.position, f"Encountered unexpected token '{found.type.name}' but expected {[item.name for item in expected]}")

    @staticmethod
    def InvalidOperator(op: OperatorType, pos: Position) -> Error:
        return Error(ErrorType.PARSING_INVALID_OPERATOR, pos, f"Invalid operator {op}")

