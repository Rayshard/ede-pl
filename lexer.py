from typing import List
from ede_token import Position, Token, TokenType, is_symbol

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


def lex_integer(reader: Reader) -> Token:
    result = ''
    position = reader.get_position()

    while True:
        char = reader.peek()
        if not char.isdigit():
            break

        result += reader.read()

    return Token.Integer(position, int(result)) if result != '' else Token.Invalid(position, reader.peek())

def lex_string(reader: Reader) -> Token:
    result = ''
    position = reader.get_position()
    
    # Attempt to read the initial "
    if reader.peek() != '"':
        return Token.Invalid(position, reader.peek())
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
            elif escaped_char == '"':
                result += '"'
            elif escaped_char == EOF:
                return Token.Invalid(position, result + '\\')
            else:
                result += "\\" + escaped_char

            continue
        elif char == '"':
            reader.read()
            break
        elif char == EOF:
            return Token.Invalid(position, result)

        result += reader.read()

    return Token.String(position, result)

def lex(reader: Reader) -> Token:
    # Skip whitespace
    while reader.peek().isspace():
        reader.read()

    char = reader.peek()

    if char.isdigit():
        return lex_integer(reader)
    elif char == '"':
        return lex_string(reader)
    elif is_symbol(char):
        return Token.Symbol(reader.get_position(), reader.read())
    elif char == EOF:
        return Token.EOF(reader.get_position())
    else:
        return Token.Invalid(reader.get_position(), reader.read())

def tokenize(reader: Reader) -> List[Token]:
    tokens = []
    
    while True:
        tokens.append(lex(reader))
        if tokens[-1].type == TokenType.EOF:
            break

    return tokens
