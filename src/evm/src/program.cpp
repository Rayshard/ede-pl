#include "program.h"
#include <map>
#include <regex>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include "instructions.h"
#include "../build.h"

#define LABEL_OPERAND_REGEX std::regex("@([a-zA-Z]|[0-9]|[_])+")
#define GLOBAL_ID_OPERAND_REGEX std::regex("\\$([a-zA-Z]|[0-9]|[_])+")
#define LABEL_DEF_REGEX std::regex("@([a-zA-Z]|[0-9]|[_])+:")
#define INTEGER_REGEX std::regex("-?(0|[1-9][0-9]*)")
#define UNSIGNED_INTEGER_REGEX std::regex("0|[1-9][0-9]*")
#define DECIMAL_REGEX std::regex("-?(0|[1-9][0-9]*)([.][0-9]+)?")
#define HEX_REGEX std::regex("0x[0-9a-fA-F]+")

using Instructions::OpCode, Instructions::SysCallCode, Instructions::DataType;

struct Position
{
    size_t line, column;
    Position(size_t _line = 1, size_t _col = 1) : line(_line), column(_col) {}
};

struct Token
{
    std::string value = "";
    Position position = Position();
};

struct ProgramMetadata
{
    std::map<std::string, vm_ui64> labels, globals;
    std::map<vm_ui64, Token> labelOperands;
};

namespace Error
{
    std::runtime_error Create(Position _pos, const std::string& _msg)
    {
        std::stringstream ss;
        ss << "(" << _pos.line << ", " << _pos.column << ") " << _msg;
        return std::runtime_error(ss.str());
    }

    static std::runtime_error FILE_OPEN(const std::string& _path) { return Create(Position(1, 1), std::string("Could not open file at ") + _path + "!"); }
    static std::runtime_error TOKEN_READ(Position _pos) { return Create(_pos, "Could not read token from file!"); }
    static std::runtime_error EXPECTATION(Position _pos, const std::string& _e, const std::string& _f) { return Create(_pos, "Expected " + _e + " but found " + _f + "."); }
    static std::runtime_error REDEFINED_LABEL(Position _pos, const std::string& _label) { return Create(_pos, "Label \"" + _label + "\" has already been defined!"); }
    static std::runtime_error UNDEFINED_LABEL(Position _pos, const std::string& _label) { return Create(_pos, "Label \"" + _label + "\" does not exist!"); }
    static std::runtime_error INVALID_PROGRAM() { return std::runtime_error("Invalid Program!"); }
};

class TokenStream
{
    std::istream* stream;
    Position position;
    Token last;
public:

    TokenStream(std::istream* _stream) : stream(_stream), position(), last() {}

    Position GetPosition() { return position; }
    Token GetLastToken() { return last; }

    std::string GetLine()
    {
        std::string line;
        std::getline(*stream, line);
        position.line++;
        position.column = 1;
        return line;
    }

    Token GetNextToken()
    {
        //Skips
        while (true)
        {
            if (std::isspace(stream->peek())) //Skip whitespace
            {
                position.column++;

                if (stream->peek() == '\n')
                {
                    position.line++;
                    position.column = 1;
                }

                stream->get();
            }
            else if (stream->peek() == '#') { GetLine(); } //Skip comments
            else { break; }
        }

        Position pos = position;
        std::string value;

        if (!(*stream >> value))
            throw Error::TOKEN_READ(pos);

        position.column += value.size();
        return last = Token{ value, pos };
    }

    vm_f32 ReadF32Operand()
    {
        Token token = GetNextToken();

        try
        {
            if (std::regex_match(token.value, DECIMAL_REGEX))
                return std::stof(token.value);

            throw 0;
        }
        catch (...)
        {
            throw Error::EXPECTATION(token.position, "a 32-bit floating point", "\"" + token.value + "\"");
        }
    }

