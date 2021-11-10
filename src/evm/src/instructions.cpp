#include "instructions.h"
#include <iostream>
#include "thread.h"
#include "vm.h"

namespace Instructions
{
    namespace Error
    {
        std::runtime_error CompilationNotImplemented(const vm_byte* _instr) { return std::runtime_error("Instruction compilation not implemented: " + ToString(_instr)); }
    }

    ExecutionFunc ExecutionFuncs[(size_t)OpCode::_COUNT];
    ExecutionFunc SysCallExecutionFuncs[(size_t)SysCallCode::_COUNT];

    namespace NOOP
    {
        void Execute(Thread* _thread) { return; }
    }

    namespace CONVERT
    {
        DataType GetFrom(const vm_byte* _instr) { return *(DataType*)(_instr + OP_CODE_SIZE); }
        DataType GetTo(const vm_byte* _instr) { return *(DataType*)(_instr + OP_CODE_SIZE + DATA_TYPE_SIZE); }

        void Execute(Thread* _thread)
        {
            DataType from = GetFrom(_thread->instrPtr), to = GetTo(_thread->instrPtr);
            auto value = _thread->PopStack();

            switch (from)
            {
            case DataType::I8: {
                switch (to)
                {
                case DataType::I8: _thread->PushStack(vm_i8(value.as_i8)); break;
                case DataType::UI8: _thread->PushStack(vm_ui8(value.as_i8)); break;
                case DataType::I16: _thread->PushStack(vm_i16(value.as_i8)); break;
                case DataType::UI16: _thread->PushStack(vm_ui16(value.as_i8)); break;
                case DataType::I32: _thread->PushStack(vm_i32(value.as_i8)); break;
                case DataType::UI32: _thread->PushStack(vm_ui32(value.as_i8)); break;
                case DataType::I64: _thread->PushStack(vm_i64(value.as_i8)); break;
                case DataType::UI64: _thread->PushStack(vm_ui64(value.as_i8)); break;
                case DataType::F32: _thread->PushStack(vm_f32(value.as_i8)); break;
                case DataType::F64: _thread->PushStack(vm_f64(value.as_i8)); break;
                default: assert(false && "Case not handled");
                }
            } break;
            case DataType::UI8: {
                switch (to)
                {
                case DataType::I8: _thread->PushStack(vm_i8(value.as_ui8)); break;
                case DataType::UI8: _thread->PushStack(vm_ui8(value.as_ui8)); break;
                case DataType::I16: _thread->PushStack(vm_i16(value.as_ui8)); break;
                case DataType::UI16: _thread->PushStack(vm_ui16(value.as_ui8)); break;
                case DataType::I32: _thread->PushStack(vm_i32(value.as_ui8)); break;
                case DataType::UI32: _thread->PushStack(vm_ui32(value.as_ui8)); break;
                case DataType::I64: _thread->PushStack(vm_i64(value.as_ui8)); break;
                case DataType::UI64: _thread->PushStack(vm_ui64(value.as_ui8)); break;
                case DataType::F32: _thread->PushStack(vm_f32(value.as_ui8)); break;
                case DataType::F64: _thread->PushStack(vm_f64(value.as_ui8)); break;
                default: assert(false && "Case not handled");
                }
            } break;
            case DataType::I16: {
                switch (to)
                {
                case DataType::I8: _thread->PushStack(vm_i8(value.as_i16)); break;
                case DataType::UI8: _thread->PushStack(vm_ui8(value.as_i16)); break;
                case DataType::I16: _thread->PushStack(vm_i16(value.as_i16)); break;
                case DataType::UI16: _thread->PushStack(vm_ui16(value.as_i16)); break;
                case DataType::I32: _thread->PushStack(vm_i32(value.as_i16)); break;
                case DataType::UI32: _thread->PushStack(vm_ui32(value.as_i16)); break;
                case DataType::I64: _thread->PushStack(vm_i64(value.as_i16)); break;
                case DataType::UI64: _thread->PushStack(vm_ui64(value.as_i16)); break;
                case DataType::F32: _thread->PushStack(vm_f32(value.as_i16)); break;
                case DataType::F64: _thread->PushStack(vm_f64(value.as_i16)); break;
                default: assert(false && "Case not handled");
                }
            } break;
            case DataType::UI16: {
                switch (to)
                {
                case DataType::I8: _thread->PushStack(vm_i8(value.as_ui16)); break;
                case DataType::UI8: _thread->PushStack(vm_ui8(value.as_ui16)); break;
                case DataType::I16: _thread->PushStack(vm_i16(value.as_ui16)); break;
                case DataType::UI16: _thread->PushStack(vm_ui16(value.as_ui16)); break;
                case DataType::I32: _thread->PushStack(vm_i32(value.as_ui16)); break;
                case DataType::UI32: _thread->PushStack(vm_ui32(value.as_ui16)); break;
                case DataType::I64: _thread->PushStack(vm_i64(value.as_ui16)); break;
                case DataType::UI64: _thread->PushStack(vm_ui64(value.as_ui16)); break;
                case DataType::F32: _thread->PushStack(vm_f32(value.as_ui16)); break;
                case DataType::F64: _thread->PushStack(vm_f64(value.as_ui16)); break;
                default: assert(false && "Case not handled");
                }
            } break;
            case DataType::I32: {
                switch (to)
                {
                case DataType::I8: _thread->PushStack(vm_i8(value.as_i32)); break;
                case DataType::UI8: _thread->PushStack(vm_ui8(value.as_i32)); break;
                case DataType::I16: _thread->PushStack(vm_i16(value.as_i32)); break;
                case DataType::UI16: _thread->PushStack(vm_ui16(value.as_i32)); break;
                case DataType::I32: _thread->PushStack(vm_i32(value.as_i32)); break;
                case DataType::UI32: _thread->PushStack(vm_ui32(value.as_i32)); break;
                case DataType::I64: _thread->PushStack(vm_i64(value.as_i32)); break;
                case DataType::UI64: _thread->PushStack(vm_ui64(value.as_i32)); break;
                case DataType::F32: _thread->PushStack(vm_f32(value.as_i32)); break;
                case DataType::F64: _thread->PushStack(vm_f64(value.as_i32)); break;
                default: assert(false && "Case not handled");
                }
            } break;
            case DataType::UI32: {
                switch (to)
                {
                case DataType::I8: _thread->PushStack(vm_i8(value.as_ui32)); break;
                case DataType::UI8: _thread->PushStack(vm_ui8(value.as_ui32)); break;
                case DataType::I16: _thread->PushStack(vm_i16(value.as_ui32)); break;
                case DataType::UI16: _thread->PushStack(vm_ui16(value.as_ui32)); break;
                case DataType::I32: _thread->PushStack(vm_i32(value.as_ui32)); break;
                case DataType::UI32: _thread->PushStack(vm_ui32(value.as_ui32)); break;
                case DataType::I64: _thread->PushStack(vm_i64(value.as_ui32)); break;
                case DataType::UI64: _thread->PushStack(vm_ui64(value.as_ui32)); break;
                case DataType::F32: _thread->PushStack(vm_f32(value.as_ui32)); break;
                case DataType::F64: _thread->PushStack(vm_f64(value.as_ui32)); break;
                default: assert(false && "Case not handled");
                }
            } break;
            case DataType::I64: {
                switch (to)
                {
                case DataType::I8: _thread->PushStack(vm_i8(value.as_i64)); break;
                case DataType::UI8: _thread->PushStack(vm_ui8(value.as_i64)); break;
                case DataType::I16: _thread->PushStack(vm_i16(value.as_i64)); break;
                case DataType::UI16: _thread->PushStack(vm_ui16(value.as_i64)); break;
                case DataType::I32: _thread->PushStack(vm_i32(value.as_i64)); break;
                case DataType::UI32: _thread->PushStack(vm_ui32(value.as_i64)); break;
                case DataType::I64: _thread->PushStack(vm_i64(value.as_i64)); break;
                case DataType::UI64: _thread->PushStack(vm_ui64(value.as_i64)); break;
                case DataType::F32: _thread->PushStack(vm_f32(value.as_i64)); break;
                case DataType::F64: _thread->PushStack(vm_f64(value.as_i64)); break;
                default: assert(false && "Case not handled");
                }
            } break;
            case DataType::UI64: {
                switch (to)
                {
                case DataType::I8: _thread->PushStack(vm_i8(value.as_ui64)); break;
                case DataType::UI8: _thread->PushStack(vm_ui8(value.as_ui64)); break;
                case DataType::I16: _thread->PushStack(vm_i16(value.as_ui64)); break;
                case DataType::UI16: _thread->PushStack(vm_ui16(value.as_ui64)); break;
                case DataType::I32: _thread->PushStack(vm_i32(value.as_ui64)); break;
                case DataType::UI32: _thread->PushStack(vm_ui32(value.as_ui64)); break;
                case DataType::I64: _thread->PushStack(vm_i64(value.as_ui64)); break;
                case DataType::UI64: _thread->PushStack(vm_ui64(value.as_ui64)); break;
                case DataType::F32: _thread->PushStack(vm_f32(value.as_ui64)); break;
                case DataType::F64: _thread->PushStack(vm_f64(value.as_ui64)); break;
                default: assert(false && "Case not handled");
                }
            } break;
            case DataType::F32: {
                switch (to)
                {
                case DataType::I8: _thread->PushStack(vm_i8(value.as_f32)); break;
                case DataType::UI8: _thread->PushStack(vm_ui8(value.as_f32)); break;
                case DataType::I16: _thread->PushStack(vm_i16(value.as_f32)); break;
                case DataType::UI16: _thread->PushStack(vm_ui16(value.as_f32)); break;
                case DataType::I32: _thread->PushStack(vm_i32(value.as_f32)); break;
                case DataType::UI32: _thread->PushStack(vm_ui32(value.as_f32)); break;
                case DataType::I64: _thread->PushStack(vm_i64(value.as_f32)); break;
                case DataType::UI64: _thread->PushStack(vm_ui64(value.as_f32)); break;
                case DataType::F32: _thread->PushStack(vm_f32(value.as_f32)); break;
                case DataType::F64: _thread->PushStack(vm_f64(value.as_f32)); break;
                default: assert(false && "Case not handled");
                }
            } break;
            case DataType::F64: {
                switch (to)
                {
                case DataType::I8: _thread->PushStack(vm_i8(value.as_f64)); break;
                case DataType::UI8: _thread->PushStack(vm_ui8(value.as_f64)); break;
                case DataType::I16: _thread->PushStack(vm_i16(value.as_f64)); break;
                case DataType::UI16: _thread->PushStack(vm_ui16(value.as_f64)); break;
                case DataType::I32: _thread->PushStack(vm_i32(value.as_f64)); break;
                case DataType::UI32: _thread->PushStack(vm_ui32(value.as_f64)); break;
                case DataType::I64: _thread->PushStack(vm_i64(value.as_f64)); break;
                case DataType::UI64: _thread->PushStack(vm_ui64(value.as_f64)); break;
                case DataType::F32: _thread->PushStack(vm_f32(value.as_f64)); break;
                case DataType::F64: _thread->PushStack(vm_f64(value.as_f64)); break;
                default: assert(false && "Case not handled");
                }
            } break;
            default: assert(false && "Case not handled");
            }
        }
    }

