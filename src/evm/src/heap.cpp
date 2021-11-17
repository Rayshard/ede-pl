#include "heap.h"
#include "../build.h"
#include "vm.h"
#include <algorithm>
#include <unordered_set>

#pragma region FreeChunksList

void FreeChunksList::Insert(Chunk* _chunk, Block* _block)
{
    map[_chunk->GetSize()][_chunk] = _block;
}

void FreeChunksList::Delete(Chunk* _chunk)
{
    auto& sizeBucket = map.at(_chunk->GetSize());
    sizeBucket.erase(_chunk);

    if (sizeBucket.empty())
        map.erase(_chunk->GetSize());
}

bool FreeChunksList::Contains(Chunk* _chunk, Block* _block) const
{
    auto sizeBucketSearch = map.find(_chunk->GetSize());
    if (sizeBucketSearch == map.end())
        return false;

    auto sizeBucketChunkSearch = sizeBucketSearch->second.find(_chunk);
    return sizeBucketChunkSearch != sizeBucketSearch->second.end() && sizeBucketChunkSearch->second == _block;
}

std::optional<std::pair<Chunk*, Block*>> FreeChunksList::Find(vm_ui64 _minSize)
{
    std::optional<std::pair<Chunk*, Block*>> result;
    auto search = std::find_if(map.begin(), map.end(), [_minSize](type::reference _kvPair)
        { return _kvPair.first >= _minSize; });

    if (search == map.end())
        return std::nullopt;

    return *search->second.begin();
}

void FreeChunksList::AssertHeuristics()
{
    assert(map.find(0) == map.end() && "Free chunks list contains a size bucket for 0!");

    std::unordered_set<Chunk*> foundChunks;
    for (auto& [chunkSize, chunks] : map)
    {
        for (auto& [chunk, block] : chunks)
        {
            assert(!chunk->IsAllocated() && "Free chunks list contains an allocated chunk!");
            assert(chunk->GetSize() == chunkSize && "Free chunks list size bucket contains a chunk with the incorrect size!");
            assert(foundChunks.find(chunk) == foundChunks.end() && "Free chunks list contains duplicate entries!");

            foundChunks.insert(chunk);
        }
    }
}

void FreeChunksList::Print()
{
    for (auto& [s, cb] : map)
    {
        std::cout << "Size Bucket: " << s << std::endl;

        for (auto& [c, b] : cb)
        {
            std::cout << "\t";
            c->Print(false);
            std::cout << " in block " << (void*)b << std::endl;
        }
    }
}

#pragma endregion

#pragma region Chunk

Chunk::Chunk(vm_byte* _start, vm_ui64 _size, Chunk* _prev, Chunk* _next)
    : start(_start), size(_size), prev(_prev), next(_next), allocated(false)
{
}

void Chunk::AssertHeuristics()
{
    assert((!next || (next->start == start + size)) && "Chunk's next chunk starts at a different place than after this chunk's end!");
    assert((!next || (next->prev == this)) && "Chunk's next should have it's previous be this chunk!");
    assert((!prev || (prev->next == this)) && "Chunk's previous should have it's next be this chunk!");
}

void Chunk::Print(bool _showData)
{
    std::cout << "Chunk at " << (void*)this << " { ";
    std::cout << "start: " << (void*)start << ", ";
    std::cout << "size: " << size << ", ";
    std::cout << "allocated: " << allocated << ", ";
    std::cout << "prev: " << (void*)prev << ", ";
    std::cout << "next: " << (void*)next;
    std::cout << " }";

    if (_showData)
    {
        for (vm_ui64 pos = 0; pos < size; pos++)
        {
            auto byte = start[pos];
            std::cout << (pos % 8 == 0 ? "\n\t\t" : "\t\t") << Hex(byte, false) << " | " << (std::isprint(byte) ? std::string(1, byte) : "~~");
        }
    }
}

#pragma endregion

#pragma region Block

Block::Block(size_t _size, FreeChunksList& _freeChunks)
{
    assert(_size > 0 && "Block must be initialized with positive size!");
    storage = Memory(_size);

    Chunk* initial = new Chunk(&storage.front(), storage.size(), nullptr, nullptr);
    _freeChunks.Insert(initial, this);
    chunks.emplace(initial->start, initial);
}

