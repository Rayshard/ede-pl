#include "program.h"
#include <map>
#include <regex>
#include <iostream>
#include <fstream>
#include "instructions.h"

using Instructions::OpCode, Instructions::SysCallCode;

struct Position
{
    size_t line, column;
    Position(size_t _line = 1, size_t _col = 1) : line(_line), column(_col) {}
};

struct Token
{
    std::string value = "";
    Position position;
};

#define LABEL_OPERAND_REGEX std::regex("@([a-zA-Z]|[0-9]|[_])+")
#define GLOBAL_ID_OPERAND_REGEX std::regex("\\$([a-zA-Z]|[0-9]|[_])+")
#define LABEL_DEF_REGEX std::regex("@([a-zA-Z]|[0-9]|[_])+:")
#define INTEGER_REGEX std::regex("-?(0|[1-9][0-9]*)")
#define UNSIGNED_INTEGER_REGEX std::regex("0|[1-9][0-9]*")
#define DECIMAL_REGEX std::regex("-?(0|[1-9][0-9]*)([.][0-9]+)?")
#define HEX_REGEX std::regex("0x[0-9a-fA-F]+")

namespace Error
{
    std::runtime_error Create(Position _pos, const std::string &_msg)
    {
        std::stringstream ss;
        ss << "(" << _pos.line << ", " << _pos.column << ") " << _msg;
        return std::runtime_error(ss.str());
    }

    static std::runtime_error FILE_OPEN(const std::string &_path) { return Create(Position(1, 1), std::string("Could not open file at ") + _path + "!"); }
    static std::runtime_error TOKEN_READ(Position _pos) { return Create(_pos, "Could not read token from file!"); }
    static std::runtime_error EXPECTATION(Position _pos, const std::string &_e, const std::string &_f) { return Create(_pos, "Expected " + _e + " but found " + _f + "."); }
    static std::runtime_error REDEFINED_LABEL(Position _pos, const std::string &_label) { return Create(_pos, "Label \"" + _label + "\" has already been defined!"); }
    static std::runtime_error UNDEFINED_LABEL(Position _pos, const std::string &_label) { return Create(_pos, "Label \"" + _label + "\" does not exist!"); }
    static std::runtime_error INVALID_PROGRAM() { return std::runtime_error("Invalid Program!"); }
};

class TokenStream
{
    std::istream *stream;

public:
    Position position;

    TokenStream(std::istream *_stream) : stream(_stream) {}

    int Peek() { return stream->peek(); }
    int Get() { return stream->get(); }

    std::string GetLine()
    {
        std::string line;
        std::getline(*stream, line);
        return line;
    }

    template <typename T>
    std::istream &operator>>(T &_obj) { return *stream >> _obj; }
};

Token GetNextToken(TokenStream &_stream)
{
    //Skip whitespace
    while (std::iswspace(_stream.Peek()))
    {
        _stream.position.column++;

        if (_stream.Peek() == '\n')
        {
            _stream.position.line++;
            _stream.position.column = 1;
        }

        _stream.Get();
    }

    //Skip comments
    if (_stream.Peek() == '#')
        _stream.GetLine();

    Position pos = _stream.position;
    std::string value;

    if (!(_stream >> value))
        throw Error::TOKEN_READ(pos);

    return Token{value, pos};
}

vm_f64 ReadF64Operand(TokenStream &_stream)
{
    Token token = GetNextToken(_stream);

    try
    {
        if (std::regex_match(token.value, DECIMAL_REGEX))
            return std::stod(token.value);

        throw 0;
    }
    catch (...)
    {
        throw Error::EXPECTATION(token.position, "a 64-bit floating point", "\"" + token.value + "\"");
    }
}

vm_i64 ReadI64Operand(TokenStream &_stream)
{
    Token token = GetNextToken(_stream);

    try
    {
        if (std::regex_match(token.value, INTEGER_REGEX))
            return std::stoll(token.value);
        else if (std::regex_match(token.value, HEX_REGEX))
            return std::stoll(token.value, 0, 16);

        throw 0;
    }
    catch (...)
    {
        throw Error::EXPECTATION(token.position, "a 64-bit integer", "\"" + token.value + "\"");
    }
}

vm_ui32 ReadUI32Operand(TokenStream &_stream)
{
    Token token = GetNextToken(_stream);

    try
    {
        if (std::regex_match(token.value, UNSIGNED_INTEGER_REGEX))
            return std::stoul(token.value);
        else if (std::regex_match(token.value, HEX_REGEX))
            return std::stoul(token.value, 0, 16);

        throw 0;
    }
    catch (...)
    {
        throw Error::EXPECTATION(token.position, "a 32-bit unsigned integer", "\"" + token.value + "\"");
    }
}