    vm_f64 ReadF64Operand()
    {
        Token token = GetNextToken();

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

    vm_i64 ReadIntegerOperand(vm_i64 _min, vm_i64 _max)
    {
        Token token = GetNextToken();

        try
        {
            vm_i64 value = 0;

            if (std::regex_match(token.value, INTEGER_REGEX))
                value = std::stoll(token.value);
            else if (std::regex_match(token.value, HEX_REGEX))
                value = std::stoll(token.value, 0, 16);

            if (value < _min || value > _max)
                throw 0;

            return value;
        }
        catch (...) { throw Error::EXPECTATION(token.position, "an integer in range [" + std::to_string(_min) + ", " + std::to_string(_max) + "]", "\"" + token.value + "\""); }
    }

    vm_ui64 ReadUnsignedIntegerOperand(vm_ui64 _max)
    {
        Token token = GetNextToken();

        try
        {
            vm_ui64 value = 0;

            if (std::regex_match(token.value, UNSIGNED_INTEGER_REGEX))
                value = std::stoull(token.value);
            else if (std::regex_match(token.value, HEX_REGEX))
                value = std::stoull(token.value, 0, 16);

            if (value > _max)
                throw 0;

            return value;
        }
        catch (...) { throw Error::EXPECTATION(token.position, "an unsigned integer in range [0, " + std::to_string(_max) + "]", "\"" + token.value + "\""); }
    }

    vm_i8 ReadI8Operand() { return ReadIntegerOperand(INT8_MIN, INT8_MAX); }
    vm_ui8 ReadUI8Operand() { return ReadUnsignedIntegerOperand(UINT8_MAX); }
    vm_i16 ReadI16Operand() { return ReadIntegerOperand(INT16_MIN, INT16_MAX); }
    vm_ui16 ReadUI16Operand() { return ReadUnsignedIntegerOperand(UINT16_MAX); }
    vm_i32 ReadI32Operand() { return ReadIntegerOperand(INT32_MIN, INT32_MAX); }
    vm_ui32 ReadUI32Operand() { return ReadUnsignedIntegerOperand(UINT32_MAX); }
    vm_i64 ReadI64Operand() { return ReadIntegerOperand(INT64_MIN, INT64_MAX); }
    vm_ui64 ReadUI64Operand() { return ReadUnsignedIntegerOperand(UINT64_MAX); }

    vm_ui64 ReadHexOperand(vm_ui64 _max)
    {
        Token token = GetNextToken();

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

    DataType ReadDataTypeOperand()
    {
        static std::map<std::string, DataType> dataTypes = {
            {"I8", DataType::I8}, {"UI8", DataType::UI8},
            {"I16", DataType::I16}, {"UI16", DataType::UI16},
            {"I32", DataType::I32}, {"UI32", DataType::UI32},
            {"I64", DataType::I64}, {"UI64", DataType::UI64},
            {"F32", DataType::F32}, {"F64", DataType::F64},
        };

        Token token = GetNextToken();
        auto search = dataTypes.find(token.value);

        if (search == dataTypes.end())
            throw Error::EXPECTATION(token.position, "a data type ", "\"" + token.value + "\"");

        return search->second;
    }

    Token ReadLabelOperand()
    {
        Token token = GetNextToken();
        if (!std::regex_match(token.value, LABEL_OPERAND_REGEX))
            throw Error::EXPECTATION(token.position, "a label operand", "\"" + token.value + "\"");

        token.value.erase(0, 1); //Remove the @ prefix
        return token;
    }

    std::string ReadGlobalIDOperand()
    {
        Token token = GetNextToken();
        if (!std::regex_match(token.value, GLOBAL_ID_OPERAND_REGEX))
            throw Error::EXPECTATION(token.position, "a global id operand", "\"" + token.value + "\"");

        token.value.erase(0, 1); //Remove the $ prefix
        return token.value;
    }
};

typedef void (*InstructionInserter)(Program&, ProgramMetadata&, TokenStream&);
static const std::map<std::string, InstructionInserter> InstructionInserters = {
    {"NOOP", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::NOOP { }); }},
    {"POP", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::POP { }); }},
    {"ADD", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::ADD {.type = _stream.ReadDataTypeOperand()}); }},
    {"SUB", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::SUB {.type = _stream.ReadDataTypeOperand()}); }},
    {"MUL", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::MUL {.type = _stream.ReadDataTypeOperand()}); }},
    {"DIV", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::DIV {.type = _stream.ReadDataTypeOperand()}); }},
    {"EQ", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::EQ {.type = _stream.ReadDataTypeOperand()}); }},
    {"NEQ", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::NEQ {.type = _stream.ReadDataTypeOperand()}); }},
    {"SLOAD", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::SLOAD {.offset = _stream.ReadI64Operand()}); }},
    {"SSTORE", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::SSTORE {.offset = _stream.ReadI64Operand()}); }},
    {"DUP", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::SLOAD {.offset = -vm_i64(WORD_SIZE)}); }},
    {"MLOAD", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::MLOAD {.offset = _stream.ReadI64Operand()}); }},
    {"MSTORE", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::MSTORE {.offset = _stream.ReadI64Operand()}); }},
    {"LLOAD", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::LLOAD {.idx = _stream.ReadUI32Operand()}); }},
    {"LSTORE", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::LSTORE {.idx = _stream.ReadUI32Operand()}); }},
    {"PLOAD", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::PLOAD {.idx = _stream.ReadUI32Operand()}); }},
    {"PSTORE", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::PSTORE {.idx = _stream.ReadUI32Operand()}); }},
    {"RET", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::RET { }); }},
    {"RETV", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::RETV { }); }},
    {"EXIT", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::SYSCALL {.code = SysCallCode::EXIT}); }},
    {"MALLOC", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::SYSCALL {.code = SysCallCode::MALLOC}); }},
    {"FREE", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::SYSCALL {.code = SysCallCode::FREE}); }},
    {"PRINTC", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) { _prog.Insert(Instructions::SYSCALL {.code = SysCallCode::PRINTC}); }},
    {"PUSH", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) {
        Word value;
        
        switch (_stream.ReadDataTypeOperand())
        {
        case DataType::I8: value = _stream.ReadI8Operand(); break;
        case DataType::UI8: value = _stream.ReadUI8Operand(); break;
        case DataType::I16: value = _stream.ReadI16Operand(); break;
        case DataType::UI16: value = _stream.ReadUI16Operand(); break;
        case DataType::I32: value = _stream.ReadI32Operand(); break;
        case DataType::UI32: value = _stream.ReadUI32Operand(); break;
        case DataType::I64: value = _stream.ReadI64Operand(); break;
        case DataType::UI64: value = _stream.ReadUI64Operand(); break;
        case DataType::F32: value = _stream.ReadF32Operand(); break;
        case DataType::F64: value = _stream.ReadF64Operand(); break;
        default: assert(false && "Case not handled");
        }

        _prog.Insert(Instructions::PUSH {.value = value});
    }},
    {"CONVERT", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) {
        auto from = _stream.ReadDataTypeOperand(), to = _stream.ReadDataTypeOperand();
        _prog.Insert(Instructions::CONVERT{.from = from, .to = to});
    }},
    {"GLOAD", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) {
        auto id = _stream.ReadGlobalIDOperand();
        auto search = _progMeta.globals.find(id);
        vm_ui64 idx = search == _progMeta.globals.end() ? (_progMeta.globals[id] = _progMeta.globals.size()) : search->second;

        _prog.Insert(Instructions::GLOAD {.idx = idx});
    }},
    {"GSTORE", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) {
        auto id = _stream.ReadGlobalIDOperand();
        auto search = _progMeta.globals.find(id);
        vm_ui64 idx = search == _progMeta.globals.end() ? (_progMeta.globals[id] = _progMeta.globals.size()) : search->second;

        _prog.Insert(Instructions::GSTORE {.idx = idx});
    }},
    {"JUMP", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) {
        _prog.Insert(Instructions::JUMP {.target = VM_NULLPTR});
        _progMeta.labelOperands.emplace(_prog.GetCode().size() - VM_PTR_SIZE, _stream.ReadLabelOperand());
    }},
    {"JUMPZ", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) {
        _prog.Insert(Instructions::JUMPZ {.target = VM_NULLPTR});
        _progMeta.labelOperands.emplace(_prog.GetCode().size() - VM_PTR_SIZE, _stream.ReadLabelOperand());
    }},
    {"JUMPNZ", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) {
        _prog.Insert(Instructions::JUMPNZ {.target = VM_NULLPTR});
        _progMeta.labelOperands.emplace(_prog.GetCode().size() - VM_PTR_SIZE, _stream.ReadLabelOperand());
    }},
    {"CALL", [](Program& _prog, ProgramMetadata& _progMeta, TokenStream& _stream) {
        Token label = _stream.ReadLabelOperand();

        _prog.Insert(Instructions::CALL {.target = VM_NULLPTR, .storage = _stream.ReadUI32Operand()});
        _progMeta.labelOperands.emplace(_prog.GetCode().size() - VM_UI32_SIZE - VM_PTR_SIZE, label);
    }},
};

