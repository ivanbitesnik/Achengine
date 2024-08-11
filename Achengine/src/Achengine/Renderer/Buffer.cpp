#include "Achenginepch.h"
#include "Buffer.h"

#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Renderer.h"

namespace Achengine
{

	void BufferLayout::CalculateOffsetsAndStride()
	{
		uint32_t offset = 0;
		m_Stride = 0;
		for (BufferElement& element : m_Elements)
		{
			element.Offset = offset;
			offset += element.Size;
			m_Stride += element.Size;
		}
	}

	VertexBuffer* VertexBuffer::Create(uint32_t size, float* vertices)
	{
		switch (*Renderer::GetAPI())
		{
			case RendererAPI::API::None: 
			{
				ACHENGINE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
				return nullptr;
			}

			case RendererAPI::API::OpenGL:
			{
				return new OpenGLVertexBuffer(size, vertices);
			}
		}

		ACHENGINE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t count, uint32_t* indices)
	{
		switch (*Renderer::GetAPI())
		{
			case RendererAPI::API::None:
			{
				ACHENGINE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
					return nullptr;
			}

			case RendererAPI::API::OpenGL:
			{
				return new OpenGLIndexBuffer(count, indices);
			}
		}

		ACHENGINE_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}