vm_ui64 ReadHexOperand(TokenStream &_stream, vm_ui64 _max)
{
    Token token = GetNextToken(_stream);

    try
    {
        if (!std::regex_match(token.value, HEX_REGEX))
            throw 0;

        auto value = std::stoull(token.value, 0, 16);
        if (value > _max)
            throw 0;

        return value;
    }
    catch (...)
    {
        throw Error::EXPECTATION(token.position, "a hex number no greater than " + Hex(_max), "\"" + token.value + "\"");
    }
}

Token ReadLabelOperand(TokenStream &_stream)
{
    Token token = GetNextToken(_stream);
    if (!std::regex_match(token.value, LABEL_OPERAND_REGEX))
        throw Error::EXPECTATION(token.position, "a label operand", "\"" + token.value + "\"");

    token.value.erase(0, 1); //Remove the @ prefix
    return token;
}

vm_ui64 ReadGlobalIDOperand(TokenStream &_stream, std::map<std::string, vm_ui64> &_globals)
{
    Token token = GetNextToken(_stream);
    if (!std::regex_match(token.value, GLOBAL_ID_OPERAND_REGEX))
        throw Error::EXPECTATION(token.position, "a global id operand", "\"" + token.value + "\"");

    token.value.erase(0, 1); //Remove the $ prefix

    auto search = _globals.find(token.value);
    if (search == _globals.end())
    {
        vm_ui64 numGlobals = _globals.size();
        _globals[token.value] = numGlobals;
        return numGlobals;
    }
    else
        return search->second;
}

typedef void (*InstructionReader)(Program &, TokenStream &, std::map<vm_ui64, Token> &, std::map<std::string, vm_ui64> &);
static const std::map<std::string, InstructionReader> InstructionReaders = {
    {"NOOP", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::NOOP); }},
    {"PUSH", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::PUSH, ReadHexOperand(_stream, UINT64_MAX)); }},
    {"PUSHI", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::PUSH, ReadI64Operand(_stream)); }},
    {"PUSHD", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::PUSH, ReadF64Operand(_stream)); }},
    {"POP", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::POP); }},
    {"IADD", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::IADD); }},
    {"ISUB", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::ISUB); }},
    {"IMUL", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::IMUL); }},
    {"IDIV", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::IDIV); }},
    {"DADD", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::DADD); }},
    {"DSUB", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::DSUB); }},
    {"DMUL", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::DMUL); }},
    {"DDIV", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::DDIV); }},
    {"EQ", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::EQ); }},
    {"NEQ", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::NEQ); }},
    {"SLOAD", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::SLOAD, ReadI64Operand(_stream)); }},
    {"SSTORE", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::SSTORE, ReadI64Operand(_stream)); }},
    {"MLOAD", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::MLOAD, ReadI64Operand(_stream)); }},
    {"MSTORE", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::MSTORE, ReadI64Operand(_stream)); }},
    {"LLOAD", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::LLOAD, ReadUI32Operand(_stream)); }},
    {"LSTORE", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::LSTORE, ReadUI32Operand(_stream)); }},
    {"PLOAD", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::PLOAD, ReadUI32Operand(_stream)); }},
    {"PSTORE", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::PSTORE, ReadUI32Operand(_stream)); }},
    {"GLOAD", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::GLOAD, ReadGlobalIDOperand(_stream, _globals)); }},
    {"GSTORE", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::GSTORE, ReadGlobalIDOperand(_stream, _globals)); }},
    {"DUP", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::SSTORE, -vm_i64(WORD_SIZE)); }},
    {"JUMP", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     {
         _prog.Insert(OpCode::JUMP);
         _labelReplacePositions.emplace(_prog.GetCode().size(), ReadLabelOperand(_stream));
         _prog.Insert(vm_ui64());
     }},
    {"JUMPZ", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     {
         _prog.Insert(OpCode::JUMPZ);
         _labelReplacePositions.emplace(_prog.GetCode().size(), ReadLabelOperand(_stream));
         _prog.Insert(vm_ui64());
     }},
    {"JUMPNZ", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     {
         _prog.Insert(OpCode::JUMPNZ);
         _labelReplacePositions.emplace(_prog.GetCode().size(), ReadLabelOperand(_stream));
         _prog.Insert(vm_ui64());
     }},
    {"CALL", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     {
         _prog.Insert(OpCode::CALL);

         _labelReplacePositions.emplace(_prog.GetCode().size(), ReadLabelOperand(_stream));
         _prog.Insert(vm_ui64());                //Insert label
         _prog.Insert(ReadUI32Operand(_stream)); //Insert storage
     }},
    {"RET", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::RET); }},
    {"RETV", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::RETV); }},
    {"EXIT", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::SYSCALL, SysCallCode::EXIT); }},
    {"MALLOC", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::SYSCALL, SysCallCode::MALLOC); }},
    {"FREE", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::SYSCALL, SysCallCode::FREE); }},
    {"PRINTC", [](Program &_prog, TokenStream &_stream, std::map<vm_ui64, Token> &_labelReplacePositions, std::map<std::string, vm_ui64> &_globals)
     { _prog.Insert(OpCode::SYSCALL, SysCallCode::PRINTC); }}};