Program::Program() : header(), code() { }
Program::Program(Program&& _p) noexcept { this->operator=(std::move(_p)); }

void Program::Resolve()
{
    vm_byte* start = code.data(), * end = &code.back() + 1;

    for (vm_byte* ptr = start; ptr != end; ptr += GetSize((OpCode)*ptr))
    {
        OpCode opcode = (OpCode)*ptr;

        //Make sure the whole instruction is in the program
        if (end - ptr < GetSize(opcode))
            throw Error::INVALID_PROGRAM();

        //Resolve branch instructions targets
        switch (opcode)
        {
        case OpCode::JUMP: Instructions::JUMP::From(ptr)->target = start + *(vm_ui64*)&Instructions::JUMP::From(ptr)->target; break;
        case OpCode::JUMPNZ: Instructions::JUMPNZ::From(ptr)->target = start + *(vm_ui64*)&Instructions::JUMPNZ::From(ptr)->target; break;
        case OpCode::JUMPZ: Instructions::JUMPZ::From(ptr)->target = start + *(vm_ui64*)&Instructions::JUMPZ::From(ptr)->target; break;
        case OpCode::CALL: Instructions::CALL::From(ptr)->target = start + *(vm_ui64*)&Instructions::CALL::From(ptr)->target; break;
        default:
        continue;
        }
    }
}

