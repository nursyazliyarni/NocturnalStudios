/*!------------------------------------------------------------------------------------------------
    File: VertexBuffer.cpp
    Project: NocturnalStudio

	Brief:
		This file contain the necessary functions for vertex buffer object
		and index buffer object. It will bind the buffer to the GPU memory.
	
    Primary Author:
         LAU XIANG YANG (100%)
		- Created the vertex buffer class and its function specification.

    All content (c) 2025 DigiPen Institute of Technology Singapore, all rights reserved.
*------------------------------------------------------------------------------------------------*/

#include "pch.h"
#include "Graphics/VertexBuffer.h"

//new 
#include "GL/glew.h"
namespace NocturnalEngine{

	/*!*************************************************************************
	Constructor for VBuffer
	****************************************************************************/
	VBuffer::VBuffer(uint32_t size)
	{
		glCreateBuffers(1, &mRenderID);
		glBindBuffer(GL_ARRAY_BUFFER, mRenderID);
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
	}

	/*!*************************************************************************
	Overloaded Constructor for VBuffer
	****************************************************************************/
	VBuffer::VBuffer(float* vertices, unsigned int size)
	{
		glCreateBuffers(1, &mRenderID);
		glBindBuffer(GL_ARRAY_BUFFER, mRenderID);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
	}

	/*!*************************************************************************
	Destructor for VBuffer
	****************************************************************************/
	VBuffer::~VBuffer()
	{
		glDeleteBuffers(1, &mRenderID);
	}

	/*!*************************************************************************
	Bind VBuffer
	****************************************************************************/
	void VBuffer::Bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, mRenderID);
	}

	/*!*************************************************************************
	Unbind VBuffer
	****************************************************************************/
	void VBuffer::Unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	/*!*************************************************************************
	Set buffer data
	****************************************************************************/
	void VBuffer::SetBufferData(const void* data, unsigned int size)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mRenderID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
	}


	////////////INDEX BUFFER///////////
	/*!*************************************************************************
	Constructor for IBuffer
	****************************************************************************/
	IBuffer::IBuffer(unsigned int* indices, unsigned int count)
		:mCount(count)
	{
		glCreateBuffers(1, &mRenderID);
		glBindBuffer(GL_ARRAY_BUFFER, mRenderID);
		glBufferData(GL_ARRAY_BUFFER, count * sizeof(unsigned int), indices, GL_STATIC_DRAW);
	}

	/*!*************************************************************************
	Destructor for IBuffer
	****************************************************************************/
	IBuffer::~IBuffer()
	{
		glDeleteBuffers(1, &mRenderID);
	}

	/*!*************************************************************************
	Bind for IBuffer
	****************************************************************************/
	void IBuffer::Bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRenderID);
	}

	/*!*************************************************************************
	Unbind for IBuffer
	****************************************************************************/
	void IBuffer::Unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}
