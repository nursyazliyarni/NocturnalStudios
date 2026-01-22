/*!------------------------------------------------------------------------------------------------
    File: MemoryManager.cpp
    Project: NocturnalStudio

    Brief:
        The memory manager is responsible for handling dynamic memory allocation
        and deallocation within the engine. The memory manager provides a custom
        memory pool that can be enabled for usage within the engine.

    Primary Author:
         LAU XIANG YANG (100%)
        - Created the MemoryManager class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include <cstddef>
#include <cstdint>

#define p_MemoryManager NocturnalEngine::MemoryManager::GetInstance()

namespace NocturnalEngine
{
    /**
     * Simple fixed-size block memory manager.
     *
     * The manager can be enabled/disabled. When enabled it serves allocations
     * up to a configured block size from pages of fixed-size blocks. Larger or
     * disabled allocations fall back to the global operator new/delete.
     */
    class MemoryManager
    {
    public:
        /** Return the singleton instance. */
        static MemoryManager& GetInstance();

        /**
         * Configure block size and blocks-per-page.
         * This will Shutdown() the current pool. Both parameters must be > 0.
         *
         * @param blockSize Size in bytes for each block.
         * @param blocksPerPage Number of blocks per allocated page.
         */
        void Configure(std::size_t blockSize, std::size_t blocksPerPage);

        /**
         * Pre-allocate pages to accommodate at least expectedBlocks blocks.
         * Calls Shutdown() first to reset state.
         *
         * @param expectedBlocks Expected total number of blocks to prime.
         */
        void Prime(std::size_t expectedBlocks);

        /**
         * Allocate memory.
         *
         * If the manager is disabled, forwards to ::operator new.
         * If size == 0 returns nullptr.
         * If size > block size, forwards to ::operator new.
         * Otherwise returns a block from the free list (allocating a new page
         * if necessary).
         *
         * @param size Number of bytes requested.
         * @return Pointer to allocated memory or nullptr on failure.
         */
        void* Allocate(std::size_t size);

        /**
         * Free memory previously returned by Allocate().
         *
         * If the pointer belongs to a managed page it is returned to the free
         * list; otherwise it is forwarded to ::operator delete.
         *
         * @param ptr Pointer to free (may be nullptr).
         */
        void  Free(void* ptr);

        /** Enable or disable the memory manager. */
        void  Enable(bool enable);

        /** Query whether the memory manager is enabled. */
        bool  IsEnabled() const;

        /** Release all pages and reset internal counters. */
        void  Shutdown();

        /** Setters for configuration (no validation). */
        void SetBlockSize(std::size_t blockSize) { mBlockSize = blockSize; }
        void SetBlocksPerPage(std::size_t blocksPerPage) { mBlocksPerPage = blocksPerPage; }

        /** Getters for configuration and statistics. */
        std::size_t GetBlockSize() const { return mBlockSize; }
        std::size_t GetBlocksPerPage() const { return mBlocksPerPage; }
        std::size_t GetTotalMemoryAllocated() const { return mTotalAllocatedBytes; }
        std::size_t GetTotalMemoryUsed() const { return mBlocksInUse * mBlockSize; }
        std::size_t GetFreeBlocks() const { return mFreeBlockCount; }

    private:
        /** Free-list node stored inside each free block. */
        struct FreeBlock
        {
            FreeBlock* next;
        };

        /** Represents a single allocated page of blocks. */
        struct MemoryPage
        {
            char*       buffer; ///< raw storage for blocks
            MemoryPage* next;   ///< linked list of pages
        };

        MemoryManager();
        ~MemoryManager();
        MemoryManager(const MemoryManager&) = delete;
        MemoryManager& operator=(const MemoryManager&) = delete;

        /**
         * Allocate a new MemoryPage and link it into the pages list.
         * Updates total allocation and free-block counters.
         *
         * @return Pointer to the new page or nullptr on allocation failure.
         */
        MemoryPage* AllocatePage();

        /**
         * Initialize a freshly allocated page by splitting its buffer into
         * blocks and pushing them onto the free list.
         *
         * @param page Page to populate (must be non-null).
         */
        void        PopulatePage(MemoryPage* page);

        /** Release all pages and free their buffers. */
        void        ReleaseAllPages();

        /**
         * Determine whether ptr points inside one of the managed pages.
         *
         * @param ptr Pointer to test.
         * @param outPage If non-null and the pointer is owned, receives the page.
         * @return true if ptr belongs to a managed page.
         */
        bool        OwnsPointer(void* ptr, MemoryPage** outPage);

    private:
        bool         mEnabled;
        std::size_t  mBlockSize;
        std::size_t  mBlocksPerPage;
        MemoryPage*  mPages;
        FreeBlock*   mFreeList;

        std::size_t  mTotalAllocatedBytes;
        std::size_t  mBlocksInUse;
        std::size_t  mFreeBlockCount;

        static MemoryManager instance;
    };
}
