#include "heap.h"
#include "../build.h"
#include "vm.h"
#include <algorithm>
#include <unordered_set>

void Chunk::AssertHeuristics()
{
    assert((next && next->start == start + size) && "Chunk's next chunk starts at a different place than after this chunk's end!");
}

Block::Block(size_t _size, FreeChunksMap &_freeChunks)
{
    assert(_size > 0 && "Block must be initialized with positive size!");
    storage = Memory(_size);

    Chunk initial(&storage.front(), storage.size(), nullptr, nullptr);
    _freeChunks[_size][&initial] = this;
    chunks.emplace(initial.start, initial);
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

#ifdef BUILD_DEBUG_HEAP
    AssertHeuristics(_freeChunks);
#endif
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

#ifdef BUILD_DEBUG_HEAP
    AssertHeuristics(_freeChunks);
#endif
}

void Block::AssertHeuristics(const FreeChunksMap &_freeChunks)
{
    assert(GetStart() == &storage.front() && "Block's start is not the start of it's storage!");
    assert(GetFirstChunk()->GetStart() == GetStart() && "Block's first chunk does not start at the block's start!");

    Chunk *currentChunk = GetFirstChunk();
    size_t summedSize = 0;
    while (currentChunk)
    {
        //Chunk::AssertHeuristics handles gaps inbetween chunks
        currentChunk->AssertHeuristics();

        assert(chunks.find(currentChunk->start) != chunks.end() && "Block's chunks list does not contain a chunk that it should!");

        if (!currentChunk->allocated)
        {
            assert((currentChunk->next && currentChunk->next->allocated) && "Block has successive unallocated chunks!");
            assert(_freeChunks.at(currentChunk->GetSize()).at(currentChunk) == this && "Block has an unallocated chunk that is not in the free list!");
        }

        summedSize += currentChunk->GetSize();
        currentChunk = currentChunk->next;
    }

    for (auto &[start, chunk] : chunks)
        assert(start == chunk.start && "Block's chunk list has invalid key/value pair!");

    assert(summedSize == GetSize() && "The sum of the sizes of the block's chunks does not equal the size of the block!");
}

Heap::Heap() : blocks(), freeChunks(), size(0) {}

vm_byte *Heap::Alloc(vm_ui64 _amt)
{
    auto search = std::find_if(freeChunks.begin(), freeChunks.end(), [_amt](FreeChunksMap::reference _kvPair)
                               { return _kvPair.first >= _amt; });

    vm_byte *chunkStart = nullptr;

    if (search != freeChunks.end()) //There is a block with a big enough chunk
    {
        auto chunkAndBlock = search->second.extract(search->second.begin());

        chunkStart = chunkAndBlock.key()->GetStart();
        chunkAndBlock.mapped()->Alloc(chunkStart, _amt, freeChunks);
    }
    else
    {
        size_t newBlockSize = std::max({size, (size_t)MIN_HEAP_BLOCK_SIZE, (size_t)_amt}) * 2;
        Block &block = blocks.emplace_back(Block(newBlockSize, freeChunks));

        chunkStart = block.GetStart();
        size += block.GetSize();
        block.Alloc(chunkStart, _amt, freeChunks);
    }

#ifdef BUILD_DEBUG_HEAP
    AssertHeuristics();
#endif

    return chunkStart;
}

void Heap::Free(vm_byte *_addr)
{
    for (auto itBlock = blocks.begin(); itBlock != blocks.end(); itBlock++)
    {
        if (itBlock->HasAddress(_addr) && itBlock->IsAllocated(_addr))
        {
            itBlock->Free(_addr, freeChunks);

            //There is only one chunk in the block and it is unallocated so there
            //is no need to have the block existing and since the chunk in the block
            //is unallocated and thus in the free chunks list, we should remove it
            if (itBlock->IsEmpty())
            {
                freeChunks.at(itBlock->GetSize()).erase(itBlock->GetFirstChunk());
                size -= itBlock->GetSize();
                blocks.erase(itBlock);
            }

#ifdef BUILD_DEBUG_HEAP
            AssertHeuristics();
#endif

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

void Heap::AssertHeuristics()
{
    assert(freeChunks.find(0) == freeChunks.end() && "Free chunks list contains a size bucket for 0!");

    std::unordered_set<Chunk *> foundFreeChunks;
    for (auto &[chunkSize, chunks] : freeChunks)
    {
        for (auto &[chunk, block] : chunks)
        {
            assert(!chunk->IsAllocated() && "Free chunks list contains an allocated chunk!");
            assert(chunk->GetSize() == chunkSize && "Free chunks list size bucket contains a chunk with the incorrect size!");

            assert(foundFreeChunks.find(chunk) == foundFreeChunks.end() && "Free chunks list contains duplicate entries!");
            foundFreeChunks.insert(chunk);
        }
    }

    size_t expectedSize = 0;
    for (auto &block : blocks)
    {
        assert(!block.IsEmpty() && "Heap contains an empty block!");

        block.AssertHeuristics(freeChunks);
        expectedSize += block.GetSize();
    }

    assert(size == expectedSize && ("Expected size of heap to be " + std::to_string(expectedSize) + " but found Heap::size = " + std::to_string(size)).c_str());
}