    namespace SYSCALL
    {
        SysCallCode GetCode(const vm_byte* _instr) { return *(SysCallCode*)(_instr + OP_CODE_SIZE); }

        void ExecuteEXIT(Thread* _thread) { _thread->GetVM()->Quit(_thread->PopStack().as_i64); }

        void ExecutePRINTC(Thread* _thread) { _thread->GetVM()->GetStdOut() << _thread->PopStack().as_byte; }

        void ExecuteMALLOC(Thread* _thread) { _thread->PushStack(_thread->GetVM()->GetHeap().Alloc(_thread->PopStack().as_i64)); }

        void ExecuteFREE(Thread* _thread) { _thread->GetVM()->GetHeap().Free(_thread->PopStack().as_ptr); }

        void Execute(Thread* _thread)
        {
            vm_byte code = (vm_byte)GetCode(_thread->instrPtr);
            if (code >= (vm_byte)SysCallCode::_COUNT)
                throw VMError::UNKNOWN_SYSCALL_CODE(code);

            SysCallExecutionFuncs[code](_thread);
        }
    }

    namespace OPERATION
    {
        DataType GetDataType(const vm_byte* _instr) { return *(DataType*)(_instr + OP_CODE_SIZE); }

        void ExecuteADD(Thread* _thread)
        {
            auto left = _thread->PopStack(), right = _thread->PopStack();

            switch (GetDataType(_thread->instrPtr))
            {
            case DataType::I8: _thread->PushStack(Word(vm_i8(left.as_i8 + right.as_i8))); break;
            case DataType::UI8: _thread->PushStack(Word(vm_ui8(left.as_ui8 + right.as_ui8))); break;
            case DataType::I16: _thread->PushStack(Word(vm_i16(left.as_i16 + right.as_i16))); break;
            case DataType::UI16: _thread->PushStack(Word(vm_ui16(left.as_ui16 + right.as_ui16))); break;
            case DataType::I32: _thread->PushStack(Word(left.as_i32 + right.as_i32)); break;
            case DataType::UI32: _thread->PushStack(Word(left.as_ui32 + right.as_ui32)); break;
            case DataType::I64: _thread->PushStack(Word(left.as_i64 + right.as_i64)); break;
            case DataType::UI64: _thread->PushStack(Word(left.as_ui64 + right.as_ui64)); break;
            case DataType::F32: _thread->PushStack(Word(left.as_f32 + right.as_f32)); break;
            case DataType::F64: _thread->PushStack(Word(left.as_f64 + right.as_f64)); break;
            default: assert(false && "Case not handled");
            }
        }

