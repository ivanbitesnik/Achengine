#pragma once

#include "Achengine/Core/Core.h"
#include <string>

namespace Achengine
{
	class Texture
	{
	public:
		virtual ~Texture() = default;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static Texture2D* Create(uint32_t width, uint32_t height);
		static Texture2D* Create(const std::string& path);
	};
}