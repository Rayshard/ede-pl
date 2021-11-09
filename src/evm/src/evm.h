#pragma once

#include <cstdint>
#include <assert.h>
#include <sstream>
#include <iomanip>
#include <vector>

typedef uint8_t vm_byte;
typedef int8_t vm_i8;
typedef int16_t vm_i16;
typedef int32_t vm_i32;
typedef int64_t vm_i64;
typedef uint8_t vm_ui8;
typedef uint16_t vm_ui16;
typedef uint32_t vm_ui32;
typedef uint64_t vm_ui64;
typedef float vm_f32;
typedef double vm_f64;
typedef std::vector<vm_byte> Memory;

#define VM_BYTE_SIZE sizeof(vm_byte)
#define VM_I32_SIZE sizeof(vm_i32)
#define VM_I64_SIZE sizeof(vm_i64)
#define VM_UI32_SIZE sizeof(vm_ui32)
#define VM_UI64_SIZE sizeof(vm_ui64)
#define VM_F32_SIZE sizeof(vm_f32)
#define VM_F64_SIZE sizeof(vm_f64)
#define VM_PTR_SIZE sizeof(vm_byte *)
#define WORD_SIZE 8ull

union Word
{
    vm_byte as_byte;
    vm_i8 as_i8;
    vm_i16 as_i16;
    vm_i32 as_i32;
    vm_i64 as_i64;
    vm_ui8 as_ui8;
    vm_ui16 as_ui16;
    vm_ui32 as_ui32;
    vm_ui64 as_ui64;
    vm_f32 as_f32;
    vm_f64 as_f64;
    vm_byte* as_ptr;
    vm_byte bytes[sizeof(vm_byte*)];

    Word() : as_ptr(0) {}
    Word(vm_i8 _i) : as_i8(_i) {}
    Word(vm_ui8 _i) : as_ui8(_i) {}
    Word(vm_i16 _i) : as_i16(_i) {}
    Word(vm_ui16 _i) : as_ui16(_i) {}
    Word(vm_i32 _i) : as_i32(_i) {}
    Word(vm_ui32 _i) : as_ui32(_i) {}
    Word(vm_i64 _i) : as_i64(_i) {}
    Word(vm_ui64 _i) : as_ui64(_i) {}
    Word(vm_f32 _f) : as_f32(_f) {}
    Word(vm_f64 _f) : as_f64(_f) {}
    Word(vm_byte* _p) : as_ptr(_p) {}
    Word(bool _b) : as_ui64(_b) {}

    Word(vm_byte _0, vm_byte _1, vm_byte _2, vm_byte _3, vm_byte _4, vm_byte _5, vm_byte _6, vm_byte _7)
    {
        bytes[0] = _0;
        bytes[1] = _1;
        bytes[2] = _2;
        bytes[3] = _3;
        bytes[4] = _4;
        bytes[5] = _5;
        bytes[6] = _6;
        bytes[7] = _7;
    }

    bool operator==(Word _w) { return as_ui64 == _w.as_ui64; }
    bool operator!=(Word _w) { return as_ui64 != _w.as_ui64; }

    bool AsBool() { return as_ui64 != 0ull; }
};

static_assert(sizeof(Word) == WORD_SIZE, "Word is not the right size!");

template <typename T>
std::string Hex(const T& _value, bool _includePrefix = true)
{
    auto first = (vm_byte*)&_value;
    auto last = first + sizeof(_value) - 1;
    std::ostringstream stream;

    if (_includePrefix)
        stream << "0x";

    stream << std::hex << std::setfill('0') << std::uppercase;

    do
    {
        stream << std::setw(2) << (vm_ui32)*last;
    } while (last-- != first);

    return stream.str();
}

template <typename T>
std::string PtrToStr(T* _ptr)
{
    const void* ptr = static_cast<const void*>(_ptr);
    std::stringstream ss;

    ss << ptr;
    return ss.str();
}