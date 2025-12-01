#include "Achenginepch.h"
#include "WaterMesh.h"

#include "Achengine/Renderer/Renderer.h"
#include "Achengine/Actor/Actor.h"

namespace Achengine
{
	UWaterMesh::UWaterMesh()
	{
		m_SineFunction = new FSineFunction();
	}

    void UWaterMesh::AddSineFunction(FSineFunction* NewSineFunction)
    {
        if (m_SineFunction)
        {
            m_SineFunction->amplitude += NewSineFunction->amplitude;
            m_SineFunction->frequency += NewSineFunction->frequency;
            m_SineFunction->phase += NewSineFunction->phase;
        }
        else
        {
            m_SineFunction = NewSineFunction;
        }
    }

    void UWaterMesh::DrawMesh(RendererStorage* RenderData)
    {
        RenderData->BasicWaterShader->Bind();
        RenderData->BasicWaterShader->SetFloat4("u_Color", {1.0f, 0.8f, 0.2f, 1.0f});
        RenderData->WhiteTexture->Bind();

		RenderData->BasicWaterShader->Bind();
		RenderData->BasicWaterShader->SetMat4("u_Transform", GetOwner()->GetActorTransform());
		RenderData->BasicWaterShader->SetFloat("u_Amplitude", GetSineFunction()->amplitude);
		RenderData->BasicWaterShader->SetFloat("u_Frequency", GetSineFunction()->frequency);
		RenderData->BasicWaterShader->SetFloat("u_Phase", GetSineFunction()->phase);
        RenderData->BasicWaterShader->SetFloat("u_Time", Application::GetTimeSeconds());

		RenderData->QuadVertexArray->Bind();
		RenderCommand::DrawIndexed(RenderData->QuadVertexArray);
    }
}