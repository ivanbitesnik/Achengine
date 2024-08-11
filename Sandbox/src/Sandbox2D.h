#pragma once

#include "Achengine.h"
#include "Achengine/Renderer/OrthographicCameraController.h"

class Sandbox2D : public Achengine::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D();

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(Achengine::Timestep timestep) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(Achengine::Event& event) override;

private:
	Achengine::OrthographicCameraController* m_CameraController;

	Achengine::Texture2D* m_Texture;
	float angle, pos = 0.0f;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};