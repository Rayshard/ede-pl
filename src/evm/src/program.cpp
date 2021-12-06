#include "program.h"
#include <map>
#include <regex>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include "instructions.h"
#include "../build.h"
#include "deps/lpc.h"

using namespace lpc;
using Instructions::OpCode, Instructions::SysCallCode, Instructions::DataType;

namespace Error
{
    std::runtime_error Create(Position _pos, const std::string& _msg)
    {
        std::stringstream ss;
        ss << "(" << _pos.line << ", " << _pos.column << ") " << _msg;
        return std::runtime_error(ss.str());
    }

    static std::runtime_error FILE_OPEN(const std::string& _path) { return Create(Position{ 1, 1 }, std::string("Could not open file at ") + _path + "!"); }
    static ParseError REDEFINED_LABEL(Position _pos, const std::string& _label) { return ParseError(_pos, "Label \"" + _label + "\" has already been defined!"); }
    static ParseError UNDEFINED_LABEL(Position _pos, const std::string& _label) { return ParseError(_pos, "Label \"" + _label + "\" does not exist!"); }
    static ParseError REDEFINED_GLOBAL(Position _pos, const std::string& _global) { return ParseError(_pos, "Global \"" + _global + "\" has already been defined!"); }
    static ParseError UNDEFINED_GLOBAL(Position _pos, const std::string& _global) { return ParseError(_pos, "Global \"" + _global + "\" does not exist!"); }
    static std::runtime_error INVALID_PROGRAM() { return std::runtime_error("Invalid Program!"); }
};

vm_i64 MapInteger(const ParseResult<std::string>& _result, vm_i64 _min, vm_i64 _max)
{
    try
    {
        vm_i64 value = std::stoll(_result.value);
        if (value < _min || value > _max)
            throw 0;

        return value;
    }
    catch (...) { throw ParseError(_result.position, "Expected an integer in range [" + std::to_string(_min) + ", " + std::to_string(_max) + "]"); }
}

vm_ui64 MapUnsignedInteger(const ParseResult<std::string>& _result, vm_ui64 _max)
{
    try
    {
        vm_ui64 value = std::stoull(_result.value);
        if (value > _max)
            throw 0;

        return value;
    }
    catch (...) { throw ParseError(_result.position, "Expected an unsigned integer in range [0, " + std::to_string(_max) + "]"); }
}

vm_i8 MapI8(const ParseResult<std::string>& _result) { return MapInteger(_result, INT8_MIN, INT8_MAX); }
vm_ui8 MapUI8(const ParseResult<std::string>& _result) { return MapUnsignedInteger(_result, UINT8_MAX); }
vm_i16 MapI16(const ParseResult<std::string>& _result) { return MapInteger(_result, INT16_MIN, INT16_MAX); }
vm_ui16 MapUI16(const ParseResult<std::string>& _result) { return MapUnsignedInteger(_result, UINT16_MAX); }
vm_i32 MapI32(const ParseResult<std::string>& _result) { return MapInteger(_result, INT32_MIN, INT32_MAX); }
vm_ui32 MapUI32(const ParseResult<std::string>& _result) { return MapUnsignedInteger(_result, UINT32_MAX); }
vm_i64 MapI64(const ParseResult<std::string>& _result) { return MapInteger(_result, INT64_MIN, INT64_MAX); }
vm_ui64 MapUI64(const ParseResult<std::string>& _result) { return MapUnsignedInteger(_result, UINT64_MAX); }

vm_f32 MapF32(const ParseResult<std::string>& _result)
{
    try { return std::stof(_result.value); }
    catch (...) { throw ParseError(_result.position, "Expected a 32-bit floating point number"); }
}

vm_f64 MapF64(const ParseResult<std::string>& _result)
{
    try { return std::stod(_result.value); }
    catch (...) { throw ParseError(_result.position, "Expected a 64-bit floating point number"); }
}

Parser<Memory> CreateCodeParser(Memory& _code, const Parser<Memory>& _instructionParser)
{
    return _instructionParser.Chain<Memory>([=, code = &_code](auto _result)
        {
            code->insert(code->end(), _result.value.begin(), _result.value.end());
            return CreateCodeParser(*code, _instructionParser);
        })
        | Value("CODE", _code);
}