        void ExecuteSUB(Thread* _thread)
        {
            auto left = _thread->PopStack(), right = _thread->PopStack();

            switch (GetDataType(_thread->instrPtr))
            {
            case DataType::I8: _thread->PushStack(Word(vm_i8(left.as_i8 - right.as_i8))); break;
            case DataType::UI8: _thread->PushStack(Word(vm_ui8(left.as_ui8 - right.as_ui8))); break;
            case DataType::I16: _thread->PushStack(Word(vm_i16(left.as_i16 - right.as_i16))); break;
            case DataType::UI16: _thread->PushStack(Word(vm_ui16(left.as_ui16 - right.as_ui16))); break;
            case DataType::I32: _thread->PushStack(Word(left.as_i32 - right.as_i32)); break;
            case DataType::UI32: _thread->PushStack(Word(left.as_ui32 - right.as_ui32)); break;
            case DataType::I64: _thread->PushStack(Word(left.as_i64 - right.as_i64)); break;
            case DataType::UI64: _thread->PushStack(Word(left.as_ui64 - right.as_ui64)); break;
            case DataType::F32: _thread->PushStack(Word(left.as_f32 - right.as_f32)); break;
            case DataType::F64: _thread->PushStack(Word(left.as_f64 - right.as_f64)); break;
            default: assert(false && "Case not handled");
            }
        }

