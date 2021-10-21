from ede.ede_utils import Error, ErrorType, Result, Success, char
from typing import List
from ede.ede_token import SYMBOL_DICT_INV, Position, Token, TokenType, is_keyword, is_symbol

# TODO: Comment File

EOF = '\0'

class Reader:
    def __init__(self, stream: str) -> None:
        self.stream = stream
        self.ptr = 0
        self.line = 1
        self.column = 1

    def peek(self) -> str:
        return self.stream[self.ptr] if self.ptr < len(self.stream) else EOF

    def read(self) -> str:
        char = self.peek()
        
        if char != EOF:
            self.ptr = self.ptr + 1

            if char == '\n':
                self.line = self.line + 1
                self.column = 1
            else:
                self.column = self.column + 1
        
        return char

    def get_position(self):
        return Position(self.line, self.column)

def lex_integer(reader: Reader) -> Result[Token]:
    result = ''
    position = reader.get_position()

    while True:
        char = reader.peek()
        if not char.isdigit():
            break

        result += reader.read()

    return Success(Token.Integer(position, int(result))) if result != '' else LexError.InvalidIntLit(position)

def lex_id_or_keyword(reader: Reader) -> Result[Token]:
    result = ''
    position = reader.get_position()

    # Attempt to read the initial character
    if not reader.peek().isalpha() and reader.peek() != '_':
        return LexError.InvalidID(position)
    
    result += reader.read()

    while True:
        char = reader.peek()
        if not char.isalnum() and char != '_':
            break

        result += reader.read()

    return Success(Token.Keyword(position, result) if is_keyword(result) else Token.Identifier(position, result))

def lex_string(reader: Reader) -> Result[Token]:
    result = ''
    position = reader.get_position()
    
    # Attempt to read the initial "
    if reader.peek() != '"':
        return LexError.InvalidStringLit(position)
    else:
        reader.read()

    # Read literal contents until an unescaped " or EOF is reached or invalid construction is detected
    while True:
        char = reader.peek()
        
        if char == '\\':
            reader.read()
            escaped_char = reader.read()

            if escaped_char == 't':
                result += '\t'
            elif escaped_char == 'n':
                result += '\n'
            elif escaped_char == '\\':
                result += '\\'
            elif escaped_char == '0':
                result += '\0'
            elif escaped_char == '"':
                result += '"'
            elif escaped_char == EOF:
                return LexError.UnexpectedEOF(position, "String literal must be closed with a \".")
            else:
                result += "\\" + escaped_char

            continue
        elif char == '"':
            reader.read()
            break
        elif char == EOF:
            return LexError.UnexpectedEOF(position, "String literal must be closed with a \".")

        result += reader.read()

    return Success(Token.String(position, result))

def lex_char(reader: Reader) -> Result[Token]:
    result = ''
    position = reader.get_position()
    
    # Attempt to read the initial '
    if reader.peek() != '\'':
        return LexError.InvalidCharLit(position, "Char literal must start with symbol: ' ")
    else:
        reader.read()

    # Read literal contents
    c = reader.peek()
    
    if c == '\\':
        reader.read()
        escaped_char = reader.read()

        if escaped_char == 't':
            result += '\t'
        elif escaped_char == 'n':
            result += '\n'
        elif escaped_char == '\\':
            result += '\\'
        elif escaped_char == '0':
            result += '\0'
        elif escaped_char == '"':
            result += '"'
        elif escaped_char == EOF:
            return LexError.UnexpectedEOF(position, "Char literal must be closed with a '.")
        else:
            return LexError.InvalidCharLit(position, "Char literal must contain only one character.")
    elif c == '\'':
        reader.read()
        return LexError.InvalidCharLit(position, "Char literal must contain one character.")
    elif c == EOF:
        return LexError.UnexpectedEOF(position, "Char literal must contain one character'.")
    else:
        result += reader.read()

    if reader.peek() != '\'':
        return LexError.InvalidCharLit(position, "Char literal must contain one character and be closed with a '.")
    else:
        reader.read()

    return Success(Token.Char(position, char(result)))

def lex(reader: Reader) -> Result[Token]:
    # Skip whitespace
    while reader.peek().isspace():
        reader.read()

    position = reader.get_position()
    char = reader.peek()

    if char.isdigit():
        return lex_integer(reader)
    elif char == '"':
        return lex_string(reader)
    elif char == '\'':
        return lex_char(reader)
    elif is_symbol(char):
        symbol = reader.read()
        
        while is_symbol(symbol + reader.peek()):
            symbol += reader.read()

        if symbol == SYMBOL_DICT_INV[TokenType.SYM_LINE_COMMENT]:
            value = ""

            while reader.peek() not in ['\n', EOF]:
                value += reader.read()

            return Success(Token.Comment(position, value))
        elif symbol == SYMBOL_DICT_INV[TokenType.SYM_COMMENT_OPEN]:
            value = ""

            while True:
                next_char = reader.read()

                if next_char == EOF:
                    return LexError.InvalidComment(position)
                elif next_char + reader.peek() == SYMBOL_DICT_INV[TokenType.SYM_COMMENT_CLOSE]:
                    reader.read()
                    return Success(Token.Comment(position, value))

                value += next_char

        return Success(Token.Symbol(position, symbol))
    elif char == EOF:
        return Success(Token.EOF(reader.get_position()))
    else:
        attempt = lex_id_or_keyword(reader)

        if attempt.is_success():
            return attempt
        else:
            return Success(Token.Invalid(reader.get_position(), reader.read()))

def tokenize(reader: Reader, keep_comments: bool = False) -> Result[List[Token]]:
    tokens : List[Token] = []
    
    while True:
        result = lex(reader)

        if result.is_error():
            return result.error()

        token = result.get()

        if not keep_comments and token.type == TokenType.COMMENT:
            continue

        tokens.append(token)
        if token.type == TokenType.EOF:
            break

    return Success(tokens)

class LexError:
    '''Wrapper for lexing errors'''
    
    @staticmethod
    def InvalidIntLit(pos: Position) -> Error:
        return Error(ErrorType.LEXING_INVALID_INT_LIT, pos)

    @staticmethod
    def InvalidStringLit(pos: Position) -> Error:
        return Error(ErrorType.LEXING_INVALID_STR_LIT, pos)

    @staticmethod
    def InvalidCharLit(pos: Position, why: str) -> Error:
        return Error(ErrorType.LEXING_INVALID_CHAR_LIT, pos, why)

    @staticmethod
    def InvalidComment(pos: Position) -> Error:
        return Error(ErrorType.LEXING_INVALID_COMMENT, pos, "Expected */ to close comment")

    @staticmethod
    def UnexpectedEOF(pos: Position, msg: str) -> Error:
        return Error(ErrorType.LEXING_UNEXPECTED_EOF, pos, msg)

    @staticmethod
    def InvalidID(pos: Position) -> Error:
        return Error(ErrorType.LEXING_INVALID_ID, pos, "Expected letter or _ for identifier.")