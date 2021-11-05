#pragma once
#include "evm.h"
#include <unordered_map>
#include <iostream>
#include <optional>
#include <thread>

#define MIN_HEAP_BLOCK_SIZE 1024ull

class VM;
struct Chunk;
class Block;

class FreeChunksList
{
public:
    typedef std::unordered_map<vm_ui64, std::unordered_map<Chunk *, Block *>> type;

    void Insert(Chunk *_chunk, Block *_block);
    void Delete(Chunk *_chunk);
    bool Contains(Chunk *_chunk, Block *_block) const;
    std::optional<std::pair<Chunk *, Block *>> Find(vm_ui64 _minSize);

    const type &GetList() { return map; }

    void AssertHeuristics();
    void Print();

private:
    type map;
};

class Chunk
{
    friend Block;

    vm_byte *start;
    vm_ui64 size;
    Chunk *prev, *next;
    bool allocated;

    Chunk(vm_byte *_start, vm_ui64 _size, Chunk *_prev, Chunk *_next);

public:
    void AssertHeuristics();
    void Print(bool _showData);

    vm_byte *GetStart() { return start; }
    vm_ui64 GetSize() { return size; }
    bool IsAllocated() { return allocated; }
};

class Block
{
    Memory storage;
    std::unordered_map<vm_byte *, Chunk *> chunks;

public:
    Block(size_t _size, FreeChunksList &_freeChunks);
    Block(Block &&_b) noexcept;
    ~Block();

    void Alloc(vm_byte *_chunkPtr, vm_ui64 _amt, FreeChunksList &_freeChunks);
    void Free(vm_byte *_chunkPtr, FreeChunksList &_freeChunks);

    bool IsAllocated(vm_byte *_addr);
    bool HasAddress(vm_byte *_addr);
    bool IsEmpty();

    void AssertHeuristics(const FreeChunksList &_freeChunks);
    void Print();

    size_t GetSize() { return storage.size(); }
    vm_byte *GetStart() { return &storage.front(); }
    Chunk *GetFirstChunk() { return chunks.at(&storage.front()); }

    Block &operator=(Block &&_b) noexcept
    {
        if (this == &_b)
            return *this;

        storage = std::move(_b.storage);
        chunks = std::move(_b.chunks);
        return *this;
    }
};

class Heap
{
    VM *vm;

    std::vector<Block *> blocks;
    FreeChunksList freeChunks;
    size_t size;

    std::thread gcThread;
    bool gcRunning;

public:
    Heap(VM *_vm);
    ~Heap();

    vm_byte *Alloc(vm_ui64 _amt);
    void Free(vm_byte *_addr);
    bool IsAddress(vm_byte *_addr);
    bool IsAddressRange(vm_byte *_start, vm_byte *_end);
    bool IsAllocated(vm_byte *_addr);

    void StartGC();
    void StopGC();

    void AssertHeuristics();
    void Print();

    size_t GetSize() { return size; }

private:
    void RunGC();
};