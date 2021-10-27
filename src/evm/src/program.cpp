#include "program.h"
#include <map>
#include <regex>
#include <fstream>
#include "instructions.h"

using Instructions::OpCode, Instructions::SysCallCode;

struct Position
{
    size_t line, column;

    Position(int _line, int _column) : line(_line), column(_column) {}

    Position(std::istream &_stream)
    {
        //TODO: Implement
        line = 0;
        column = 0;
    }
};

struct Token
{
    std::string value = "";
    Position position = Position(0, 0);
};

#define LABEL_OPERAND_REGEX std::regex("@([a-zA-Z]|[0-9]|[_])+")
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

Token GetNextToken(std::istream &_stream)
{
    //Skip whitespace
    while (std::isspace(_stream.peek()))
        _stream.get();

    //Skip comments
    if (_stream.peek() == '#')
    {
        std::string _;
        std::getline(_stream, _);
    }

    Position pos(_stream);
    std::string value;

    if (!(_stream >> value))
        throw Error::TOKEN_READ(pos);

    return Token{value, pos};
}

double ReadDoubleOperand(std::istream &_stream)
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

int64_t ReadInt64Operand(std::istream &_stream)
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

uint32_t ReadUInt32Operand(std::istream &_stream)
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

uint64_t ReadHexOperand(std::istream &_stream, uint64_t _max)
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

Token ReadLabelOperand(std::istream &_stream)
{
    Token token = GetNextToken(_stream);
    if (!std::regex_match(token.value, LABEL_OPERAND_REGEX))
        throw Error::EXPECTATION(token.position, "a label operand", "\"" + token.value + "\"");

    token.value.erase(0, 1); //Remove the @ prefix
    return token;
}

typedef void (*InstructionReader)(Program &, std::istream &, std::map<size_t, Token> &);
static const std::map<std::string, InstructionReader> InstructionReaders = {
    {"NOOP", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::NOOP); }},
    {"PUSH", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::PUSH, ReadHexOperand(_stream, UINT64_MAX)); }},
    {"PUSHI", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::PUSH, ReadInt64Operand(_stream)); }},
    {"PUSHD", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::PUSH, ReadDoubleOperand(_stream)); }},
    {"POP", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::POP); }},
    {"IADD", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::IADD); }},
    {"ISUB", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::ISUB); }},
    {"IMUL", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::IMUL); }},
    {"IDIV", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::IDIV); }},
    {"DADD", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::DADD); }},
    {"DSUB", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::DSUB); }},
    {"DMUL", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::DMUL); }},
    {"DDIV", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::DDIV); }},
    {"EQ", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::EQ); }},
    {"NEQ", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::NEQ); }},
    {"SLOAD", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::SLOAD, ReadInt64Operand(_stream)); }},
    {"SSTORE", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::SSTORE, ReadInt64Operand(_stream)); }},
    {"LLOAD", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::LLOAD, ReadUInt32Operand(_stream)); }},
    {"LSTORE", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::LSTORE, ReadUInt32Operand(_stream)); }},
    {"PLOAD", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::PLOAD, ReadUInt32Operand(_stream)); }},
    {"PSTORE", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::PSTORE, ReadUInt32Operand(_stream)); }},
    {"GLOAD", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::GLOAD, ReadUInt32Operand(_stream)); }},
    {"GSTORE", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &_labelReplacePositions)
     { Instructions::Insert(_prog, OpCode::GSTORE, ReadUInt32Operand(_stream)); }},
    {"JUMP", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &labelOperands)
     {
         Instructions::Insert(_prog, OpCode::JUMP);
         labelOperands.emplace(_prog.size(), ReadLabelOperand(_stream));
         Instructions::Insert(_prog, size_t());
     }},
    {"JUMPZ", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &labelOperands)
     {
         Instructions::Insert(_prog, OpCode::JUMPZ);
         labelOperands.emplace(_prog.size(), ReadLabelOperand(_stream));
         Instructions::Insert(_prog, size_t());
     }},
    {"JUMPNZ", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &labelOperands)
     {
         Instructions::Insert(_prog, OpCode::JUMPNZ);
         labelOperands.emplace(_prog.size(), ReadLabelOperand(_stream));
         Instructions::Insert(_prog, size_t());
     }},
    {"CALL", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &labelOperands)
     {
         Instructions::Insert(_prog, OpCode::CALL);

         labelOperands.emplace(_prog.size(), ReadLabelOperand(_stream));
         Instructions::Insert(_prog, size_t());                   //Insert label
         Instructions::Insert(_prog, ReadUInt32Operand(_stream)); //Insert storage
     }},
    {"RET", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &labelOperands)
     { Instructions::Insert(_prog, OpCode::RET); }},
    {"RETV", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &labelOperands)
     { Instructions::Insert(_prog, OpCode::RETV); }},
    {"EXIT", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &labelOperands)
     { Instructions::Insert(_prog, OpCode::SYSCALL, SysCallCode::EXIT); }},
    {"MALLOC", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &labelOperands)
     { Instructions::Insert(_prog, OpCode::SYSCALL, SysCallCode::MALLOC); }},
    {"FREE", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &labelOperands)
     { Instructions::Insert(_prog, OpCode::SYSCALL, SysCallCode::FREE); }},
    {"PRINTC", [](Program &_prog, std::istream &_stream, std::map<size_t, Token> &labelOperands)
     { Instructions::Insert(_prog, OpCode::SYSCALL, SysCallCode::PRINTC); }}};

