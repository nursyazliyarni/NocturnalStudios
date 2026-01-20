/*!------------------------------------------------------------------------------------------------
    File: VertexBuffer.h
    Project: NocturnalStudio

	Brief:
		This file contain the neccesary declaration functions for class vertexbuffer,
		vertexarray, index array and buffer element and layout.
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the VertexBuffer.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Buffer.h"
namespace NocturnalEngine
{
	class VBuffer : public VertexBuffer
	{
	public:
		VBuffer(uint32_t size);
		VBuffer(float* data, unsigned int size);
		virtual ~VBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;
		virtual void SetBufferData(const void* data, unsigned int size) override;

		virtual const BufferLayout& GetLayout() const override { return mLayout; }
		virtual void  SetLayout(const BufferLayout& layout) override { mLayout = layout; }
	private:
		unsigned int mRenderID;
		BufferLayout mLayout;
	};

	
	class IBuffer : public IndexBuffer
	{
	public:
		 IBuffer(unsigned int* indices, unsigned int count);
		 virtual ~IBuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual unsigned int GetCount() const { return mCount; }
	private:

		unsigned int mRenderID;
		unsigned int mCount;
	};
	
}