        void ExecuteMUL(Thread* _thread)
        {
            auto left = _thread->PopStack(), right = _thread->PopStack();

            switch (GetDataType(_thread->instrPtr))
            {
            case DataType::I8: _thread->PushStack(Word(vm_i8(left.as_i8 * right.as_i8))); break;
            case DataType::UI8: _thread->PushStack(Word(vm_ui8(left.as_ui8 * right.as_ui8))); break;
            case DataType::I16: _thread->PushStack(Word(vm_i16(left.as_i16 * right.as_i16))); break;
            case DataType::UI16: _thread->PushStack(Word(vm_ui16(left.as_ui16 * right.as_ui16))); break;
            case DataType::I32: _thread->PushStack(Word(left.as_i32 * right.as_i32)); break;
            case DataType::UI32: _thread->PushStack(Word(left.as_ui32 * right.as_ui32)); break;
            case DataType::I64: _thread->PushStack(Word(left.as_i64 * right.as_i64)); break;
            case DataType::UI64: _thread->PushStack(Word(left.as_ui64 * right.as_ui64)); break;
            case DataType::F32: _thread->PushStack(Word(left.as_f32 * right.as_f32)); break;
            case DataType::F64: _thread->PushStack(Word(left.as_f64 * right.as_f64)); break;
            default: assert(false && "Case not handled");
            }
        }

        void ExecuteDIV(Thread* _thread)
        {
            auto left = _thread->PopStack(), right = _thread->PopStack();

            switch (GetDataType(_thread->instrPtr))
            {
            case DataType::I8: {
                if (right.as_i8 == vm_i8(0))
                    throw VMError::DIV_BY_ZERO();

                _thread->PushStack(Word(vm_i8(left.as_i8 / right.as_i8)));
            } break;
            case DataType::UI8: {
                if (right.as_ui8 == vm_ui8(0))
                    throw VMError::DIV_BY_ZERO();

                _thread->PushStack(Word(vm_ui8(left.as_ui8 / right.as_ui8)));
            } break;
            case DataType::I16: {
                if (right.as_i16 == vm_i16(0))
                    throw VMError::DIV_BY_ZERO();

                _thread->PushStack(Word(vm_i16(left.as_i16 / right.as_i16)));
            } break;
            case DataType::UI16: {
                if (right.as_ui16 == vm_ui16(0))
                    throw VMError::DIV_BY_ZERO();

                _thread->PushStack(Word(vm_ui16(left.as_ui16 / right.as_ui16)));
            } break;
            case DataType::I32: {
                if (right.as_i32 == vm_i32(0))
                    throw VMError::DIV_BY_ZERO();

                _thread->PushStack(Word(left.as_i32 / right.as_i32));
            } break;
            case DataType::UI32: {
                if (right.as_ui32 == vm_ui32(0))
                    throw VMError::DIV_BY_ZERO();

                _thread->PushStack(Word(left.as_ui32 / right.as_ui32));
            } break;
            case DataType::I64: {
                if (right.as_i64 == vm_i64(0))
                    throw VMError::DIV_BY_ZERO();

                _thread->PushStack(Word(left.as_i64 / right.as_i64));
            } break;
            case DataType::UI64: {
                if (right.as_ui64 == vm_ui64(0))
                    throw VMError::DIV_BY_ZERO();

                _thread->PushStack(Word(left.as_ui64 / right.as_ui64));
            } break;
            case DataType::F32: {
                if (right.as_f32 == vm_f32(0))
                    throw VMError::DIV_BY_ZERO();

                _thread->PushStack(Word(left.as_f32 / right.as_f32));
            } break;
            case DataType::F64: {
                if (right.as_f64 == vm_f64(0))
                    throw VMError::DIV_BY_ZERO();

                _thread->PushStack(Word(left.as_f64 / right.as_f64));
            } break;
            default: assert(false && "Case not handled");
            }
        }

