import sys
import lexer

if __name__ == '__main__':
    args = sys.argv[1:]
    assert len(args) != 0, "Expected file path"

    if args[0] == '-test':
        import tests
        tests.run()
        exit(0)

    file_path = args[0]
    
    with open(file_path) as f:
        tokens = lexer.tokenize(lexer.Reader(f.read()))
        for token in tokens:
            print(token)

