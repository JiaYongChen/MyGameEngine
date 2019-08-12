#include "Allocator.hpp"
#include <cassert>
#include <cstring>

#ifndef ALIGN
#define ALIGN(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
#endif

using namespace GameEngine;

GameEngine::Allocator::Allocator(size_t dataSize, size_t pageSize, size_t alignment)
:m_pPageList(nullptr), m_pFreeList(nullptr)
{
    Reset(dataSize, pageSize, alignment);
}

GameEngine::Allocator::~Allocator()
{
    FreeAll();
}

void GameEngine::Allocator::Reset(size_t dataSize, size_t pageSize, size_t alignment)
{
    FreeAll();

    m_szDataSize = dataSize;
    m_szPageSize = pageSize;

    size_t minimalSize = (sizeof(BlockHeader) > m_szDataSize) ? sizeof(BlockHeader) : m_szDataSize;
    //this magic only works when alignment is 2^n, which should general be the case
    //because most CPU/GPU also requires the aligment be in 2^n
    //but still we use a assert to guarantee it
#if defined(_DEBUG)
    assert(alignment > 0 && ((alignment & (alignment - 1))) == 0);
#endif
    m_szBlockSize = ALIGN(minimalSize, alignment);
    m_szAlignmentSize = m_szBlockSize - minimalSize;
    m_nBlocksPerPage = (m_szPageSize - sizeof(PageHeader)) / m_szBlockSize;
}

void* GameEngine::Allocator::Allocate(void)
{
    
}