        void ExecuteEQ(Thread* _thread)
        {
            auto left = _thread->PopStack(), right = _thread->PopStack();

            switch (GetDataType(_thread->instrPtr))
            {
            case DataType::I8: _thread->PushStack(Word(left.as_i8 == right.as_i8)); break;
            case DataType::UI8: _thread->PushStack(Word(left.as_ui8 == right.as_ui8)); break;
            case DataType::I16: _thread->PushStack(Word(left.as_i16 == right.as_i16)); break;
            case DataType::UI16: _thread->PushStack(Word(left.as_ui16 == right.as_ui16)); break;
            case DataType::I32: _thread->PushStack(Word(left.as_i32 == right.as_i32)); break;
            case DataType::UI32: _thread->PushStack(Word(left.as_ui32 == right.as_ui32)); break;
            case DataType::I64: _thread->PushStack(Word(left.as_i64 == right.as_i64)); break;
            case DataType::UI64: _thread->PushStack(Word(left.as_ui64 == right.as_ui64)); break;
            case DataType::F32: _thread->PushStack(Word(left.as_f32 == right.as_f32)); break;
            case DataType::F64: _thread->PushStack(Word(left.as_f64 == right.as_f64)); break;
            default: assert(false && "Case not handled");
            }
        }

        void ExecuteNEQ(Thread* _thread)
        {
            auto left = _thread->PopStack(), right = _thread->PopStack();

            switch (GetDataType(_thread->instrPtr))
            {
            case DataType::I8: _thread->PushStack(Word(left.as_i8 != right.as_i8)); break;
            case DataType::UI8: _thread->PushStack(Word(left.as_ui8 != right.as_ui8)); break;
            case DataType::I16: _thread->PushStack(Word(left.as_i16 != right.as_i16)); break;
            case DataType::UI16: _thread->PushStack(Word(left.as_ui16 != right.as_ui16)); break;
            case DataType::I32: _thread->PushStack(Word(left.as_i32 != right.as_i32)); break;
            case DataType::UI32: _thread->PushStack(Word(left.as_ui32 != right.as_ui32)); break;
            case DataType::I64: _thread->PushStack(Word(left.as_i64 != right.as_i64)); break;
            case DataType::UI64: _thread->PushStack(Word(left.as_ui64 != right.as_ui64)); break;
            case DataType::F32: _thread->PushStack(Word(left.as_f32 != right.as_f32)); break;
            case DataType::F64: _thread->PushStack(Word(left.as_f64 != right.as_f64)); break;
            default: assert(false && "Case not handled");
            }
        }
    }

    namespace JUMP
    {
        vm_byte* GetTarget(const vm_byte* _instr) { return *(vm_byte**)(_instr + OP_CODE_SIZE); }
        void SetTarget(vm_byte* _instr, vm_byte* _target) { *(vm_byte**)(_instr + OP_CODE_SIZE) = _target; }

        void ExecuteJUMP(Thread* _thread) { _thread->instrPtr = GetTarget(_thread->instrPtr) - GetSize(OpCode::JUMP); }

        void ExecuteJUMPNZ(Thread* _thread)
        {
            if (!_thread->PopStack().AsBool())
                return;

            _thread->instrPtr = GetTarget(_thread->instrPtr) - GetSize(OpCode::JUMPNZ);
        }

        void ExecuteJUMPZ(Thread* _thread)
        {
            if (_thread->PopStack().AsBool())
                return;

            _thread->instrPtr = GetTarget(_thread->instrPtr) - GetSize(OpCode::JUMPZ);
        }
    }

    namespace CALL
    {
        vm_byte* GetTarget(const vm_byte* _instr) { return *(vm_byte**)(_instr + OP_CODE_SIZE); }
        void SetTarget(vm_byte* _instr, vm_byte* _target) { *(vm_byte**)(_instr + OP_CODE_SIZE) = _target; }

