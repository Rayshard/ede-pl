from ede_utils import Error, ErrorType, Result, Success, char
from typing import List, cast
from ede_token import Position, Token, TokenType, is_keyword, is_symbol

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

    return Success(Token.Integer(position, int(result))) if result != '' else Error(ErrorType.INVALID_INT_LIT, position)

def lex_id_or_keyword(reader: Reader) -> Result[Token]:
    result = ''
    position = reader.get_position()

    # Attempt to read the initial character
    if not reader.peek().isalpha() and reader.peek() != '_':
        return Error(ErrorType.INVALID_ID, position, "Expected letter or _ for identifier.")
    
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
        return Error(ErrorType.INVALID_STR_LIT, position, "Expected \" to begin string literal.")
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
                return Error(ErrorType.UNEXPECTED_EOF, position, "String literal must be closed with a \".")
            else:
                result += "\\" + escaped_char

            continue
        elif char == '"':
            reader.read()
            break
        elif char == EOF:
            return Error(ErrorType.UNEXPECTED_EOF, position, "String literal must be closed with a \".")

        result += reader.read()

    return Success(Token.String(position, result))

def lex_char(reader: Reader) -> Result[Token]:
    result = ''
    position = reader.get_position()
    
    # Attempt to read the initial '
    if reader.peek() != '\'':
        return Error(ErrorType.INVALID_CHAR_LIT, position, "Expected ' to begin char literal.")
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
            return Error(ErrorType.UNEXPECTED_EOF, position, "Char literal must be closed with a '.")
        else:
            return Error(ErrorType.INVALID_CHAR_LIT, position, "Char literal must contain only one character.")
    elif c == '\'':
        reader.read()
        return Error(ErrorType.INVALID_CHAR_LIT, position, "Char literal must contain one character.")
    elif c == EOF:
        return Error(ErrorType.UNEXPECTED_EOF, position, "Char literal must contain one character'.")
    else:
        result += reader.read()

    if reader.peek() != '\'':
        return Error(ErrorType.INVALID_CHAR_LIT, position, "Char literal must be closed with a '.")
    else:
        reader.read()

    return Success(Token.Char(position, char(result)))

def lex(reader: Reader) -> Result[Token]:
    # Skip whitespace
    while reader.peek().isspace():
        reader.read()

    char = reader.peek()

    if char.isdigit():
        return lex_integer(reader)
    elif char == '"':
        return lex_string(reader)
    elif char == '\'':
        return lex_char(reader)
    elif is_symbol(char):
        return Success(Token.Symbol(reader.get_position(), reader.read()))
    elif char == EOF:
        return Success(Token.EOF(reader.get_position()))
    else:
        attempt = lex_id_or_keyword(reader)

        if attempt.is_success():
            return attempt
        else:
            return Success(Token.Invalid(reader.get_position(), reader.read()))

def tokenize(reader: Reader) -> Result[List[Token]]:
    tokens : List[Token] = []
    
    while True:
        result = lex(reader)

        if result.is_error():
            return cast(Error, result)

        tokens.append(result.get())
        if tokens[-1].type == TokenType.EOF:
            break

    return Success(tokens)
