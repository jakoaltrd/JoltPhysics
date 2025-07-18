// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/NonCopyable.h>
#include <Jolt/Core/Memory.h>

JPH_NAMESPACE_BEGIN

/// Allocator for temporary allocations. 
/// This allocator works as a stack: The blocks must always be freed in the reverse order as they are allocated.
/// Note that allocations and frees can take place from different threads, but the order is guaranteed though
/// job dependencies, so it is not needed to use any form of locking.
class TempAllocator : public NonCopyable
{
public:
	/// Destructor
	virtual							~TempAllocator() = default;

	/// Allocates inSize bytes of memory, returned memory address must be 16 byte aligned
	virtual void *					Allocate(uint inSize) = 0;

	/// Frees inSize bytes of memory located at inAddress
	virtual void					Free(void *inAddress, uint inSize) = 0;
};

/// Default implementation of the temp allocator that allocates a large block through malloc upfront
class TempAllocatorImpl final : public TempAllocator
{
public:
	/// Constructs the allocator with a maximum allocatable size of inSize
	explicit						TempAllocatorImpl(uint inSize) :
		mBase(static_cast<uint8 *>(malloc(inSize))),
		mSize(inSize)
	{
	}

	/// Destructor, frees the block
	virtual							~TempAllocatorImpl() override
	{
		JPH_ASSERT(mTop == 0);
		free(mBase);
	}

	// See: TempAllocator
	virtual void *					Allocate(uint inSize) override
	{
		if (inSize == 0)
		{
			return nullptr;
		}
		else
		{
			uint new_top = mTop + AlignUp(inSize, 16);
			if (new_top > mSize)
				JPH_CRASH; // Out of memory
			void *address = mBase + mTop;
			mTop = new_top;
			return address;
		}
	}

	// See: TempAllocator
	virtual void					Free(void *inAddress, uint inSize) override
	{
		if (inAddress == nullptr)
		{
			JPH_ASSERT(inSize == 0);
		}
		else
		{
			mTop -= AlignUp(inSize, 16);
			if (mBase + mTop != inAddress)
				JPH_CRASH; // Freeing in the wrong order
		}
	}

	// Check if no allocations have been made
	bool							IsEmpty() const
	{
		return mTop == 0;
	}

private:
	uint8 *							mBase;							///< Base address of the memory block
	uint							mSize;							///< Size of the memory block
	uint							mTop = 0;						///< Current top of the stack
};

/// Implementation of the TempAllocator that just falls back to malloc/free
/// Note: This can be quite slow when running in the debugger as large memory blocks need to be initialized with 0xcd
class TempAllocatorMalloc final : public TempAllocator
{
public:
	// See: TempAllocator
	virtual void *					Allocate(uint inSize) override
	{
		return AlignedAlloc(inSize, 16);
	}

	// See: TempAllocator
	virtual void					Free(void *inAddress, uint inSize) override
	{
		AlignedFree(inAddress);
	}
};

JPH_NAMESPACE_END