        vm_ui32 GetStorage(const vm_byte* _instr) { return *(vm_ui32*)(_instr + OP_CODE_SIZE + VM_PTR_SIZE); }

        void Execute(Thread* _thread)
        {
            _thread->PushStack(_thread->instrPtr + GetSize(OpCode::CALL)); //Push return value
            _thread->PushFrame();                                          //Push current frame pointer and set the frame pointer for new frame
            _thread->OffsetSP(GetStorage(_thread->instrPtr));                                    //Allocate space of stack for function local storage
            _thread->instrPtr = GetTarget(_thread->instrPtr) - GetSize(OpCode::CALL);            //Jump to target
        }
    }

    namespace RET
    {
        void ExecuteRET(Thread* _thread)
        {
            _thread->PopFrame();                                                   //Clear current frame and restore previous frame pointer
            _thread->instrPtr = _thread->PopStack().as_ptr - GetSize(OpCode::RET); //Jump to instruction after most recent call
        }

        void ExecuteRETV(Thread* _thread)
        {
            Word retValue = _thread->PopStack(); //Pop off return value
            ExecuteRET(_thread);                 //Execute return
            _thread->PushStack(retValue);        //Push return value
        }
    }

    namespace PUSH
    {
        Word GetValue(const vm_byte* _instr) { return *(Word*)(_instr + OP_CODE_SIZE); }
        void Execute(Thread* _thread) { _thread->PushStack(PUSH::GetValue(_thread->instrPtr)); }
    }

    namespace POP
    {
        void Execute(Thread* _thread) { _thread->PopStack(); }
    }

    namespace INDEXED_LS
    {
        vm_ui32 GetIndex(const vm_byte* _instr) { return *(vm_ui32*)&_instr[OP_CODE_SIZE]; }

        void ExecuteLLOAD(Thread* _thread) { _thread->PushStack(_thread->ReadStack<Word>(_thread->GetFP() + GetIndex(_thread->instrPtr) * WORD_SIZE)); }
        void ExecuteLSTORE(Thread* _thread) { _thread->WriteStack(_thread->GetFP() + GetIndex(_thread->instrPtr) * WORD_SIZE, _thread->PopStack()); }
        void ExecutePLOAD(Thread* _thread) { _thread->PushStack(_thread->ReadStack<Word>(_thread->GetFP() - WORD_SIZE * 2 - (GetIndex(_thread->instrPtr) + 1) * WORD_SIZE)); }
        void ExecutePSTORE(Thread* _thread) { _thread->WriteStack(_thread->GetFP() - WORD_SIZE * 2 - (GetIndex(_thread->instrPtr) + 1) * WORD_SIZE, _thread->PopStack()); }

        void ExecuteGLOAD(Thread* _thread)
        {
            vm_byte* addr = _thread->ReadStack<vm_byte*>(0) + GetIndex(_thread->instrPtr) * WORD_SIZE;

            if (!_thread->GetVM()->GetHeap().IsAddressRange(addr, addr + WORD_SIZE - 1))
                throw VMError::INVALID_MEM_ACCESS(addr, addr + WORD_SIZE - 1);

            _thread->PushStack(*(Word*)addr);
        }

        void ExecuteGSTORE(Thread* _thread)
        {
            vm_byte* addr = _thread->ReadStack<vm_byte*>(0) + GetIndex(_thread->instrPtr) * WORD_SIZE;
            Word value = _thread->PopStack();

            if (!_thread->GetVM()->GetHeap().IsAddressRange(addr, addr + WORD_SIZE - 1))
                throw VMError::INVALID_MEM_ACCESS(addr, addr + WORD_SIZE - 1);

            std::copy((vm_byte*)&value, (vm_byte*)&value + sizeof(value), addr);
        }
    }

    namespace OFFSETED_LS
    {
        vm_i64 GetOffset(const vm_byte* _instr) { return *(vm_i64*)&_instr[OP_CODE_SIZE]; }

        void ExecuteSLOAD(Thread* _thread) { _thread->PushStack(_thread->ReadStack<Word>(_thread->GetSP() + GetOffset(_thread->instrPtr))); }

        void ExecuteSSTORE(Thread* _thread)
        {
            auto value = _thread->PopStack();
            _thread->WriteStack(_thread->GetSP() + GetOffset(_thread->instrPtr), value);
        }

        void ExecuteMLOAD(Thread* _thread)
        {
            vm_byte* addr = _thread->PopStack().as_ptr + GetOffset(_thread->instrPtr);

            if (!_thread->GetVM()->GetHeap().IsAddressRange(addr, addr + WORD_SIZE - 1))
                throw VMError::INVALID_MEM_ACCESS(addr, addr + WORD_SIZE - 1);

            _thread->PushStack(*(Word*)addr);
        }