namespace Instructions
{
    Program ParseFile(const std::string &_filePath)
    {
        std::ifstream file(_filePath);
        if (!file.is_open())
            throw Error::FILE_OPEN(_filePath);

        return ParseStream(file);
    }

    Program ParseStream(std::istream &_stream)
    {
        Program program;
        std::map<std::string, size_t> labels;
        std::map<size_t, Token> labelOperands;

        while (true)
        {
            Token token;

            try
            {
                token = GetNextToken(_stream);
            }
            catch (...)
            {
                break;
            }

            auto instructionReaderSearch = InstructionReaders.find(token.value);
            if (instructionReaderSearch != InstructionReaders.end())
            {
                size_t insertOffset = program.size();
                instructionReaderSearch->second(program, _stream, labelOperands);
                assert(program.size() - insertOffset == GetSize((OpCode)program[insertOffset]) && "Inserted instruction has different size than it should!");
            }
            else if (std::regex_match(token.value, LABEL_DEF_REGEX))
            {
                std::string label = token.value.substr(1, token.value.size() - 2);

                auto labelSearch = labels.find(label);
                if (labelSearch != labels.end())
                    throw Error::REDEFINED_LABEL(token.position, label);

                labels.emplace(label, program.size());
            }
            else
                throw Error::EXPECTATION(token.position, "OPCODE or LABEL", token.value);
        }

        //Replace operands that are labels with the correct position in the program
        for (auto &[pos, token] : labelOperands)
        {
            auto labelSearch = labels.find(token.value);
            if (labelSearch == labels.end())
                throw Error::UNDEFINED_LABEL(token.position, token.value);

            size_t target = labelSearch->second;
            std::copy((byte *)&target, (byte *)&target + sizeof(target), &program[pos]);
        }

        ValidateAndInitProgram(program);
        return program;
    }

    void ValidateAndInitProgram(Program &_prog)
    {
        byte *start = _prog.data(), *end = &_prog.back() + 1;

        for (byte *ptr = start; ptr != end; ptr += GetSize((OpCode)*ptr))
        {
            OpCode opcode = (OpCode)*ptr;

            //Make sure the whole instruction is in the program
            if (end - ptr < GetSize(opcode))
                throw Error::INVALID_PROGRAM();

            //Update instructions that have label operands so that the
            //operands are the actual pointers in memory
            switch (opcode)
            {
            case OpCode::JUMP:
            case OpCode::JUMPNZ:
            case OpCode::JUMPZ:
            case OpCode::CALL:
            {
                byte *operand = ptr + 1;
                size_t offset = *(size_t *)operand;
                if (offset >= _prog.size())
                    throw Error::INVALID_PROGRAM();

                byte *target = start + offset;
                if (end - target <= 0)
                    throw Error::INVALID_PROGRAM();

                std::copy((byte *)&target, (byte *)&target + sizeof(target), operand);
            }
            break;
            }
        }
    }
}