void Program::Validate()
{
    vm_byte* start = code.data(), * end = &code.back() + 1;
    std::unordered_set<vm_byte*> possibleTargets; //a list of the targets of branching instructions

    for (vm_byte* ptr = start; ptr != end; ptr += GetSize((OpCode)*ptr))
    {
        OpCode opcode = (OpCode)*ptr;

        //Make sure the whole instruction is in the program
        if (end - ptr < GetSize(opcode))
            throw Error::INVALID_PROGRAM();


        //Assert global references are in bounds
        //Collect branching instructions targets
        switch (opcode)
        {
        case OpCode::JUMP: possibleTargets.emplace(Instructions::JUMP::From(ptr)->target); break;
        case OpCode::JUMPNZ: possibleTargets.emplace(Instructions::JUMPNZ::From(ptr)->target); break;
        case OpCode::JUMPZ: possibleTargets.emplace(Instructions::JUMPZ::From(ptr)->target); break;
        case OpCode::CALL: possibleTargets.emplace(Instructions::CALL::From(ptr)->target); break;
        case OpCode::GLOAD:
        {
            if (Instructions::GLOAD::From(ptr)->idx >= header.numGlobals)
                throw Error::INVALID_PROGRAM();
        } break;
        case OpCode::GSTORE:
        {
            if (Instructions::GSTORE::From(ptr)->idx >= header.numGlobals)
                throw Error::INVALID_PROGRAM();
        } break;
        default:
        continue;
        }
    }

    //Assert branching instructions targets point to places in the code
    for (auto& target : possibleTargets)
    {
        if (target >= end)
            throw Error::INVALID_PROGRAM();
    }
}