        void ExecuteMSTORE(Thread* _thread)
        {
            vm_byte* addr = _thread->PopStack().as_ptr + GetOffset(_thread->instrPtr);
            Word value = _thread->PopStack();

            if (!_thread->GetVM()->GetHeap().IsAddressRange(addr, addr + WORD_SIZE - 1))
                throw VMError::INVALID_MEM_ACCESS(addr, addr + WORD_SIZE - 1);

            std::copy((vm_byte*)&value, (vm_byte*)&value + sizeof(value), addr);
        }
    }

    void Init()
    {
        ExecutionFuncs[(size_t)OpCode::NOOP] = &NOOP::Execute;
        ExecutionFuncs[(size_t)OpCode::PUSH] = &PUSH::Execute;
        ExecutionFuncs[(size_t)OpCode::POP] = &POP::Execute;
        ExecutionFuncs[(size_t)OpCode::ADD] = &OPERATION::ExecuteADD;
        ExecutionFuncs[(size_t)OpCode::SUB] = &OPERATION::ExecuteSUB;
        ExecutionFuncs[(size_t)OpCode::MUL] = &OPERATION::ExecuteMUL;
        ExecutionFuncs[(size_t)OpCode::DIV] = &OPERATION::ExecuteDIV;
        ExecutionFuncs[(size_t)OpCode::EQ] = &OPERATION::ExecuteEQ;
        ExecutionFuncs[(size_t)OpCode::NEQ] = &OPERATION::ExecuteNEQ;
        ExecutionFuncs[(size_t)OpCode::JUMP] = &JUMP::ExecuteJUMP;
        ExecutionFuncs[(size_t)OpCode::JUMPZ] = &JUMP::ExecuteJUMPZ;
        ExecutionFuncs[(size_t)OpCode::JUMPNZ] = &JUMP::ExecuteJUMPNZ;
        ExecutionFuncs[(size_t)OpCode::SYSCALL] = &SYSCALL::Execute;
        ExecutionFuncs[(size_t)OpCode::SLOAD] = &OFFSETED_LS::ExecuteSLOAD;
        ExecutionFuncs[(size_t)OpCode::SSTORE] = &OFFSETED_LS::ExecuteSSTORE;
        ExecutionFuncs[(size_t)OpCode::LLOAD] = &INDEXED_LS::ExecuteLLOAD;
        ExecutionFuncs[(size_t)OpCode::LSTORE] = &INDEXED_LS::ExecuteLSTORE;
        ExecutionFuncs[(size_t)OpCode::GLOAD] = &INDEXED_LS::ExecuteGLOAD;
        ExecutionFuncs[(size_t)OpCode::GSTORE] = &INDEXED_LS::ExecuteGSTORE;
        ExecutionFuncs[(size_t)OpCode::PLOAD] = &INDEXED_LS::ExecutePLOAD;
        ExecutionFuncs[(size_t)OpCode::PSTORE] = &INDEXED_LS::ExecutePSTORE;
        ExecutionFuncs[(size_t)OpCode::MLOAD] = &OFFSETED_LS::ExecuteMLOAD;
        ExecutionFuncs[(size_t)OpCode::MSTORE] = &OFFSETED_LS::ExecuteMSTORE;
        ExecutionFuncs[(size_t)OpCode::CONVERT] = &CONVERT::Execute;
        ExecutionFuncs[(size_t)OpCode::CALL] = &CALL::Execute;
        ExecutionFuncs[(size_t)OpCode::RET] = &RET::ExecuteRET;
        ExecutionFuncs[(size_t)OpCode::RETV] = &RET::ExecuteRETV;

        SysCallExecutionFuncs[(size_t)SysCallCode::EXIT] = &SYSCALL::ExecuteEXIT;
        SysCallExecutionFuncs[(size_t)SysCallCode::PRINTC] = &SYSCALL::ExecutePRINTC;
        SysCallExecutionFuncs[(size_t)SysCallCode::MALLOC] = &SYSCALL::ExecuteMALLOC;
        SysCallExecutionFuncs[(size_t)SysCallCode::FREE] = &SYSCALL::ExecuteFREE;
    }

    std::string ToString(DataType _dt)
    {
        switch (_dt)
        {
        case DataType::I8: return "I8";
        case DataType::UI8: return "UI8";
        case DataType::I16: return "I16";
        case DataType::UI16: return "UI16";
        case DataType::I32: return "I32";
        case DataType::UI32: return "UI32";
        case DataType::I64: return "I64";
        case DataType::UI64: return "UI64";
        case DataType::F32: return "F32";
        case DataType::F64: return "F64";
        default: assert(false && "Case not handled");
        }

        return "";
    }

