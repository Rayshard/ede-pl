from enum import Enum, auto
import os
from typing import Dict, List, NamedTuple, Optional, OrderedDict, Set, cast
from .ede_ast.ede_definition import Definition, FuncDef, ObjDef
from .ede_ast.ede_expr import ArrayInitExpr, DefaultExpr, ExprType, Expression, FuncCallExpr, IdentifierExpr, ObjInitExpr, BinopExpr, BinopType, TupleInitExpr
from .ede_ast.ede_module import Module
from .ede_ast.ede_stmt import Block, ExprStmt, IfElseStmt, ReturnStmt, Statement, StmtType, VarDeclStmt
from .ede_ast.ede_type_symbol import ArrayTypeSymbol, NameTypeSymbol, PrimitiveTypeSymbol, TupleTypeSymbol, TypeSymbol
from .ede_ast.ede_typesystem import EdePrimitive
from . import ede_lexer
from .ede_token import Token, TokenType
from .ede_utils import Error, ErrorType, Position, Positioned, Result, Success, char
from .ede_ast.ede_literal import BoolLiteral, CharLiteral, IntLiteral, StringLiteral, UnitLiteral

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

class ParsingMeta(NamedTuple):
    '''
    Metadata that is passed around while parsing to
    change how nodes are parsed.
    '''

    StmtBlacklist : Set[StmtType] = set()
    ExprBlacklist : Set[StmtType] = set()

    def is_blacklisted(self, item: StmtType | ExprType) -> bool:
        match item:
            case StmtType(): return item in self.StmtBlacklist
            case ExprType(): return item in self.ExprBlacklist
            case _: return False

    def is_whitelisted(self, item: StmtType | ExprType) -> bool:
        return not self.is_blacklisted(item)

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

def parse_type_symbol(reader: TokenReader) -> Result[TypeSymbol]:
    '''Parse an type symbol'''

    position = reader.get_position()
    
    match reader.peek().type:
        case TokenType.IDENTIFIER:
            match reader.read().value:
                case "unit": return Success(PrimitiveTypeSymbol(EdePrimitive.UNIT(), position))
                case "int": return Success(PrimitiveTypeSymbol(EdePrimitive.INT(), position))
                case "string": return Success(PrimitiveTypeSymbol(EdePrimitive.STRING(), position))
                case "char": return Success(PrimitiveTypeSymbol(EdePrimitive.CHAR(), position))
                case "bool": return Success(PrimitiveTypeSymbol(EdePrimitive.BOOL(), position))
                case id: return Success(NameTypeSymbol(id, position))
        case TokenType.SYM_LEFT_SBRACKET: # Array Type Symbol
            reader.read() # Consume left square bracket

            # Get inner type symbol
            inner_type_symbol = parse_type_symbol(reader)
            if inner_type_symbol.is_error():
                return inner_type_symbol

            # Read right bracket
            if reader.peek_read(TokenType.SYM_RIGHT_SBRACKET) is not None:
                return Success(ArrayTypeSymbol(inner_type_symbol.get(), position))

            return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_RIGHT_SBRACKET])
        case TokenType.SYM_LPAREN: # Tuple Type Symbol
            reader.read() # Consume left parenthesis

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
        case _:
            return ParseError.UnexpectedToken(reader.peek(), [
                TokenType.IDENTIFIER,
                TokenType.SYM_LEFT_SBRACKET,
                TokenType.SYM_LPAREN]) 

def parse_obj_init_expr(reader: TokenReader) -> Result[Expression]:
    '''Parse object initializer expression'''

    position = reader.get_position()
    
    # Read object name
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
        for posed_mem_name in members:
            if posed_mem_name.value == mem_name.value:
                return ParseError.DuplicateName(mem_name.value, reader.prev().position)

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

def parse_csv_exprs(reader: TokenReader, delim: TokenType = TokenType.SYM_COMMA) -> Result[List[Expression]]:
    '''Parse Comma Separated Expressions'''

    # Get inner exprs
    exprs : List[Expression] = []

    while True:
        expr = parse_expr(reader)
        if expr.is_error():
            return expr.error()

        exprs.append(expr.get())

        # Try to read a comma
        if reader.peek_read(delim) is None:
            break

    return Success(exprs)