Program::Program() : header(), code() {}
Program::Program(Program &&_p) noexcept { this->operator=(std::move(_p)); }

void Program::ValidateAndInit()
{
    vm_byte *start = code.data(), *end = &code.back() + 1;

    for (vm_byte *ptr = start; ptr != end; ptr += GetSize((OpCode)*ptr))
    {
        OpCode opcode = (OpCode)*ptr;

        //Make sure the whole instruction is in the program
        if (end - ptr < GetSize(opcode))
            throw Error::INVALID_PROGRAM();

        //Update instructions that have label operands so that the
        //operands are the actual pointers in memory; Update program
        //globals count
        switch (opcode)
        {
        case OpCode::JUMP:
        case OpCode::JUMPNZ:
        case OpCode::JUMPZ:
        case OpCode::CALL:
        {
            vm_byte *operand = ptr + 1;
            vm_ui64 offset = *(vm_ui64 *)operand;
            if (offset >= code.size())
                throw Error::INVALID_PROGRAM();

            vm_byte *target = start + offset;
            if (end - target <= 0)
                throw Error::INVALID_PROGRAM();

            std::copy((vm_byte *)&target, (vm_byte *)&target + VM_PTR_SIZE, operand);
        }
        break;
        case OpCode::GLOAD:
        case OpCode::GSTORE:
        {
            vm_ui64 idx = *(vm_ui64 *)(ptr + 1);
            header.numGlobals = std::max(header.numGlobals, idx + 1); //Set globals count
        }
        break;
        default:
            continue;
        }
    }
}

vm_byte *Program::GetEntryPtr() { return &code[header.entryPoint]; }

Program Program::FromFile(const std::string &_filePath)
{
    std::ifstream file(_filePath);
    if (!file.is_open())
        throw Error::FILE_OPEN(_filePath);

    return FromStream(file);
}

Program Program::FromStream(std::istream &_stream)
{
    TokenStream stream(&_stream);
    Program program;
    std::map<std::string, vm_ui64> labels, globals;
    std::map<vm_ui64, Token> labelReplacePositions;

    while (true)
    {
        Token token;

        try
        {
            token = GetNextToken(stream);
        }
        catch (...)
        {
            break;
        }

        auto instructionReaderSearch = InstructionReaders.find(token.value);
        if (instructionReaderSearch != InstructionReaders.end())
        {
            size_t insertOffset = program.code.size();
            instructionReaderSearch->second(program, stream, labelReplacePositions, globals);
            assert(program.code.size() - insertOffset == GetSize((OpCode)program.code[insertOffset]) && "Inserted instruction has different size than it should!");
        }
        else if (std::regex_match(token.value, LABEL_DEF_REGEX))
        {
            std::string label = token.value.substr(1, token.value.size() - 2);

            auto labelSearch = labels.find(label);
            if (labelSearch != labels.end())
                throw Error::REDEFINED_LABEL(token.position, label);

            labels.emplace(label, program.code.size());
        }
        else
            throw Error::EXPECTATION(token.position, "OPCODE or LABEL", token.value);
    }

    //Replace operands that are labels with the correct position in the program
    for (auto &[pos, token] : labelReplacePositions)
    {
        auto labelSearch = labels.find(token.value);
        if (labelSearch == labels.end())
            throw Error::UNDEFINED_LABEL(token.position, token.value);

        vm_ui64 target = labelSearch->second;
        std::copy((vm_byte *)&target, (vm_byte *)&target + VM_PTR_SIZE, &program.code[pos]);
    }

    program.ValidateAndInit();
    return std::move(program);
}
