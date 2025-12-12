#include "Achenginepch.h"
#include "Achengine/Actor/Mesh.h"

#include "Achengine/Core/Utilities.h"
#include "Achengine/Renderer/Renderer.h"
#include "Achengine/Actor/WorldActorCache.h"

#include <glad/glad.h>

namespace Achengine
{
    void UMesh::Initialize()
    {
        Renderer::AddShader(m_ShaderPath);
        if (!Renderer::GetVertexArray(GetShaderName()))
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
}