def parse_atom(reader: TokenReader) -> Result[Expression]:
    '''Parse an atom node'''

    position = reader.get_position()

    match reader.peek().type:
        case TokenType.INTEGER: return Success(IntLiteral(position, cast(int, reader.read().value)))
        case TokenType.STRING: return Success(StringLiteral(position, cast(str, reader.read().value)))
        case TokenType.CHAR: return Success(CharLiteral(position, cast(char, reader.read().value)))
        case TokenType.KW_TRUE:
            reader.read()
            return Success(BoolLiteral(position, True))
        case TokenType.KW_FALSE:
            reader.read()
            return Success(BoolLiteral(position, False))
        case TokenType.IDENTIFIER:
            id = cast(str, reader.read().value)
            
            match reader.peek().type:
                case TokenType.SYM_LEFT_CBRACKET:       # Object Initializer Expression
                    reader.unread()
                    return parse_obj_init_expr(reader)
                case TokenType.SYM_LPAREN:              # Function Call Expression
                    reader.read() # Read left parenthesis
                    
                    # Parse args
                    exprs_res = parse_csv_exprs(reader)
                    if exprs_res.is_error():
                        return exprs_res.error()

                    # Read right parenthesis
                    if reader.peek_read(TokenType.SYM_RPAREN) is None:
                        ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_RPAREN])
                        
                    return Success(FuncCallExpr(id, exprs_res.get(), position))
                case _:
                    return Success(IdentifierExpr(position, id))
        case TokenType.SYM_LEFT_SBRACKET: # Array
            reader.read() # Consume left square brack

            # Get inner exprs
            exprs = parse_csv_exprs(reader)
            if exprs.is_error():
                return exprs.error()

            # Read right bracket
            if reader.peek_read(TokenType.SYM_RIGHT_SBRACKET) is not None:
                return Success(ArrayInitExpr(exprs.get(), position))

            return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_RIGHT_SBRACKET])
        case TokenType.SYM_LPAREN:
            reader.read() # Consume left parenthesis

            final_expr = UnitLiteral(position) # Assume a unit expression will be parsed
            
            exprs_res = parse_csv_exprs(reader)
            if exprs_res.is_error():
                return exprs_res.error()
            
            match exprs_res.get():
                case []: pass                                               # Unit Literal
                case [expr]: final_expr = expr                              # Parenthesized Expression
                case exprs: final_expr = TupleInitExpr(exprs, position)     # Tuple

            # Read right parenthesis
            if reader.peek_read(TokenType.SYM_RPAREN) is not None:
                return Success(final_expr)

            return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_RPAREN])
        case TokenType.KW_DEFAULT:
            reader.read()

            # Read open parenthesis
            if reader.peek_read(TokenType.SYM_LPAREN) is None:
                return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_LPAREN])

            # Parse type symbol
            type_symbol = parse_type_symbol(reader)
            if type_symbol.is_error():
                return type_symbol.error()

            # Read closing parenthesis
            if reader.peek_read(TokenType.SYM_RPAREN) is None:
                return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_RPAREN])

            return Success(DefaultExpr(type_symbol.get()))
        case _:
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
            reader.unread()
            break

        rhs = parse_expr(reader, op_prec if is_op_right_assoc(op) else (op_prec + 1))  
        if rhs.is_error():
            return rhs

        match op:
            case OperatorType.ADD: expr = BinopExpr(op_tok.position, expr, rhs.get(), BinopType.ADD)
            case OperatorType.SUB: expr = BinopExpr(op_tok.position, expr, rhs.get(), BinopType.SUB)
            case OperatorType.MUL: expr = BinopExpr(op_tok.position, expr, rhs.get(), BinopType.MUL)
            case OperatorType.DIV: expr = BinopExpr(op_tok.position, expr, rhs.get(), BinopType.DIV)
            case OperatorType.ASSIGN:
                # Ensure that assignments have ids as their LHS
                if expr.get_expr_type() != ExprType.ID:
                    return ParseError.InvalidOperator(op, op_tok.position)

                expr = BinopExpr(op_tok.position, expr, rhs.get(), BinopType.ASSIGN)
            case _:
                raise Exception("Case not handled")

    return Success(expr)

