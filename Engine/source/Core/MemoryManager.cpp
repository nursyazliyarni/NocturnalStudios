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

#include "pch.h"
#include "Core/MemoryManager.h"
#include <new>

namespace NocturnalEngine
{
	MemoryManager MemoryManager::instance;	

	/*!*************************************************************************
	Returns an instance of the MemoryManager
	****************************************************************************/
	MemoryManager& MemoryManager::GetInstance()
	{
		return instance;
	}
	/*!*************************************************************************
	Constructor
	****************************************************************************/
	MemoryManager::MemoryManager()
		: mEnabled(false),
		mBlockSize(kDefaultBlockSize),
		mBlocksPerPage(kDefaultBlocksPerPage),
		mPages(nullptr),
		mFreeList(nullptr),
		mTotalAllocatedBytes(0u),
		mBlocksInUse(0u),
		mFreeBlockCount(0u)
	{
	}
	/*!*************************************************************************
	Destructor
	****************************************************************************/
	MemoryManager::~MemoryManager()
	{
		Shutdown();
	}
	/*!*************************************************************************
	Configure block size and blocks-per-page of the memory manager.
	****************************************************************************/
	void MemoryManager::Configure(std::size_t blockSize, std::size_t blocksPerPage)
	{
		if (blockSize == 0u || blocksPerPage == 0u)
		{
			return;
		}

		Shutdown();
		mBlockSize = blockSize;
		mBlocksPerPage = blocksPerPage;
	}
	/*!*************************************************************************
	To pre-allocate pages to accommodate at least expectedBlocks blocks.
	****************************************************************************/
	void MemoryManager::Prime(std::size_t expectedBlocks)
	{
		Shutdown();

		if (expectedBlocks == 0u)
		{
			return;
		}

		std::size_t totalBlocksPerPage = mBlocksPerPage;
		std::size_t pagesNeeded = expectedBlocks / totalBlocksPerPage;
		if ((expectedBlocks % totalBlocksPerPage) != 0u)
		{
			++pagesNeeded;
		}

		for (std::size_t i = 0; i < pagesNeeded; ++i)
		{
			MemoryPage* page = AllocatePage();
			if (!page)
			{
				break;
			}
			PopulatePage(page);
		}
	}
	/*!*************************************************************************
	Memory allocation function
	****************************************************************************/
	void* MemoryManager::Allocate(std::size_t size)
	{
		if (!mEnabled)
		{
			return ::operator new(size);
		}

		if (size == 0u)
		{
			return nullptr;
		}

		if (size > mBlockSize)
		{
			return ::operator new(size);
		}

		if (!mFreeList)
		{
			MemoryPage* page = AllocatePage();
			if (page)
			{
				PopulatePage(page);
			}
			else
			{
				return nullptr;
			}
		}

		FreeBlock* block = mFreeList;
		mFreeList = block->next;

		if (mFreeBlockCount > 0u)
		{
			--mFreeBlockCount;
		}
		++mBlocksInUse;

		return static_cast<void*>(block);
	}
	/*!*************************************************************************
	Return memory to the memory manager
	****************************************************************************/
	void MemoryManager::Free(void* ptr)
	{
		if (!ptr)
		{
			return;
		}

		MemoryPage* owningPage = nullptr;
		bool managed = OwnsPointer(ptr, &owningPage);

		if (managed)
		{
			FreeBlock* block = static_cast<FreeBlock*>(ptr);
			block->next = mFreeList;
			mFreeList = block;

			++mFreeBlockCount;
			if (mBlocksInUse > 0u)
			{
				--mBlocksInUse;
			}
			return;
		}

		::operator delete(ptr);
	}
	/*!*************************************************************************
	Enable or disable the memory manager
	****************************************************************************/
	void MemoryManager::Enable(bool enable)
	{
		mEnabled = enable;
	}
	/*!*************************************************************************
	Get whether the memory manager is enabled
	****************************************************************************/
	bool MemoryManager::IsEnabled() const
	{
		return mEnabled;
	}
	/*!*************************************************************************
	Release all pages and reset internal counters
	****************************************************************************/
	void MemoryManager::Shutdown()
	{
		ReleaseAllPages();
		mFreeList = nullptr;
		mTotalAllocatedBytes = 0u;
		mBlocksInUse = 0u;
		mFreeBlockCount = 0u;
	}
	/*!*************************************************************************
	Allocate a new MemoryPage and link it into the pages list.
	****************************************************************************/
	MemoryManager::MemoryPage* MemoryManager::AllocatePage()
	{
		MemoryPage* page = new MemoryPage();
		page->buffer = new (std::nothrow) char[mBlockSize * mBlocksPerPage];
		if (!page->buffer)
		{
			delete page;
			return nullptr;
		}
		page->next = mPages;
		mPages = page;

		mTotalAllocatedBytes += mBlockSize * mBlocksPerPage;
		mFreeBlockCount += mBlocksPerPage;

		return page;
	}
	/*!*************************************************************************
	Populate a freshly allocated page by splitting its buffer into blocks
	****************************************************************************/
	void MemoryManager::PopulatePage(MemoryPage* page)
	{
		if (!page)
		{
			return;
		}

		char* cursor = page->buffer;
		for (std::size_t i = 0; i < mBlocksPerPage; ++i)
		{
			FreeBlock* block = reinterpret_cast<FreeBlock*>(cursor);
			block->next = mFreeList;
			mFreeList = block;
			cursor += mBlockSize;
		}
	}
	/*!*************************************************************************
	Release all pages and free their buffers.
	****************************************************************************/
	void MemoryManager::ReleaseAllPages()
	{
		MemoryPage* page = mPages;
		while (page)
		{
			MemoryPage* next = page->next;
			delete[] page->buffer;
			delete page;
			page = next;
		}
		mPages = nullptr;
	}
	/*!*************************************************************************
	Check if a pointer belongs to a managed page
	****************************************************************************/
	bool MemoryManager::OwnsPointer(void* ptr, MemoryPage** outPage)
	{
		MemoryPage* page = mPages;
		while (page)
		{
			char* start = page->buffer;
			char* end = start + (mBlockSize * mBlocksPerPage);
			if (ptr >= start && ptr < end)
			{
				if (outPage)
				{
					*outPage = page;
				}
				return true;
			}
			page = page->next;
		}
		return false;
	}
}