Block::Block(Block&& _b) noexcept { this->operator=(std::move(_b)); }

Block::~Block()
{
    for (auto& [_, chunk] : chunks)
        delete chunk;

    chunks.clear();
}

bool Block::HasAddress(vm_byte* _addr) { return _addr >= &storage.front() && _addr <= &storage.back(); }

bool Block::IsEmpty() { return chunks.size() == 1 && !chunks.begin()->second->allocated; }

bool Block::IsAllocated(vm_byte* _addr)
{
    auto search = chunks.find(_addr);
    return search != chunks.end() && search->second->allocated;
}

void Block::Alloc(vm_byte* _chunkPtr, vm_ui64 _amt, FreeChunksList& _freeChunks)
{
    Chunk* chunk = chunks.at(_chunkPtr);
    assert(_amt <= chunk->size && ("Cannot alloc " + std::to_string(_amt) + " bytes for an unallocated chunk of size " + std::to_string(chunk->size)).c_str());

    //Possibly fragment chunk
    if (_amt < chunk->size)
    {
        vm_byte* nextChunkStart = _chunkPtr + _amt;
        Chunk* next = new Chunk(nextChunkStart, chunk->size - _amt, chunk, chunk->next);

        if (next->next)
            next->next->prev = next;

        chunk->next = next;

        chunks.emplace(nextChunkStart, next);
        _freeChunks.Insert(next, this);
    }

    //Remove from free chunks list
    _freeChunks.Delete(chunk);

    //Set chunk
    chunk->allocated = true;
    chunk->size = _amt;

#ifdef BUILD_DEBUG_HEAP
    AssertHeuristics(_freeChunks);
#endif
}

void Block::Free(vm_byte* _chunkPtr, FreeChunksList& _freeChunks)
{
    Chunk* freedChunk = chunks.at(_chunkPtr), * current = freedChunk;
    freedChunk->allocated = false;

    //Start merge at previous chunk if it is unallocated
    if (freedChunk->prev && !freedChunk->prev->allocated)
    {
        _freeChunks.Insert(freedChunk, this);
        current = freedChunk->prev;
        _freeChunks.Delete(current); //Remove current chunk fron free list since its size is about to change
    }

    // Merge chunks
    while (current->next && !current->next->allocated)
    {
        auto chunkToDelete = current->next;
        _freeChunks.Delete(chunkToDelete); //Remove next chunk fron free list since it is being deleted

        //Merge chunks
        current->size += chunkToDelete->size;
        current->next = chunkToDelete->next;

        if (current->next)
            current->next->prev = current;

        //Delete chunk that was just merged with the current
        chunks.erase(chunkToDelete->start);
        delete chunkToDelete;
    }

    _freeChunks.Insert(current, this); //Put current chunk in free list

#ifdef BUILD_DEBUG_HEAP
    AssertHeuristics(_freeChunks);
#endif
}

void Block::AssertHeuristics(const FreeChunksList& _freeChunks)
{
    assert(GetStart() == &storage.front() && "Block's start is not the start of it's storage!");
    assert(GetFirstChunk()->GetStart() == GetStart() && "Block's first chunk does not start at the block's start!");

    Chunk* currentChunk = GetFirstChunk();
    size_t summedSize = 0, count = 0;

    while (currentChunk)
    {
        currentChunk->AssertHeuristics(); //Chunk::AssertHeuristics handles gaps inbetween chunks

        assert(chunks.find(currentChunk->start) != chunks.end() && "Block's chunks list does not contain a chunk that it should!");

        if (!currentChunk->allocated)
        {
            assert((!currentChunk->next || currentChunk->next->allocated) && "Block has successive unallocated chunks!");
            assert(_freeChunks.Contains(currentChunk, this) && "Block has an unallocated chunk that is not in the free list!");
        }

        summedSize += currentChunk->GetSize();
        currentChunk = currentChunk->next;
        count++;
    }

    assert(count == chunks.size() && "Chunks list contains a different amount of chunks than the chunk network!");

    for (auto& [start, chunk] : chunks)
        assert(start == chunk->start && "Block's chunk list has invalid key/value pair!");

    assert(summedSize == GetSize() && "The sum of the sizes of the block's chunks does not equal the size of the block!");
}

