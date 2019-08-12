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
    if(!m_pFreeList){
        //allocate a new page
        PageHeader* pNewPage = reinterpret_cast<PageHeader*>(new uint8_t[m_szPageSize]);
        ++m_nPages;
        m_nBlocks += m_nBlocksPerPage;
        m_nFreeBlocks += m_nBlocksPerPage;

#if defined(_DEBUG)
        FillFreePage(pNewPage);
#endif
        if(m_pPageList){
            pNewPage->pNext = m_pPageList;
        }

        m_pPageList = pNewPage;

        BlockHeader* pBlock = pNewPage->Blocks();
        //link each block in the page
        for (uint32_t i = 0; i < m_nBlocksPerPage; ++i){
            pBlock->pNext = NextBlock(pBlock);
            pBlock = NextBlock(pBlock);
        }
        pBlock->pNext = nullptr;

        m_pFreeList = pNewPage->Blocks();
    }

    BlockHeader* freeBlock = m_pFreeList;
    m_pFreeList = m_pFreeList->pNext;
    --m_nFreeBlocks;

#if defined(_DEBUG)
    FillAllocatedBlock(freeBlock);
#endif

    return reinterpret_cast<void*>(freeBlock);
}

void GameEngine::Allocator::Free(void* p)
{
    BlockHeader* block = reinterpret_cast<BlockHeader*>(p);

#if defined(_DEBUG)
    FillAllocatedBlock(block);
#endif

    block->pNext = m_pFreeList;
    m_pFreeList = block;
    ++m_nFreeBlocks;
}

void GameEngine::Allocator::FreeAll()
{
    PageHeader* pPage = m_pPageList;
    while (pPage)
    {
        PageHeader* _p = pPage;
        pPage = pPage->pNext;

        delete [] reinterpret_cast<uint32_t*>(_p);
    }

    m_pPageList = nullptr;
    m_pFreeList = nullptr;

    m_nPages = 0;
    m_nBlocks = 0;
    m_nFreeBlocks = 0;
}

#if defined(_DEBUG)
void GameEngine::Allocator::FillFreePage(PageHeader* pPage)
{
    //page header
    pPage->pNext = nullptr;

    //blocks
    BlockHeader* pBlock = pPage->Blocks();
    for (uint32_t i = 0; i < m_nBlocksPerPage; i++)
    {
        FillFreeBlock(pBlock);
        pBlock = NextBlock(pBlock);
    }
}

void GameEngine::Allocator::FillFreeBlock(BlockHeader* pBlock)
{
    //block header + data
    std::memset(pBlock, PATTERN_FREE, m_szBlockSize - m_szAlignmentSize);

    //alignment
    std::memset(reinterpret_cast<uint8_t*>(pBlock) + m_szBlockSize - m_szAlignmentSize, PATTERN_ALIGN, m_szAlignmentSize);
}

void GameEngine::Allocator::FillAllocatedBlock(BlockHeader *pBlock)
{
    //block header + data
    std::memset(pBlock, PATTERN_ALLOC, m_szBlockSize - m_szAlignmentSize);

    //alignment
    std::memset(reinterpret_cast<uint8_t*>(pBlock) + m_szBlockSize - m_szAlignmentSize, PATTERN_ALIGN, m_szAlignmentSize);
}
#endif

GameEngine::BlockHeader* GameEngine::Allocator::NextBlock(BlockHeader* pBlock)
{
    return reinterpret_cast<BlockHeader*>(reinterpret_cast<uint8_t*>(pBlock) + m_szBlockSize);
}