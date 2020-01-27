#ifndef _ALLOCATOR_HPP_
#define _ALLOCATOR_HPP_

#include <cstddef>
#include <cstdint>

namespace GameEngine {
    struct BlockHeader
    {
        BlockHeader* pNext;
    };

    struct PageHeader{
        PageHeader* pNext;
        BlockHeader* Blocks(){
            return reinterpret_cast<BlockHeader*>(this+1);
        }
    };
    
    class  Allocator{
        public:
     
        // debug patterns
        static const uint8_t PATTERN_ALIGN = 0xFC;
        static const uint8_t PATTERN_ALLOC = 0xFD;
        static const uint8_t PATTERN_FREE  = 0xFE;
        
        // constructor
        Allocator();
        Allocator(size_t dataSize, size_t pageSize, size_t alignment);
    
        // destructor
        ~Allocator(void);
    
        // resets the allocator to a new configuration
        void Reset(size_t dataSize, size_t pageSize, size_t alignment);
    
        // allocates a block of memory
        void* Allocate(void);
    
        // deallocates a block of memory
        void Free(void *p);
    
        // deallocates all memory
        void FreeAll(void);
    
    private:
    #if defined(_DEBUG)
        // fill a free page with debug patterns
        void FillFreePage(PageHeader  *p);
    
        // fill a free block with debug patterns
        void FillFreeBlock(BlockHeader *p);
    
        // fill an allocated block with debug patterns
        void FillAllocatedBlock(BlockHeader *p);
    #endif
    
        // gets the next block
        BlockHeader *NextBlock(BlockHeader *p);
    
        // the page list
        PageHeader *m_pPageList;
    
        // the free list
        BlockHeader *m_pFreeList;
    
        // size-related data
        size_t m_szDataSize;
        size_t m_szPageSize;
        size_t m_szAlignmentSize;
        size_t m_szBlockSize;
        uint32_t m_nBlocksPerPage;
    
        // statistics
        uint32_t m_nPages;
        uint32_t m_nBlocks;
        uint32_t m_nFreeBlocks;
    
        // disable copy & assignment
        Allocator(const Allocator &clone);
        Allocator &operator=(const Allocator &rhs);
    };
}

#endif