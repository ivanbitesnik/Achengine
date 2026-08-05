#include "Achenginepch.h"
#include "Achengine/Actor/Mesh.h"

#include "Achengine/Core/Utilities.h"
#include "Achengine/Renderer/Renderer.h"
#include "Achengine/Actor/WorldActorCache.h"

#include <glad/glad.h>
#include <cstdint>

namespace Achengine
{
    void UMesh::Initialize()
    {
        Renderer::AddShader(m_ShaderPath);
        m_VertexArrayName = format("%s_%llu", GetShaderName().c_str(), (unsigned long long)(uintptr_t)this);

        if (!Renderer::GetVertexArray(m_VertexArrayName))
        {
            GenerateVertexArray();
            SetUniforms();
        }
    }

    std::string UMesh::GetShaderName() const
    {
        return GetObjectNameFromFilePath(m_ShaderPath);
    }

    void UMesh::DrawMesh()
    {
        Renderer::DrawMesh(this);
    }

    void UMesh::DrawGeometry()
    {
        Renderer::DrawVertexArray(GetVertexArrayName());
    }
}