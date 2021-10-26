#pragma once

#include <cstdint>
#include <assert.h>
#include <sstream>
#include <iomanip>

typedef uint8_t byte;

union Word
{
    int64_t as_int;
    uint64_t as_uint;
    double as_double;
    void *as_ptr;
    byte bytes[sizeof(int64_t)];

    Word() : as_int(0) {}
    Word(int64_t _i) : as_int(_i) {}
    Word(uint64_t _ui) : as_uint(_ui) {}
    Word(double _d) : as_double(_d) {}
    Word(void *_p) : as_ptr(_p) {}
};

#define WORD_SIZE sizeof(Word)
static_assert(WORD_SIZE == sizeof(int64_t), "Word is not the right size!");

template <typename T>
std::string Hex(const T &_value)
{
    std::stringstream stream("0x");
    stream << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << _value;
    return stream.str();
}
