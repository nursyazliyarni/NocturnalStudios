/*!------------------------------------------------------------------------------------------------
    File: Buffer.cpp
    Project: NocturnalStudio

	Brief:
		This file contain all the helper function needed for buffer
		optimatization so that it will be easier to handle the buffer
		data
	
    Primary Author:
         Rishika Bimal Attodi (100%)
		- Created the buffer class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Graphics/Buffer.h"
#include "Graphics/VertexBuffer.h"

namespace NocturnalEngine {
	/*!*************************************************************************
	Dynamic draw buffer
	****************************************************************************/
	MultiRefs<VertexBuffer> VertexBuffer::Create(uint32_t size)
	{
		return CreateMultiRefs<VBuffer>(size);
	}

	/*!*************************************************************************
	Static draw buffer
	****************************************************************************/
	MultiRefs<VertexBuffer> VertexBuffer::Create(float* data, unsigned int size)
	{
		return CreateMultiRefs<VBuffer>(data, size);
	}

	/*!*************************************************************************
	draw buffer
	****************************************************************************/
	MultiRefs<IndexBuffer> IndexBuffer::Create(unsigned int* indices, unsigned int count)
	{
		return CreateMultiRefs<IBuffer>(indices, count);
	}
}