    std::string ToString(const vm_byte* _instr)
    {
        switch ((OpCode)*_instr)
        {
        case OpCode::NOOP: return "NOOP";
        case OpCode::POP: return "POP";
        case OpCode::RET: return "RET";
        case OpCode::RETV: return "RETV";
        case OpCode::CONVERT: return "CONVERT " + ToString(CONVERT::GetFrom(_instr)) + " " + ToString(CONVERT::GetTo(_instr));
        case OpCode::ADD: return "ADD " + ToString(OPERATION::GetDataType(_instr));
        case OpCode::SUB: return "SUB " + ToString(OPERATION::GetDataType(_instr));
        case OpCode::MUL: return "MUL " + ToString(OPERATION::GetDataType(_instr));
        case OpCode::DIV: return "DIV " + ToString(OPERATION::GetDataType(_instr));
        case OpCode::EQ: return "EQ " + ToString(OPERATION::GetDataType(_instr));
        case OpCode::NEQ: return "NEQ " + ToString(OPERATION::GetDataType(_instr));
        case OpCode::PUSH: return "PUSH " + Hex(PUSH::GetValue(_instr).as_ui64);
        case OpCode::JUMP: return "JUMP " + Hex(JUMP::GetTarget(_instr));
        case OpCode::JUMPNZ: return "JUMPNZ " + Hex(JUMP::GetTarget(_instr));
        case OpCode::JUMPZ: return "JUMPZ " + Hex(JUMP::GetTarget(_instr));
        case OpCode::CALL: return "CALL " + Hex(CALL::GetTarget(_instr)) + " " + std::to_string(CALL::GetStorage(_instr));
        case OpCode::SYSCALL:
        {
            switch (SYSCALL::GetCode(_instr))
            {
            case SysCallCode::EXIT: return "SYSCALL EXIT";
            case SysCallCode::PRINTC: return "SYSCALL PRINTC";
            case SysCallCode::MALLOC: return "SYSCALL MALLOC";
            case SysCallCode::FREE: return "SYSCALL FREE";
            default: assert(false && "Case not handled");
            }
        } break;
        case OpCode::SLOAD: return "SLOAD " + std::to_string(OFFSETED_LS::GetOffset(_instr));
        case OpCode::SSTORE: return "SSTORE " + std::to_string(OFFSETED_LS::GetOffset(_instr));
        case OpCode::LLOAD: return "LLOAD " + std::to_string(INDEXED_LS::GetIndex(_instr));
        case OpCode::LSTORE: return "LSTORE " + std::to_string(INDEXED_LS::GetIndex(_instr));
        case OpCode::GLOAD: return "GLOAD " + std::to_string(INDEXED_LS::GetIndex(_instr));
        case OpCode::GSTORE: return "GSTORE " + std::to_string(INDEXED_LS::GetIndex(_instr));
        case OpCode::PLOAD: return "PLOAD " + std::to_string(INDEXED_LS::GetIndex(_instr));
        case OpCode::PSTORE: return "PSTORE " + std::to_string(INDEXED_LS::GetIndex(_instr));
        case OpCode::MLOAD: return "MLOAD " + std::to_string(OFFSETED_LS::GetOffset(_instr));
        case OpCode::MSTORE: return "MSTORE " + std::to_string(OFFSETED_LS::GetOffset(_instr));
        default: assert(false && "Case not handled");
        }

        return "";
    }

    void ToNASM(const vm_byte* _instr, std::ostream& _stream, const std::string& _indent)
    {
        switch ((OpCode)*_instr)
        {
        case OpCode::PUSH:
        {
            _stream << _indent << "mov rax, " << Hex(Instructions::PUSH::GetValue(_instr)) << "\n";
            _stream << _indent << "push rax";
        } break;
        case OpCode::NOOP: {_stream << _indent << "mov rax, rax";} break;
        case OpCode::SYSCALL:
        {
            switch (SYSCALL::GetCode(_instr))
            {
            case SysCallCode::EXIT:
            {
                _stream << _indent << "pop rdi\n";
                _stream << _indent << "mov rax, 0x02000001\n";
                _stream << _indent << "syscall";
            } break;
            default: throw Error::CompilationNotImplemented(_instr);
            }
        } break;
        case OpCode::POP: { _stream << _indent << "pop rax"; } break;
        default:
        throw Error::CompilationNotImplemented(_instr);
        }
    }
}