const vm_byte* Program::GetEntryPtr() { return &code[header.entryPoint]; }

Program Program::FromFile(const std::string& _filePath)
{
    std::ifstream file(_filePath);
    if (!file.is_open())
        throw Error::FILE_OPEN(_filePath);

    return FromStream(file);
}

Program Program::FromStream(std::istream& _stream)
{
    TokenStream stream(&_stream);
    Program program;
    ProgramMetadata metadata;

    while (true)
    {
        Token token;

        try { token = stream.GetNextToken(); }
        catch (...) { break; }

        auto instructionInserterSearch = InstructionInserters.find(token.value);
        if (instructionInserterSearch != InstructionInserters.end())
        {
            size_t insertOffset = program.code.size();
            instructionInserterSearch->second(program, metadata, stream);
            assert(program.code.size() - insertOffset == GetSize((OpCode)program.code[insertOffset]) && "Inserted instruction has different size than it should!");
        }
        else if (std::regex_match(token.value, LABEL_DEF_REGEX))
        {
            std::string label = token.value.substr(1, token.value.size() - 2);

            auto labelSearch = metadata.labels.find(label);
            if (labelSearch != metadata.labels.end())
                throw Error::REDEFINED_LABEL(token.position, label);

            metadata.labels.emplace(label, program.code.size());
        }
        else
            throw Error::EXPECTATION(token.position, "OPCODE or LABEL", token.value);
    }

    //Replace operands that are labels with the correct position in the program
    for (auto& [pos, token] : metadata.labelOperands)
    {
        auto labelSearch = metadata.labels.find(token.value);
        if (labelSearch == metadata.labels.end())
            throw Error::UNDEFINED_LABEL(token.position, token.value);

        vm_byte* target = program.code.data() + labelSearch->second;
        std::copy((vm_byte*)&target, (vm_byte*)&target + VM_PTR_SIZE, &program.code[pos]);
    }

    //Set number of globals
    for (auto& [id, idx] : metadata.globals)
        program.header.numGlobals = std::max(program.header.numGlobals, idx + 1);

#ifdef BUILD_DEBUG
    program.Validate(); //Note that the program should already be validated since we generated a valid program
#endif

    return std::move(program);
}

Program Program::FromString(const std::string& _string)
{
    std::stringstream stream(_string);
    return Program::FromStream(stream);
}

void Program::ToNASM(std::ostream& _stream)
{
    vm_byte* start = code.data(), * end = &code.back() + 1;

    //Get Label Positions
    std::unordered_map<vm_byte*, std::string> labelPositions; //Map from label pointer position to a unique string

    for (vm_byte* ptr = start; ptr != end; ptr += GetSize((OpCode)*ptr))
    {
        OpCode opcode = (OpCode)*ptr;

        switch (opcode)
        {
        case OpCode::JUMP: labelPositions.emplace(Instructions::JUMP::From(ptr)->target, "label" + std::to_string(labelPositions.size())); break;
        case OpCode::JUMPNZ: labelPositions.emplace(Instructions::JUMPNZ::From(ptr)->target, "label" + std::to_string(labelPositions.size())); break;
        case OpCode::JUMPZ: labelPositions.emplace(Instructions::JUMPZ::From(ptr)->target, "label" + std::to_string(labelPositions.size())); break;
        case OpCode::CALL: labelPositions.emplace(Instructions::CALL::From(ptr)->target, "label" + std::to_string(labelPositions.size())); break;
        default:
        continue;
        }
    }

    //Translate
    _stream << "\t\tglobal\t\tstart\n\n";
    _stream << "\t\tsection\t\t.text\n";
    _stream << "start:\n";

    for (vm_byte* ptr = start; ptr != end; ptr += GetSize((OpCode)*ptr))
    {
        _stream << "\t\t;" << Instructions::ToString(ptr) << "\n";
        Instructions::ToNASM(ptr, _stream, "\t\t");
        _stream << "\n";
    }
}