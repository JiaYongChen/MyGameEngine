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
        Allocator(size_t dataSize, size_t pageSize, size_t alignment);
    
        // destructor
        ~Allocator(void);
    
        // resets the allocator to a new configuration
        void Reset(size_t dataSize, size_t pageSize, size_t alignment);
    
        // allocates a block of memory
        void *Allocate(void);
    
        // deallocates a block of memory
        void Free(void *p);
    
        // deallocates all memory
        void FreeAll(void);
    
    private:
    
        // fill a free page with debug patterns
        void FillFreePage(PageHeader  *p);
    
        // fill a free block with debug patterns
        void FillFreeBlock(BlockHeader *p);
    
        // fill an allocated block with debug patterns
        void FillAllocatedBlock(BlockHeader *p);
    
        // gets the next block
        BlockHeader *NextBlock(BlockHeader *p);
    
        // the page list
        PageHeader *m_pageList;
    
        // the free list
        BlockHeader *m_freeList;
    
        // size-related data
        size_t m_dataSize     ;
        size_t m_pageSize     ;
        size_t m_alignmentSize;
        size_t m_blockSize    ;
        uint32_t m_blocksPerPage;
    
        // statistics
        uint32_t m_numPages     ;
        uint32_t m_numBlocks    ;
        uint32_t m_numFreeBlocks;
    
        // disable copy & assignment
        Allocator(const Allocator &clone);
        Allocator &operator=(const Allocator &rhs);
    };

}