def parse_var_decl(reader: TokenReader) -> Result[Statement]:
    '''Parse a variable declaration'''

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

def parse_definition(reader: TokenReader, meta: ParsingMeta) -> Result[Definition]:
    '''Parse a definition'''

    position = reader.get_position()

    if reader.peek_read(TokenType.KW_DEF) is None:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.KW_DEF])
    
    if reader.peek_read(TokenType.IDENTIFIER) is None:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.IDENTIFIER])

    def_id = reader.prev().value

    if reader.peek_read(TokenType.SYM_EQUALS) is None:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_EQUALS])

    match reader.peek().type:
        case TokenType.KW_OBJECT:
            reader.read() # Consume keyword
            
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
                if mem_name.value in [mem.value for mem in members]:
                    return ParseError.DuplicateName(mem_name.value, reader.prev().position)

                # Try to read an colon
                if reader.peek_read(TokenType.SYM_COLON) is None:
                    return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_COLON])

                # Try parse member type symbol
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
        case TokenType.KW_ENUM:
            raise Exception("Not Implemented")
        case TokenType.KW_FUNC:
            reader.read() # Consume keyword
            
            if reader.peek_read(TokenType.SYM_LPAREN) is None:
                return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_LPAREN])

            # Get args
            args : OrderedDict[Positioned[str], TypeSymbol] = OrderedDict()

            while True:
                # Read arg id
                if reader.peek_read(TokenType.IDENTIFIER) is None:
                    return ParseError.UnexpectedToken(reader.peek(), [TokenType.IDENTIFIER])

                arg_name = Positioned[str](reader.prev().value, reader.prev().position)

                # Ensure there are no duplicate names
                if arg_name.value in [arg.value for arg in args]:
                    return ParseError.DuplicateName(arg_name.value, reader.prev().position)

                # Try to read an colon
                if reader.peek_read(TokenType.SYM_COLON) is None:
                    return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_COLON])

                # Try parse arg type symbol
                type_sym = parse_type_symbol(reader)
                if type_sym.is_error():
                    return type_sym.error()

                args[arg_name] = type_sym.get()

                # Try to read a comma
                if reader.peek_read(TokenType.SYM_COMMA) is None:
                    break

            # Read right parenthesis
            if reader.peek_read(TokenType.SYM_RPAREN) is None:
                return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_RPAREN])

            # Read arrow
            if reader.peek_read(TokenType.SYM_ARROW) is None:
                return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_ARROW])

            # Try parse return type symbol
            ret_type_sym = parse_type_symbol(reader)
            if ret_type_sym.is_error():
                return ret_type_sym.error()

            # Parse body
            body = parse_block(reader, meta)
            if body.is_error():
                if reader.peek_read(TokenType.SYM_LAMBDA) is not None:
                    expr = parse_expr(reader)
                    if expr.is_error():
                        return expr.error()
                    elif reader.peek_read(TokenType.SYM_SEMICOLON) is None: # Try to read a semicolon
                        return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_SEMICOLON])

                    body = Success(ReturnStmt(expr.get(), expr.get().position))
                else:
                    return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_LEFT_CBRACKET, TokenType.SYM_LAMBDA])

            return Success(FuncDef(def_id, args, ret_type_sym.get(), body.get(), position))
        case _:
            return ParseError.UnexpectedToken(reader.peek(), [
                TokenType.KW_OBJECT,
                TokenType.KW_FUNC,
                TokenType.KW_ENUM])

def parse_block(reader: TokenReader, meta: ParsingMeta) -> Result[Statement]:
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

        block_stmt = parse_stmt(reader, meta)
        if block_stmt.is_error():
            return block_stmt

        stmts.append(block_stmt.get())

    return Success(Block(stmts, position))

