#include "Achenginepch.h"
#include "LightComponent.h"

#include "Achengine/Renderer/Renderer.h"
#include "Achengine/Actor/Actor.h"

namespace Achengine
{
	ULightComponent::ULightComponent()
	{
		m_LightSource = new FLightSource();
	}

	ULightComponent::~ULightComponent()
	{
		delete m_LightSource;
		m_LightSource = nullptr;
	}

	void ULightComponent::Tick(float DeltaTime)
	{
		(void)DeltaTime;
	}

	void ULightComponent::SubmitLighting() const
	{
		if (!GetOwner() || !GetLightSource())
		{
			return;
		}

		Renderer::AddSceneLight(
			GetOwner()->GetActorLocation(),
			GetLightSource()->ambient,
			GetLightSource()->diffuse,
			GetLightSource()->specular,
			GetLightSource()->constant,
			GetLightSource()->linear,
			GetLightSource()->quadratic
		);
	}
}