struct Label
{
    Position position; std::string value;

    Label(Position _pos, const std::string& _value) : position(_pos), value(_value) {}
    Label() : Label(Position(), "") { }
};

struct Instruction
{
    Memory code;
    std::map<std::string, std::any> metadata;

    Instruction() : code(), metadata() { }
    Instruction(const Memory& _code, std::map<std::string, std::any> _metadata = {}) : code(_code), metadata(_metadata) { }

    OpCode GetOpCode() { return *(OpCode*)&code.front(); }
};

class ProgramParseValue
{
    std::map<std::string, vm_ui64> labels;
    std::map<vm_ui64, Label> labelOperands;
    Memory code;

public:
    void AppendCode(const Memory& _code) { code.insert(code.end(), _code.begin(), _code.end()); }

    void AddLabel(const Label& _label)
    {
        auto labelSearch = labels.find(_label.value);
        if (labelSearch != labels.end())
            throw Error::REDEFINED_LABEL(_label.position, _label.value);

        labels[_label.value] = code.size();
    }

    void AddLabelOperand(vm_ui64 _codePoint, const Label& _label)
    {
        auto labelOpSearch = labelOperands.find(_codePoint);
        assert(labelOpSearch == labelOperands.end() && "Code point is already in use!");

        labelOperands[_codePoint] = _label;
    }

    const Memory& GetCode() const { return code; }
    const std::map<vm_ui64, Label>& GetLabelOperands() const { return labelOperands; }
    const std::map<std::string, vm_ui64>& GetLabels() const { return labels; }
};

