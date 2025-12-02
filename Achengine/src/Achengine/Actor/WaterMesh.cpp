#include "Achenginepch.h"
#include "WaterMesh.h"

#include "Achengine/Renderer/Renderer.h"
#include "Achengine/Actor/Actor.h"

namespace Achengine
{
    UWaterMesh::UWaterMesh()
    {
#ifdef ACHENGINE_PLATFORM_LINUX
        m_ShaderPath = "/home/acheto/Desktop/projects/Achengine/Sandbox/assets/shaders/BasicWater.glsl";
#else
        m_ShaderPath = "assets/shaders/BasicWater.glsl";
#endif
        m_ShaderName = "BasicWaterShader";

        Initialize();
    }

    void UWaterMesh::DrawMesh(RendererStorage* RenderData)
    {
		Renderer::SetShaderUniform(m_ShaderName, "u_Transform", GetOwner()->GetActorTransform());

        constexpr int numWaves = 8;
        Renderer::SetShaderUniform(m_ShaderName, "u_NumWaves", numWaves);
        Renderer::SetShaderUniform(m_ShaderName, "u_envMap", 0);
		for (int i = 0; i < numWaves; ++i) 
        {
            float amplitude = 0.5f / (i + 1);
            Renderer::SetShaderUniform(m_ShaderName, format("u_Amplitude[%d]", i), amplitude);

            float wavelength = 8 * M_PI / (i + 1);
            Renderer::SetShaderUniform(m_ShaderName, format("u_Wavelength[%d]", i), wavelength);

            float speed = 1.0f + 2*i;
            Renderer::SetShaderUniform(m_ShaderName, format("u_Speed[%d]", i), speed);
            
            float angle = uniformRandomInRange(-M_PI/3, M_PI/3);
            Renderer::SetShaderUniform(m_ShaderName, format("u_Direction[%d]", i), {cos(angle), sin(angle)});
        }

		RenderData->BasicWaterVertexArray->Bind();
		RenderCommand::DrawIndexed(RenderData->BasicWaterVertexArray);
    }
}