#pragma once

#include "Achengine.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

struct ImGuiViewport;

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
	bool SaveMapToFile(const std::string& filePath);
	bool LoadMapFromFile(const std::string& filePath);
	void ReinitializeGameGlobals();
	void SpawnDefaultScene();
	void StartPlayMode();
	void StopPlayMode();
	void EnsurePlaySessionActorPossession();
	void UpdateCollisionBroadPhase();
	Achengine::Camera* GetActiveSceneCamera() const;
	glm::vec3 GetActiveSceneCameraPosition(const Achengine::Camera* camera) const;
	void RenderOutlinerPanel(const ImGuiViewport* viewport, float minOutlinerWidth, float maxOutlinerWidth);
	void RenderMapPanel(const ImGuiViewport* viewport, float topPanelWidth, float minTopPanelHeight, float maxTopPanelHeight);

	enum class EGizmoMode
	{
		Translate = 0,
		Rotate = 1,
		Scale = 2
	};

	enum class EGizmoSpace
	{
		World = 0,
		Local = 1
	};

	Achengine::EditorCameraController* m_CameraController;
	Achengine::APlayerController* m_PlayerController = nullptr;
	Achengine::APlayer* m_PlayerActor = nullptr;
	Achengine::GameGlobals* m_GameGlobals = nullptr;
	Achengine::CollisionOctree m_CollisionOctree;
	bool m_IsPlaying = false;
	uint32_t m_CollisionCandidateCount = 0;
	uint32_t m_ConfirmedCollisionCount = 0;
	std::unordered_set<uint64_t> m_ActiveCollisionPairs;
	std::unordered_map<uint64_t, std::pair<Achengine::UActorComponent*, Achengine::UActorComponent*>> m_ActiveCollisionComponents;

	Achengine::AActor* m_ModelActor = nullptr;
	Achengine::UMesh* m_ModelMesh = nullptr;
	Achengine::AActor* m_ActiveActor = nullptr;
	std::unordered_set<Achengine::AActor*> m_SelectedActors;
	glm::vec3 m_ModelScale = {6.0f, 6.0f, 6.0f};
	char m_ModelPathBuffer[512] = {};
	char m_MapPathBuffer[512] = {};
	std::string m_MapStatus;
	float m_TopMapPanelHeight = 92.0f;
	float m_AssetBrowserHeight = 300.0f;
	float m_OutlinerWidth = 360.0f;
	EGizmoMode m_GizmoMode = EGizmoMode::Translate;
	EGizmoSpace m_GizmoSpace = EGizmoSpace::World;
	bool m_UseGizmoSnapping = false;
	float m_TranslateSnapStep = 1.0f;
	float m_RotateSnapStep = 15.0f;
	float m_ScaleSnapStep = 0.1f;
	int m_GizmoActiveAxis = -1;
	bool m_GizmoDragging = false;
};