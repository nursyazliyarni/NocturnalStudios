
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