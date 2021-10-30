#include "instructions.h"
#include <iostream>
#include "thread.h"
#include "vm.h"

namespace Instructions
{
    ExecutionFunc ExecutionFuncs[(size_t)OpCode::_COUNT];
    ExecutionFunc SysCallExecutionFuncs[(size_t)SysCallCode::_COUNT];

    void NOOP(Thread *_thread)
    {
        return;
    }

#pragma region Binops
    void IADD(Thread *_thread)
    {
        _thread->PushStack(_thread->PopStack<vm_i64>() + _thread->PopStack<vm_i64>());
    }

    void ISUB(Thread *_thread)
    {
        auto left = _thread->PopStack<vm_i64>(), right = _thread->PopStack<vm_i64>();
        _thread->PushStack(left - right);
    }

    void IMUL(Thread *_thread)
    {
        _thread->PushStack(_thread->PopStack<vm_i64>() * _thread->PopStack<vm_i64>());
    }

    void IDIV(Thread *_thread)
    {
        vm_i64 left = _thread->PopStack<vm_i64>(), right = _thread->PopStack<vm_i64>();
        if (right == 0ll)
            throw VMError::DIV_BY_ZERO();

        _thread->PushStack(left / right);
    }

    void DADD(Thread *_thread)
    {
        _thread->PushStack(_thread->PopStack<vm_f64>() + _thread->PopStack<vm_f64>());
    }

    void DSUB(Thread *_thread)
    {
        auto left = _thread->PopStack<vm_f64>(), right = _thread->PopStack<vm_f64>();
        _thread->PushStack(left - right);
    }

    void DMUL(Thread *_thread)
    {
        _thread->PushStack(_thread->PopStack<vm_f64>() * _thread->PopStack<vm_f64>());
    }

    void DDIV(Thread *_thread)
    {

        vm_f64 left = _thread->PopStack<vm_f64>(), right = _thread->PopStack<vm_f64>();
        if (right == 0.0)
            throw VMError::DIV_BY_ZERO();

        _thread->PushStack(left / right);
    }

    void EQ(Thread *_thread)
    {
        _thread->PushStack(vm_i64(_thread->PopStack<vm_i64>() == _thread->PopStack<vm_i64>()));
    }

    void NEQ(Thread *_thread)
    {
        _thread->PushStack(vm_i64(_thread->PopStack<vm_i64>() != _thread->PopStack<vm_i64>()));
    }

#pragma endregion

#pragma region Branching
    void JUMP(Thread *_thread)
    {
        vm_byte *target = ((Word *)(_thread->instrPtr + OP_CODE_SIZE))->as_ptr;
        _thread->instrPtr = target - GetSize(OpCode::JUMP);
    }

    void JUMPNZ(Thread *_thread)
    {
        if (_thread->PopStack<vm_i64>() == 0ull)
            return;

        vm_byte *target = ((Word *)(_thread->instrPtr + OP_CODE_SIZE))->as_ptr;
        _thread->instrPtr = target - GetSize(OpCode::JUMPNZ);
    }

    void JUMPZ(Thread *_thread)
    {
        if (_thread->PopStack<vm_i64>() != 0ull)
            return;

        vm_byte *target = ((Word *)(_thread->instrPtr + OP_CODE_SIZE))->as_ptr;
        _thread->instrPtr = target - GetSize(OpCode::JUMPZ);
    }

    void CALL(Thread *_thread)
    {
        vm_byte *target = ((Word *)(_thread->instrPtr + OP_CODE_SIZE))->as_ptr;
        vm_ui32 storage = *(vm_ui32 *)(_thread->instrPtr + OP_CODE_SIZE + VM_PTR_SIZE);

        _thread->PushStack(_thread->instrPtr + GetSize(OpCode::CALL)); //Push return value
        _thread->PushFrame();                                          //Push current frame pointer and set the frame pointer for new frame
        _thread->OffsetSP(storage);                                    //Allocate space of stack for function local storage
        _thread->instrPtr = target - GetSize(OpCode::CALL);            //Jump to target
    }

    void RET(Thread *_thread)
    {
        _thread->PopFrame();                                                       //Clear current frame and restore previous frame pointer
        _thread->instrPtr = _thread->PopStack<vm_byte *>() - GetSize(OpCode::RET); //Jump to instruction after most recent call
    }

    void RETV(Thread *_thread)
    {
        Word retValue = _thread->PopStack<Word>(); //Pop off return value
        RET(_thread);                              //Execute return
        _thread->PushStack(retValue);              //Push return value
    }
#pragma endregion

#pragma region Syscalls
    void SYSCALL_EXIT(Thread *_thread)
    {
        _thread->GetVM()->Quit(_thread->PopStack<vm_i64>());
    }

    void SYSCALL_PRINTC(Thread *_thread)
    {
        _thread->GetVM()->GetStdOut() << static_cast<wchar_t>(_thread->PopStack<vm_i64>());
    }