lpc::LPC<ProgramParseValue> CreateLPC()
{
    auto unknownAction = [](StringStream& _stream, const Lexer::Token& _token) { throw std::runtime_error("Unrecognized token: " + _token.value + " at " + _token.position.ToString()); };
    Lexer lexer(std::monostate(), unknownAction);
    lexer.AddPattern("WS", Regex("\\s+"));
    lexer.AddPattern("COMMENT", Regex("#.*"));

    auto LABEL = lexer.AddPattern("LABEL", Regex("@([a-zA-Z]|[0-9]|[_])+")).AsTerminal().Map<Label>([](auto _result) { return Label(_result.position, _result.value.substr(1, _result.value.size() - 1)); });
    auto LABEL_DEF = LABEL << Char("COLON", ':');
    auto GLOBAL_ID = lexer.AddPattern("GLOBAL_ID", Regex("\\$([a-zA-Z]|[0-9]|[_])+")).AsTerminal().Map<std::string>([](auto _result) { return _result.value.substr(1); });
    auto INTEGER = lexer.AddPattern("INTEGER", Regex("-?(0|[1-9][0-9]*)")).AsTerminal();
    auto DECIMAL = lexer.AddPattern("DECIMAL", Regex("-?(0|[1-9][0-9]*)([.][0-9]+)?")).AsTerminal();
    auto HEX = lexer.AddPattern("HEX", Regex("0x[0-9a-fA-F]+")).AsTerminal().Map<vm_ui64>([](auto _result) { return std::stoull(_result.value); });
    auto OPCODE = lexer.AddPattern("OPCODE", Regex("PUSH|EXIT|JUMP|NOOP|POP|ADD|SUB|MUL|DIV|EQ|NEQ|DUP|SLOAD|STORE|MLOAD|MSTORE"));

    static std::map<std::string, DataType> dataTypes =
    {
        {"I8", DataType::I8}, {"UI8", DataType::UI8},
        {"I16", DataType::I16}, {"UI16", DataType::UI16},
        {"I32", DataType::I32}, {"UI32", DataType::UI32},
        {"I64", DataType::I64}, {"UI64", DataType::UI64},
        {"F32", DataType::F32}, {"F64", DataType::F64},
    };
    auto DATA_TYPE = lexer.AddPattern("DATA_TYPE", Regex("(U?I(8|16|32|64))|(F(32|64))")).AsTerminal().Map<DataType>([](auto _result) { return dataTypes.at(_result.value); });
    auto I8 = INTEGER.Map<vm_i8>(MapI8);
    auto UI8 = INTEGER.Map<vm_ui8>(MapUI8);
    auto I16 = INTEGER.Map<vm_i16>(MapI16);
    auto UI16 = INTEGER.Map<vm_ui16>(MapUI16);
    auto I32 = INTEGER.Map<vm_i32>(MapI32);
    auto UI32 = INTEGER.Map<vm_ui32>(MapUI32);
    auto I64 = INTEGER.Map<vm_i64>(MapI64);
    auto UI64 = INTEGER.Map<vm_ui64>(MapUI64);
    auto F32 = DECIMAL.Map<vm_f32>(MapF32);
    auto F64 = DECIMAL.Map<vm_f64>(MapF64);

    typedef TryValue<Instruction, std::monostate> InstructionParseValue;

    auto INSTR_PUSH = OPCODE.AsTerminal("PUSH") >> Try(Parser(
            Parser(Prefixed("PUSH I8 <8-bit integer>", DATA_TYPE.Satisfy(DataType::I8), I8.Map<Word>([](auto result) { return Word(result.value); }))
                | Prefixed("PUSH UI8 <8-bit unsigned integer>", DATA_TYPE.Satisfy(DataType::UI8), UI8.Map<Word>([](auto result) { return Word(result.value); }))
                | Prefixed("PUSH I16 <16-bit integer>", DATA_TYPE.Satisfy(DataType::I16), I16.Map<Word>([](auto result) { return Word(result.value); }))
                | Prefixed("PUSH UI16 <16-bit unsigned integer>", DATA_TYPE.Satisfy(DataType::UI16), UI16.Map<Word>([](auto result) { return Word(result.value); }))
                | Prefixed("PUSH I32 <32-bit integer>", DATA_TYPE.Satisfy(DataType::I32), I32.Map<Word>([](auto result) { return Word(result.value); }))
                | Prefixed("PUSH UI32 <32-bit unsigned integer>", DATA_TYPE.Satisfy(DataType::UI32), UI32.Map<Word>([](auto result) { return Word(result.value); }))
                | Prefixed("PUSH I64 <64-bit integer>", DATA_TYPE.Satisfy(DataType::I64), I64.Map<Word>([](auto result) { return Word(result.value); }))
                | Prefixed("PUSH UI64 <64-bit unsigned integer>", DATA_TYPE.Satisfy(DataType::UI64), UI64.Map<Word>([](auto result) { return Word(result.value); }))
                | Prefixed("PUSH F32 <32-bit float>", DATA_TYPE.Satisfy(DataType::F32), F32.Map<Word>([](auto result) { return Word(result.value); }))
                | Prefixed("PUSH F64 <64-bit float>", DATA_TYPE.Satisfy(DataType::F64), F64.Map<Word>([](auto result) { return Word(result.value); }))
            ).Map<Instruction>([](auto _result) { return GetBytes(Instructions::PUSH{ .value = _result.value }); })
            | LABEL.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::PUSH{ .value = VM_NULLPTR }), { {"LABEL", _result.value} }); })));

    auto INSTR_CALL = OPCODE.AsTerminal("CALL") >> Try(Parser(LABEL & UI32).Map<Instruction>([](auto _result)
        {
            auto [label, storage] = _result.value;
            return Instruction(GetBytes(Instructions::CALL{ .target = VM_NULLPTR, .storage = storage.value }), { {"LABEL", label.value} });
        }));

    auto INSTR_CONVERT = OPCODE.AsTerminal("CONVERT") >> Try(Parser(DATA_TYPE + DATA_TYPE).Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::CONVERT{ .from = _result.value[0].value, .to = _result.value[1].value })); }));
    auto INSTR_JUMP = OPCODE.AsTerminal("JUMP") >> Try(LABEL.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::JUMP{ .target = VM_NULLPTR }), { {"LABEL", _result.value} }); }));
    auto INSTR_JUMPZ = OPCODE.AsTerminal("JUMPZ") >> Try(LABEL.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::JUMPZ{ .target = VM_NULLPTR }), { {"LABEL", _result.value} }); }));
    auto INSTR_JUMPNZ = OPCODE.AsTerminal("JUMPNZ") >> Try(LABEL.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::JUMPNZ{ .target = VM_NULLPTR }), { {"LABEL", _result.value} }); }));
    auto INSTR_ADD = OPCODE.AsTerminal("ADD") >> Try(DATA_TYPE.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::ADD{ .type = _result.value })); }));
    auto INSTR_SUB = OPCODE.AsTerminal("SUB") >> Try(DATA_TYPE.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::SUB{ .type = _result.value })); }));
    auto INSTR_MUL = OPCODE.AsTerminal("MUL") >> Try(DATA_TYPE.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::MUL{ .type = _result.value })); }));
    auto INSTR_DIV = OPCODE.AsTerminal("DIV") >> Try(DATA_TYPE.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::DIV{ .type = _result.value })); }));
    auto INSTR_EQ = OPCODE.AsTerminal("EQ") >> Try(DATA_TYPE.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::EQ{ .type = _result.value })); }));
    auto INSTR_NEQ = OPCODE.AsTerminal("NEQ") >> Try(DATA_TYPE.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::NEQ{ .type = _result.value })); }));
    auto INSTR_DUP = OPCODE.AsTerminal("DUP") >> Try(I64.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::SLOAD{ .offset = -vm_i64(WORD_SIZE) })); }));
    auto INSTR_SLOAD = OPCODE.AsTerminal("SLOAD") >> Try(I64.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::SLOAD{ .offset = _result.value })); }));
    auto INSTR_SSTORE = OPCODE.AsTerminal("SSTORE") >> Try(I64.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::SSTORE{ .offset = _result.value })); }));
    auto INSTR_MLOAD = OPCODE.AsTerminal("MLOAD") >> Try(I64.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::MLOAD{ .offset = _result.value })); }));
    auto INSTR_MSTORE = OPCODE.AsTerminal("MSTORE") >> Try(I64.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::MSTORE{ .offset = _result.value })); }));
    auto INSTR_LLOAD = OPCODE.AsTerminal("LLOAD") >> Try(UI32.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::LLOAD{ .idx = _result.value })); }));
    auto INSTR_LSTORE = OPCODE.AsTerminal("LSTORE") >> Try(UI32.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::LSTORE{ .idx = _result.value })); }));
    auto INSTR_PLOAD = OPCODE.AsTerminal("PLOAD") >> Try(UI32.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::PLOAD{ .idx = _result.value })); }));
    auto INSTR_PSTORE = OPCODE.AsTerminal("PSTORE") >> Try(UI32.Map<Instruction>([](auto _result) { return Instruction(GetBytes(Instructions::PSTORE{ .idx = _result.value })); }));
    auto INSTR_NOOP = OPCODE.AsTerminal("NOOP").Map<InstructionParseValue>([](auto _) { return InstructionParseValue(GetBytes(Instructions::NOOP{ })); });
    auto INSTR_RET = OPCODE.AsTerminal("RET").Map<InstructionParseValue>([](auto _) { return InstructionParseValue(GetBytes(Instructions::RET{ })); });
    auto INSTR_RETV = OPCODE.AsTerminal("RETV").Map<InstructionParseValue>([](auto _) { return InstructionParseValue(GetBytes(Instructions::RETV{ })); });
    auto INSTR_POP = OPCODE.AsTerminal("POP").Map<InstructionParseValue>([](auto _) { return InstructionParseValue(GetBytes(Instructions::POP{ })); });
    auto INSTR_EXIT = OPCODE.AsTerminal("EXIT").Map<InstructionParseValue>([](auto _) { return InstructionParseValue(GetBytes(Instructions::SYSCALL{ .code = SysCallCode::EXIT })); });
    auto INSTR_MALLOC = OPCODE.AsTerminal("MALLOC").Map<InstructionParseValue>([](auto _) { return InstructionParseValue(GetBytes(Instructions::SYSCALL{ .code = SysCallCode::MALLOC })); });
    auto INSTR_FREE = OPCODE.AsTerminal("FREE").Map<InstructionParseValue>([](auto _) { return InstructionParseValue(GetBytes(Instructions::SYSCALL{ .code = SysCallCode::FREE })); });
    auto INSTR_PRINTC = OPCODE.AsTerminal("PRINTC").Map<InstructionParseValue>([](auto _) { return InstructionParseValue(GetBytes(Instructions::SYSCALL{ .code = SysCallCode::PRINTC })); });

    auto INSTRUCTION = Parser("INSTRUCTION", INSTR_PUSH | INSTR_EXIT | INSTR_JUMP);

    typedef ProgramParseValue CodeParseValue;
    auto CODE = FoldL<VariantValue<InstructionParseValue, Label>, CodeParseValue>("CODE", INSTRUCTION || LABEL_DEF, CodeParseValue(), [](CodeParseValue& _value, auto _result)
        {
            if (_result.value.template Is<0>()) //INSTRUCTION
            {
                auto instructionResultValue = _result.value.template Get<0>();
                if (instructionResultValue.IsSuccess())
                {
                    Instruction instruction = instructionResultValue.GetSuccess();
                    assert(!instruction.code.empty() && "Parsing INSTRUCTION resulted in a instruction with no code!");

                    auto insertPoint = _value.GetCode().size();
                    _value.AppendCode(instruction.code);

                    switch (instruction.GetOpCode())
                    {
                    case OpCode::JUMP: _value.AddLabelOperand(insertPoint + OP_CODE_SIZE, std::any_cast<Label>(instruction.metadata.at("LABEL"))); break;
                    case OpCode::JUMPZ: _value.AddLabelOperand(insertPoint + OP_CODE_SIZE, std::any_cast<Label>(instruction.metadata.at("LABEL"))); break;
                    case OpCode::JUMPNZ: _value.AddLabelOperand(insertPoint + OP_CODE_SIZE, std::any_cast<Label>(instruction.metadata.at("LABEL"))); break;
                    case OpCode::CALL: _value.AddLabelOperand(insertPoint + OP_CODE_SIZE, std::any_cast<Label>(instruction.metadata.at("LABEL"))); break;
                    case OpCode::PUSH: {
                        auto search = instruction.metadata.find("LABEL");
                        if (search != instruction.metadata.end())
                            _value.AddLabelOperand(insertPoint + OP_CODE_SIZE, std::any_cast<Label>(search->second));
                    } break;
                    default: break;
                    }
                }
                else { throw instructionResultValue.GetParseError(); }
            }
            else { _value.AddLabel(_result.value.template Get<1>()); } //LABEL
        });

    return LPC(lexer, Parser("PROGRAM", CODE), { "WS" });
}

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


        //Collect branching instructions targets
        switch (opcode)
        {
        case OpCode::JUMP: possibleTargets.emplace(Instructions::JUMP::From(ptr)->target); break;
        case OpCode::JUMPNZ: possibleTargets.emplace(Instructions::JUMPNZ::From(ptr)->target); break;
        case OpCode::JUMPZ: possibleTargets.emplace(Instructions::JUMPZ::From(ptr)->target); break;
        case OpCode::CALL: possibleTargets.emplace(Instructions::CALL::From(ptr)->target); break;
        default: continue;
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
    static LPC parser = CreateLPC();
    ProgramParseValue parseResult = parser.Parse(IStreamToString(_stream)).value;

    Program program;
    program.code = std::move(parseResult.GetCode());

    //Replace operands that are labels with the correct position in the program
    for (auto& [pos, token] : parseResult.GetLabelOperands())
    {
        auto labelSearch = parseResult.GetLabels().find(token.value);
        if (labelSearch == parseResult.GetLabels().end())
            throw Error::UNDEFINED_LABEL(token.position, token.value);

        vm_byte* target = program.code.data() + labelSearch->second;
        *(vm_byte**)&program.code[pos] = target;
    }

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