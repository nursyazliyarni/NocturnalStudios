/*!------------------------------------------------------------------------------------------------
    File: VertexArray.cpp
    Project: NocturnalStudio

	Brief:
		This file contain the necessary functions for vertex array object
		it will bind the vertex buffer and index buffer to the vertex array object.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the vertex array class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Graphics/VertexArray.h"

namespace NocturnalEngine {

	////////////Vertex Array///////////
	/*!*************************************************************************
	Constructor for VertexArray
	****************************************************************************/
	VertexArray::VertexArray()
	{
		glCreateVertexArrays(1, &mRenderID);
	}

	/*!*************************************************************************
	Destructor for VertexArray
	****************************************************************************/
	VertexArray::~VertexArray()
	{
		glDeleteVertexArrays(0, &mRenderID);
	}

	/*!*************************************************************************
	Bind VertexArray
	****************************************************************************/
	void VertexArray::Bind() const
	{
		glBindVertexArray(mRenderID);
	}

	/*!*************************************************************************
	Bind VertexArray
	****************************************************************************/
	void VertexArray::Unbind() const
	{
		glBindVertexArray(0);
	}

	/*!*************************************************************************
	Add vertex buffer
	****************************************************************************/
	void VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& vertexbuffer)
	{
		glBindVertexArray(mRenderID);
		vertexbuffer->Bind();

		const auto& layout = vertexbuffer->GetLayout();
		for (const auto& element : layout)
		{
			glEnableVertexAttribArray(mVBIndex);
			glVertexAttribPointer(mVBIndex, element.GetComponentCount(),
				GL_FLOAT, element.Normalized, layout.GetStride(), (const void*) element.Offset);

			mVBIndex++;
		}

		mVertexBuffers.push_back(vertexbuffer);
	}

	/*!*************************************************************************
	Set index buffer
	****************************************************************************/
	void VertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& indexBuffer)
	{
		glBindVertexArray(mRenderID);
		indexBuffer->Bind();

		mIndexBuffers = indexBuffer;
	}
}