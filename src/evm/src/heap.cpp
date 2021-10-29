#include "heap.h"
#include "../build.h"
#include "vm.h"
#include <algorithm>

Block::Block(size_t _size, FreeChunksMap &_freeChunks)
{
    assert(_size > 0 && "Block must be initialized with positive size!");
    storage = Memory(_size);

    Chunk initial(&storage.front(), storage.size(), nullptr, nullptr);
    _freeChunks[_size][&initial] = this;
    chunks.emplace(initial.start, initial);
}

void Block::Defragment()
{
    Chunk *current = &chunks.at(&storage.front());

    while (current)
    {
        if (current->allocated)
        {
            current = current->next;
            continue;
        }

        //Merge with right adjacent unallocated blocks
        while (current->next && !current->next->allocated)
        {
            current->size += current->next->size;
            current->next = current->next->next;
            chunks.erase(current->next->start);

            //Ensure chunk pointers are continuous
            if (current->next)
                current->next->prev = current;
        }
    }
}

bool Block::HasAddress(vm_byte *_addr) { return _addr >= &storage.front() && _addr <= &storage.back(); }

bool Block::IsEmpty() { return chunks.size() == 1 && !chunks.begin()->second.allocated; }

bool Block::IsAllocated(vm_byte *_addr)
{
    auto search = chunks.find(_addr);
    return search != chunks.end() && search->second.allocated;
}

void Block::Alloc(vm_byte *_chunkPtr, vm_ui64 _amt, FreeChunksMap &_freeChunks)
{
    Chunk &chunk = chunks.at(_chunkPtr);
    assert(_amt <= chunk.size && ("Cannot alloc " + std::to_string(_amt) + " bytes for an unallocated chunk of size " + std::to_string(chunk.size)).c_str());

    //Possibly fragment chunk
    if (_amt < chunk.size)
    {
        vm_byte *nextChunkStart = _chunkPtr + _amt;
        Chunk next(nextChunkStart, chunk.size - _amt, &chunk, chunk.next);
        chunks.emplace(next.start, next);
        chunk.next = &next;
        _freeChunks[next.size][&next] = this;
    }

    //Update free chunks list
    auto &freeChunksSizeBucket = _freeChunks.at(chunk.size);
    freeChunksSizeBucket.erase(&chunk);
    if (freeChunksSizeBucket.size() == 0)
        _freeChunks.erase(chunk.next->size);

    //Set chunk
    chunk.allocated = true;
    chunk.size = _amt;
}

void Block::Free(vm_byte *_chunkPtr, FreeChunksMap &_freeChunks)
{
    Chunk &chunk = chunks.at(_chunkPtr);
    chunk.allocated = false;

    //Possibly merge with next chunk
    if (chunk.next && !chunk.next->allocated)
    {
        auto chunkToDeleteStart = chunk.next->start;

        //Update free chunks list
        auto &freeChunksSizeBucket = _freeChunks.at(chunk.next->size);
        freeChunksSizeBucket.erase(chunk.next);
        if (freeChunksSizeBucket.size() == 0)
            _freeChunks.erase(chunk.next->size);

        //Merge chunks
        chunk.size += chunk.next->size;
        chunk.next = chunk.next->next;
        chunks.erase(chunkToDeleteStart);
    }

    //Possibly merge with previous chunk
    if (chunk.prev && !chunk.prev->allocated)
    {
        //Update free chunks list
        auto &freeChunksSizeBucket = _freeChunks.at(chunk.prev->size);
        freeChunksSizeBucket.erase(chunk.prev);
        if (freeChunksSizeBucket.size() == 0)
            _freeChunks.erase(chunk.prev->size);

        //Merge chunks
        chunk.prev->next = chunk.next;
        chunk.prev->size += chunk.size;
        _freeChunks[chunk.prev->size][chunk.prev] = this;
        chunks.erase(_chunkPtr);
    }
    else
    {
        _freeChunks[chunk.size][&chunk] = this;
    }
}

Heap::Heap(size_t _initialSize) : blocks(), freeChunks(), size(0)
{
    blocks.emplace_back(Block(_initialSize, freeChunks));
}

vm_byte *Heap::Alloc(vm_ui64 _amt)
{
    auto search = std::find_if(freeChunks.begin(), freeChunks.end(), [_amt](FreeChunksMap::reference _kvPair)
                               { return _kvPair.first >= _amt; });

    if (search != freeChunks.end()) //There is a block with a big enough chunk
    {
        auto chunkAndBlock = search->second.extract(search->second.begin());
        auto chunkStart = chunkAndBlock.key()->start;

        chunkAndBlock.mapped()->Alloc(chunkStart, _amt, freeChunks);
        return chunkStart;
    }
    else
    {
        size_t newBlockSize = std::max(size, _amt) * 2;
        Block &block = blocks.emplace_back(Block(newBlockSize, freeChunks));
        vm_byte *chunkStart = block.GetStart();

        block.Alloc(chunkStart, _amt, freeChunks);
        return chunkStart;
    }
}

void Heap::Free(vm_byte *_addr)
{
    for (auto &block : blocks)
    {
        if (block.HasAddress(_addr) && block.IsAllocated(_addr))
        {
            block.Free(_addr, freeChunks);
            if (block.IsEmpty())
                return;
        }
    }

    throw VMError::CANNOT_FREE_UNALLOCATED_PTR(_addr);
}

bool Heap::IsAddress(vm_byte *_addr)
{
    for (auto &block : blocks)
    {
        if (block.HasAddress(_addr))
            return true;
    }

    return false;
}

bool Heap::IsAllocated(vm_byte *_addr)
{
    for (auto &block : blocks)
    {
        if (block.HasAddress(_addr) && block.IsAllocated(_addr))
            return true;
    }

    return false;
}

size_t Heap::GetSize()
{
#ifdef BUILD_DEBUG
    size_t expected = 0;
    for (auto &block : blocks)
        expected += block.GetSize();

    assert(size == expected && ("Expected size of heap to be " + std::to_string(expected) + " but found Heap::size = " + std::to_string(size)).c_str());
#endif

    return size;
}