    void SYSCALL_MALLOC(Thread *_thread)
    {
        _thread->PushStack(_thread->GetVM()->GetHeap().Alloc(_thread->PopStack<vm_i64>()));
    }

    void SYSCALL_FREE(Thread *_thread)
    {
        _thread->GetVM()->GetHeap().Free(_thread->PopStack<vm_byte *>());
    }

    void SYSCALL(Thread *_thread)
    {
        vm_byte code = _thread->instrPtr[OP_CODE_SIZE];
        if (code >= (vm_byte)SysCallCode::_COUNT)
            throw VMError::UNKNOWN_SYSCALL_CODE(code);

        SysCallExecutionFuncs[code](_thread);
    }
#pragma endregion

#pragma region Converters
    void I2D(Thread *_thread)
    {
        _thread->PushStack((vm_f64)_thread->PopStack<vm_i64>());
    }

    void D2I(Thread *_thread)
    {
        _thread->PushStack((vm_i64)_thread->PopStack<vm_f64>());
    }
#pragma endregion

#pragma region Loads and Stores
    void PUSH(Thread *_thread)
    {
        Word word = *(Word *)&_thread->instrPtr[OP_CODE_SIZE];
        _thread->PushStack(word);
    }

    void POP(Thread *_thread)
    {
        _thread->PopStack<Word>();
    }

    void SLOAD(Thread *_thread)
    {
        vm_i64 offset = *(vm_i64 *)&_thread->instrPtr[OP_CODE_SIZE];
        _thread->PushStack(_thread->ReadStack<Word>(_thread->GetSP() + offset));
    }

    void SSTORE(Thread *_thread)
    {
        vm_i64 offset = *(vm_i64 *)&_thread->instrPtr[OP_CODE_SIZE];
        auto value = _thread->PopStack<Word>();
        _thread->WriteStack(_thread->GetSP() + offset, value);
    }

    void LLOAD(Thread *_thread)
    {
        vm_ui32 idx = *(vm_ui32 *)&_thread->instrPtr[OP_CODE_SIZE];
        _thread->PushStack(_thread->ReadStack<Word>(_thread->GetFP() + idx * WORD_SIZE));
    }

    void LSTORE(Thread *_thread)
    {
        vm_ui32 idx = *(vm_ui32 *)&_thread->instrPtr[OP_CODE_SIZE];
        _thread->WriteStack(_thread->GetFP() + idx * WORD_SIZE, _thread->PopStack<Word>());
    }

    void PLOAD(Thread *_thread)
    {
        vm_ui32 idx = *(vm_ui32 *)&_thread->instrPtr[OP_CODE_SIZE];
        _thread->PushStack(_thread->ReadStack<Word>(_thread->GetFP() - WORD_SIZE * 2 - (idx + 1) * WORD_SIZE));
    }

    void PSTORE(Thread *_thread)
    {
        vm_ui32 idx = *(vm_ui32 *)&_thread->instrPtr[OP_CODE_SIZE];
        _thread->WriteStack(_thread->GetFP() - WORD_SIZE * 2 - (idx + 1) * WORD_SIZE, _thread->PopStack<Word>());
    }

    void GLOAD(Thread *_thread)
    {
        vm_ui32 idx = *(vm_ui32 *)&_thread->instrPtr[OP_CODE_SIZE];
        _thread->PushStack(_thread->ReadStack<Word>(idx * WORD_SIZE));
    }

    void GSTORE(Thread *_thread)
    {
        vm_ui32 idx = *(vm_ui32 *)&_thread->instrPtr[OP_CODE_SIZE];
        _thread->WriteStack(idx * WORD_SIZE, _thread->PopStack<Word>());
    }
#pragma endregion

