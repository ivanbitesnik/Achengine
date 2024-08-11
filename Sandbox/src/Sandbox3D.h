#pragma once

#include "Achengine.h"
#include "Achengine/Renderer/EditorCameraController.h"

class Sandbox3D : public Achengine::Layer
{
public:
	Sandbox3D();
	virtual ~Sandbox3D();

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	virtual void OnUpdate(Achengine::Timestep timestep) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(Achengine::Event& event) override;

private:
	Achengine::EditorCameraController* m_CameraController;

	Achengine::Texture2D* m_Texture;

	float angle, pos = 0.0f;

	glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };
};