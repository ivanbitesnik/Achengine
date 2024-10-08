#pragma once

#include <memory>
#include "Buffer.h"

namespace Achengine
{

	class VertexArray
	{
	public:
		virtual ~VertexArray() {}

		static VertexArray* Create();

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void AddVertexBuffer(VertexBuffer* vertexBuffer) = 0;
		virtual void SetIndexBuffer(IndexBuffer* indexBuffer) = 0;

		virtual const std::vector<VertexBuffer*>& GetVertexBuffers() const = 0;
		virtual const IndexBuffer* GetIndexBuffer() const = 0;
	};
}