from ede_utils import Error
import sys
import lexer

if __name__ == '__main__':
    args = sys.argv[1:]
    assert len(args) != 0, "Expected file path"

    file_path = args[0]
    
    with open(file_path) as f:
        result = lexer.tokenize(lexer.Reader(f.read()))

        if result.is_error():
            print(result)
        else:
            for token in result.get():
                print(token)

