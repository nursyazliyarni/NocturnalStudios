/*!------------------------------------------------------------------------------------------------
    File: VertexArray.h
    Project: NocturnalStudio

	Brief:
		This file contain VertexBuffer binding and unbind set the buffer layout
		to the vertex array needed for gpu rendering.
		
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the VertexArray.h file and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#pragma once
#include "Buffer.h"
namespace NocturnalEngine {

	class VertexArray
	{
	public:
		VertexArray();
		~VertexArray();
		void Bind() const;
		void Unbind() const;

		void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexbuffer);
		void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer);

		const std::vector<std::shared_ptr<VertexBuffer>>& GetVertexBuffers() const { return mVertexBuffers; }
		const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const { return mIndexBuffers; }
		static MultiRefs<VertexArray> Create() { return CreateMultiRefs<VertexArray>(); }
	private:
		unsigned int mRenderID;
		unsigned int mVBIndex = 0;
		std::vector<std::shared_ptr<VertexBuffer>> mVertexBuffers;
		std::shared_ptr<IndexBuffer> mIndexBuffers;
	};
}