void Block::Print()
{
    std::cout << std::string(40, '=') << std::endl;
    std::cout << "Ptr: " << (void*)this << std::endl;
    std::cout << "Start: " << (void*)GetStart() << std::endl;
    std::cout << "Size: " << GetSize() << std::endl;

    for (auto& [chunkStart, chunk] : chunks)
    {
        std::cout << '\t';
        chunk->Print(true);
        std::cout << std::endl;
    }

    std::cout << std::string(40, '=') << std::endl;
}

#pragma endregion

#pragma region Heap

Heap::Heap(VM* _vm) : vm(_vm), blocks(), freeChunks(), size(0) { }

Heap::~Heap()
{
    for (auto& block : blocks)
        delete block;

    blocks.clear();
}

vm_byte* Heap::Alloc(vm_ui64 _amt)
{
    Block* block = nullptr;
    vm_byte* chunkStart = nullptr;

    if (auto chunkAndBlock = freeChunks.Find(_amt)) //There is a block with a big enough chunk
    {
        chunkStart = chunkAndBlock->first->GetStart();
        block = chunkAndBlock->second;
    }
    else
    {
        size_t newBlockSize = std::max({ size * 2, (size_t)MIN_HEAP_BLOCK_SIZE, (size_t)_amt * 2 });
        block = new Block(newBlockSize, freeChunks);

        blocks.push_back(block);
        chunkStart = block->GetStart();
        size += block->GetSize();
    }

    block->Alloc(chunkStart, _amt, freeChunks);

#ifdef BUILD_DEBUG_HEAP
    assert(IsAllocated(chunkStart) && "Allocation did not occur!");
    AssertHeuristics();
#endif

    return chunkStart;
}

void Heap::Free(vm_byte* _addr)
{
    for (auto itBlock = blocks.begin(); itBlock != blocks.end(); itBlock++)
    {
        auto block = *itBlock;

        if (block->HasAddress(_addr) && block->IsAllocated(_addr))
        {
            block->Free(_addr, freeChunks);

            //There is only one chunk in the block and it is unallocated so there
            //is no need to have the block existing and since the chunk in the block
            //is unallocated and thus in the free chunks list, we should remove it
            if (block->IsEmpty())
            {
                size -= block->GetSize();

                freeChunks.Delete(block->GetFirstChunk());
                blocks.erase(itBlock);

                delete block;
            }

#ifdef BUILD_DEBUG_HEAP
            AssertHeuristics();
            assert(!IsAllocated(_addr) && "Deallocation did not occur!");
#endif

            return;
        }
    }

    throw VMError::CANNOT_FREE_UNALLOCATED_PTR(_addr);
}

bool Heap::IsAddress(vm_byte* _addr)
{
    for (auto& block : blocks)
    {
        if (block->HasAddress(_addr))
            return true;
    }

    return false;
}

bool Heap::IsAddressRange(vm_byte* _start, vm_byte* _end)
{
    for (auto& block : blocks)
    {
        if (block->HasAddress(_start))
            return block->HasAddress(_end);
    }

    return false;
}

bool Heap::IsAllocated(vm_byte* _addr)
{
    for (auto& block : blocks)
    {
        if (block->HasAddress(_addr) && block->IsAllocated(_addr))
            return true;
    }

    return false;
}

void Heap::AssertHeuristics()
{
    freeChunks.AssertHeuristics();

    size_t expectedSize = 0;
    for (auto& block : blocks)
    {
        assert(!block->IsEmpty() && "Heap contains an empty block!");

        block->AssertHeuristics(freeChunks);
        expectedSize += block->GetSize();
    }

    assert(size == expectedSize && ("Expected size of heap to be " + std::to_string(expectedSize) + " but found Heap::size = " + std::to_string(size)).c_str());
}

void Heap::Print()
{
    std::cout << "Heap Size: " << size << std::endl;

    std::cout << "========================Blocks========================" << std::endl;
    for (auto& block : blocks)
        block->Print();
    std::cout << "===========================================================" << std::endl;

    std::cout << "========================Free Chunks========================" << std::endl;
    freeChunks.Print();
    std::cout << "===========================================================" << std::endl;
}

#pragma endregion