def parse_if_else_stmt(reader: TokenReader, meta: ParsingMeta) -> Result[Statement]:
    '''Parse if else statement'''

    position = reader.get_position()

    if reader.peek_read(TokenType.KW_IF) is None:
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.KW_IF])

    # Parse the condition expression
    condExpr = parse_expr(reader)
    if condExpr.is_error():
        return condExpr.error()

    # Parse the then clause
    thenClause = parse_block(reader, meta)
    if thenClause.is_error():
        return thenClause

    # Attempt to parse the else clause
    if reader.peek_read(TokenType.KW_ELSE) is None:
        return Success(IfElseStmt(condExpr.get(), thenClause.get(), None, position))

    # Parse the else clause
    elseClause = parse_block(reader, meta)
    if elseClause.is_error():
        return elseClause

    return Success(IfElseStmt(condExpr.get(), thenClause.get(), elseClause.get(), position))

def parse_stmt(reader: TokenReader, meta: ParsingMeta) -> Result[Statement]:
    '''Parse a statement'''

    stmt : Optional[Result[Statement]] = None

    # Parse Statement
    match reader.peek().type:
        case TokenType.KW_LET if meta.is_whitelisted(StmtType.VAR_DECL):            # Var Declaration
            stmt = parse_var_decl(reader)
            if stmt.is_error():
                return stmt
        case TokenType.KW_IF if meta.is_whitelisted(StmtType.IF_ELSE):              # If Else
            return parse_if_else_stmt(reader, meta)
        case TokenType.SYM_LEFT_CBRACKET if meta.is_whitelisted(StmtType.BLOCK):    # Block
            return parse_block(reader, meta)
        case TokenType.KW_RETURN if meta.is_whitelisted(StmtType.RETURN):           # Return Statement
            position = reader.get_position()
            reader.read() # Read keyword

            if reader.peek().type == TokenType.SYM_SEMICOLON:
                stmt = Success(ReturnStmt(UnitLiteral(reader.get_position()), position))
            else:
                expr = parse_expr(reader)
                if expr.is_error():
                    return expr.error()

                stmt = Success(ReturnStmt(expr.get(), position))
        case _ if meta.is_whitelisted(StmtType.EXPR):                               # Expression Statement
            expr = parse_expr(reader)
            if expr.is_error():
                return expr.error()

            stmt = Success(ExprStmt(expr.get()))

    if stmt is None: # Could not parse statement
        return ParseError.UnexpectedToken(reader.peek(), [])
    elif reader.peek_read(TokenType.SYM_SEMICOLON) is None: # Try to read a semicolon
        return ParseError.UnexpectedToken(reader.peek(), [TokenType.SYM_SEMICOLON])

    return stmt

def parse_module(file_path:str, name: str, reader: TokenReader) -> Result[Module]:
    '''Parses a stream of tokens and returns the AST module'''

    defs : List[Definition] = []
    stmts : List[Statement] = []
    meta = ParsingMeta()

    while not reader.is_eof():
        if reader.peek().type == TokenType.KW_DEF:
            definition = parse_definition(reader, meta)
            if definition.is_error():
                return definition.error()

            defs.append(definition.get())
            continue

        stmt = parse_stmt(reader, meta)
        if stmt.is_error():
            return stmt.error()

        stmts.append(stmt.get())

    return Success(Module(file_path, name, defs, stmts))

def parse_file(file_path: str) -> Result[Module]:
    file_name = os.path.splitext(file_path)[0]
    
    with open(file_path) as f:
        lex_result = ede_lexer.tokenize(ede_lexer.Reader(f.read()))
        if lex_result.is_error():
            return lex_result.error()
        
        reader = TokenReader(lex_result.get())
        if reader.is_eof():
            return Success(Module(file_path, file_name, [], []))
        
        return parse_module(file_path, file_name, reader)

class ParseError:
    '''Wrapper for parsing errors'''

    @staticmethod
    def DuplicateName(name: str, pos: Position) -> Error:
        return Error(ErrorType.PARSING_DUP_NAME, pos, f"Duplicate name '{name}' found")

    @staticmethod
    def UnexpectedToken(found: Token, expected: List[TokenType]) -> Error:
        return Error(ErrorType.PARSING_UNEXPECTED_TOKEN, found.position, f"Encountered unexpected token '{found.type.name}' but expected {[item.name for item in expected]}")

    @staticmethod
    def InvalidOperator(op: OperatorType, pos: Position) -> Error:
        return Error(ErrorType.PARSING_INVALID_OPERATOR, pos, f"Invalid operator {op}")

