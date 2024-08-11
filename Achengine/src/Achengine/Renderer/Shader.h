#pragma once

#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace Achengine
{
	class Shader
	{
	public:
		virtual ~Shader() = default;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void SetInt(const std::string& name, const int value) = 0;
		virtual void SetFloat3(const std::string& name, const glm::vec3 value) = 0;
		virtual void SetFloat4(const std::string& name, const glm::vec4 value) = 0;
		virtual void SetMat4(const std::string& name, const glm::mat4 value) = 0;

		virtual const std::string& GetName() const = 0;

		static Shader* Create(const std::string& filePath);
		static Shader* Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
	};

	class ShaderLibrary
	{
	public:
		~ShaderLibrary();

		void Add(Shader* shader);
		void Add(const std::string& name, Shader* shader);
		Shader* Load(const std::string& filePath);
		Shader* Load(const std::string& name, const std::string& filePath);

		Shader* Get(const std::string& name);

		bool Exists(const std::string& name) const;
	private:
		std::unordered_map<std::string, Shader*> m_Shaders;
	};
}