    void Init()
    {
        ExecutionFuncs[(size_t)OpCode::NOOP] = &NOOP;
        ExecutionFuncs[(size_t)OpCode::PUSH] = &PUSH;
        ExecutionFuncs[(size_t)OpCode::POP] = &POP;
        ExecutionFuncs[(size_t)OpCode::IADD] = &IADD;
        ExecutionFuncs[(size_t)OpCode::ISUB] = &ISUB;
        ExecutionFuncs[(size_t)OpCode::IMUL] = &IMUL;
        ExecutionFuncs[(size_t)OpCode::IDIV] = &IDIV;
        ExecutionFuncs[(size_t)OpCode::DADD] = &DADD;
        ExecutionFuncs[(size_t)OpCode::DSUB] = &DSUB;
        ExecutionFuncs[(size_t)OpCode::DMUL] = &DMUL;
        ExecutionFuncs[(size_t)OpCode::DDIV] = &DDIV;
        ExecutionFuncs[(size_t)OpCode::JUMP] = &JUMP;
        ExecutionFuncs[(size_t)OpCode::JUMPZ] = &JUMPZ;
        ExecutionFuncs[(size_t)OpCode::JUMPNZ] = &JUMPNZ;
        ExecutionFuncs[(size_t)OpCode::SYSCALL] = &SYSCALL;
        ExecutionFuncs[(size_t)OpCode::EQ] = &EQ;
        ExecutionFuncs[(size_t)OpCode::NEQ] = &NEQ;
        ExecutionFuncs[(size_t)OpCode::SLOAD] = &SLOAD;
        ExecutionFuncs[(size_t)OpCode::SSTORE] = &SSTORE;
        ExecutionFuncs[(size_t)OpCode::LLOAD] = &LLOAD;
        ExecutionFuncs[(size_t)OpCode::LSTORE] = &LSTORE;
        ExecutionFuncs[(size_t)OpCode::GLOAD] = &GLOAD;
        ExecutionFuncs[(size_t)OpCode::GSTORE] = &GSTORE;
        ExecutionFuncs[(size_t)OpCode::PLOAD] = &PLOAD;
        ExecutionFuncs[(size_t)OpCode::PSTORE] = &PSTORE;
        ExecutionFuncs[(size_t)OpCode::I2D] = &I2D;
        ExecutionFuncs[(size_t)OpCode::D2I] = &D2I;
        ExecutionFuncs[(size_t)OpCode::CALL] = &CALL;
        ExecutionFuncs[(size_t)OpCode::RET] = &RET;
        ExecutionFuncs[(size_t)OpCode::RETV] = &RETV;

        SysCallExecutionFuncs[(size_t)SysCallCode::EXIT] = &SYSCALL_EXIT;
        SysCallExecutionFuncs[(size_t)SysCallCode::PRINTC] = &SYSCALL_PRINTC;
        SysCallExecutionFuncs[(size_t)SysCallCode::MALLOC] = &SYSCALL_MALLOC;
        SysCallExecutionFuncs[(size_t)SysCallCode::FREE] = &SYSCALL_FREE;
    }

    std::string ToString(const vm_byte *_instr)
    {
        switch ((OpCode)*_instr)
        {
        case OpCode::NOOP:
            return "NOOP";
        case OpCode::I2D:
            return "I2D";
        case OpCode::D2I:
            return "D2I";
        case OpCode::POP:
            return "POP";
        case OpCode::IADD:
            return "IADD";
        case OpCode::ISUB:
            return "ISUB";
        case OpCode::IMUL:
            return "IMUL";
        case OpCode::IDIV:
            return "IDIV";
        case OpCode::DADD:
            return "DADD";
        case OpCode::DSUB:
            return "DSUB";
        case OpCode::DMUL:
            return "DMUL";
        case OpCode::DDIV:
            return "DDIV";
        case OpCode::EQ:
            return "EQ";
        case OpCode::NEQ:
            return "NEQ";
        case OpCode::RET:
            return "RET";
        case OpCode::RETV:
            return "RETV";
        case OpCode::PUSH:
            return "PUSH " + Hex(*(vm_ui64 *)&_instr[OP_CODE_SIZE]);
        case OpCode::JUMP:
            return "JUMP " + Hex(*(vm_ui64 *)&_instr[OP_CODE_SIZE]);
        case OpCode::JUMPNZ:
            return "JUMPNZ " + Hex(*(vm_ui64 *)&_instr[OP_CODE_SIZE]);
        case OpCode::JUMPZ:
            return "JUMPZ " + Hex(*(vm_ui64 *)&_instr[OP_CODE_SIZE]);
        case OpCode::CALL:
            return "CALL " + Hex(*(vm_ui64 *)&_instr[OP_CODE_SIZE]) + ", " + std::to_string(*(vm_ui32 *)&_instr[OP_CODE_SIZE + VM_UI64_SIZE]);
        case OpCode::SYSCALL:
        {
            switch ((SysCallCode)_instr[1])
            {
            case SysCallCode::EXIT:
                return "SYSCALL EXIT";
            case SysCallCode::PRINTC:
                return "SYSCALL PRINTC";
            case SysCallCode::MALLOC:
                return "SYSCALL MALLOC";
            case SysCallCode::FREE:
                return "SYSCALL FREE";
            default:
                assert(false && "Case not handled");
            }
        }
        break;
        case OpCode::SLOAD:
            return "SLOAD " + std::to_string(*(vm_i64 *)&_instr[1]);
        case OpCode::SSTORE:
            return "SSTORE " + std::to_string(*(vm_i64 *)&_instr[1]);
        case OpCode::LLOAD:
            return "LLOAD " + std::to_string(*(vm_ui32 *)&_instr[1]);
        case OpCode::LSTORE:
            return "LSTORE " + std::to_string(*(vm_ui32 *)&_instr[1]);
        case OpCode::GLOAD:
            return "GLOAD " + std::to_string(*(vm_ui32 *)&_instr[1]);
        case OpCode::GSTORE:
            return "GSTORE " + std::to_string(*(vm_ui32 *)&_instr[1]);
        case OpCode::PLOAD:
            return "PLOAD " + std::to_string(*(vm_ui32 *)&_instr[1]);
        case OpCode::PSTORE:
            return "PSTORE " + std::to_string(*(vm_ui32 *)&_instr[1]);
        default:
            assert(false && "Case not handled");
        }

        return "";
    }
}