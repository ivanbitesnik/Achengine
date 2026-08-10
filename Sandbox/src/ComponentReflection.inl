#ifndef SANDBOX_COMPONENT_REFLECTION_INL
#define SANDBOX_COMPONENT_REFLECTION_INL

	struct FComponentPropertyDescriptor
	{
		std::function<void(Achengine::UActorComponent*, std::unordered_map<const Achengine::UActorComponent*, std::array<char, 512>>&, std::string&)> Draw;
	};

	struct FComponentReflectionDescriptor
	{
		const char* SectionName = "";
		std::function<bool(Achengine::UActorComponent*)> Matches;
		std::vector<FComponentPropertyDescriptor> Properties;
		std::function<void(const Achengine::UActorComponent*, std::ostream&)> SerializeOverrideJson;
		std::function<void(Achengine::UActorComponent*, const FJsonValue&, const std::string&)> ApplyOverrideJson;
	};

	static const std::vector<FComponentReflectionDescriptor>& GetComponentReflectionDescriptors();

	static void WriteComponentOverrideJson(std::ostream& out, const Achengine::UActorComponent* component, size_t componentIndex)
	{
		const std::string typeTag = GetComponentTypeTag(component);

		out << "        {\n";
		out << "          \"index\": " << componentIndex << ",\n";
		out << "          \"type\": \"" << JsonEscape(typeTag) << "\"";

		for (const FComponentReflectionDescriptor& descriptor : GetComponentReflectionDescriptors())
		{
			if (!descriptor.Matches || !descriptor.SerializeOverrideJson)
			{
				continue;
			}

			Achengine::UActorComponent* mutableComponent = const_cast<Achengine::UActorComponent*>(component);
			if (!descriptor.Matches(mutableComponent))
			{
				continue;
			}

			descriptor.SerializeOverrideJson(component, out);
		}

		out << "\n        }";
	}

	static void ApplyComponentOverrideFromJson(Achengine::UActorComponent* component, const FJsonValue& componentValue, const std::string& mapFilePath)
	{
		if (!component || componentValue.Type != FJsonValue::EType::Object)
		{
			return;
		}

		for (const FComponentReflectionDescriptor& descriptor : GetComponentReflectionDescriptors())
		{
			if (!descriptor.Matches || !descriptor.ApplyOverrideJson)
			{
				continue;
			}

			if (!descriptor.Matches(component))
			{
				continue;
			}

			descriptor.ApplyOverrideJson(component, componentValue, mapFilePath);
		}
	}

	static const char* GetComponentDisplayName(const Achengine::UActorComponent* component)
	{
		const std::string typeTag = GetComponentTypeTag(component);
		if (typeTag == "camera")
		{
			return "UCameraComponent";
		}
		if (typeTag == "light")
		{
			return "ULightComponent";
		}
		if (typeTag == "water")
		{
			return "UWaterMesh";
		}
		if (typeTag == "mesh")
		{
			return "UMesh";
		}

		return "UActorComponent";
	}

	static void SyncModelPathBufferForComponent(Achengine::UActorComponent* component, std::unordered_map<const Achengine::UActorComponent*, std::array<char, 512>>& modelPathBuffers)
	{
		if (Achengine::UMesh* mesh = dynamic_cast<Achengine::UMesh*>(component))
		{
			auto& buffer = modelPathBuffers[component];
			std::strncpy(buffer.data(), mesh->GetModelPath().c_str(), buffer.size() - 1);
			buffer[buffer.size() - 1] = '\0';
		}
	}

	static void DrawDefaultComponentTransformProperties(Achengine::UActorComponent* component)
	{
		if (!component)
		{
			return;
		}

		glm::vec3 compLocation = component->GetRelativeLocation();
		if (ImGui::DragFloat3("Comp Location", &compLocation.x, 0.1f))
		{
			component->SetRelativeLocation(compLocation);
		}

		Achengine::FRotation compRotation = component->GetRelativeRotation();
		glm::vec3 compAxis = compRotation.RotationAxis;
		if (glm::length(compAxis) < 0.0001f)
		{
			compAxis = glm::vec3(1.0f, 0.0f, 0.0f);
		}
		float compAngle = compRotation.Angle;
		if (ImGui::DragFloat3("Comp Rot Axis", &compAxis.x, 0.01f, -1.0f, 1.0f))
		{
			if (glm::length(compAxis) < 0.0001f)
			{
				compAxis = glm::vec3(1.0f, 0.0f, 0.0f);
			}
			component->SetRelativeRotation(glm::normalize(compAxis), compAngle);
		}
		if (ImGui::DragFloat("Comp Rot Angle", &compAngle, 0.25f, -360.0f, 360.0f))
		{
			component->SetRelativeRotation(glm::normalize(compAxis), compAngle);
		}

		glm::vec3 compScale = component->GetComponentScale();
		if (ImGui::DragFloat3("Comp Scale", &compScale.x, 0.05f, 0.01f, 1000.0f))
		{
			component->SetComponentScale(compScale);
		}
	}

	static const std::vector<FComponentReflectionDescriptor>& GetComponentReflectionDescriptors()
	{
		static const std::vector<FComponentReflectionDescriptor> descriptors = {
			{
				"",
				[](Achengine::UActorComponent* component) {
					return component != nullptr;
				},
				{},
				[](const Achengine::UActorComponent* component, std::ostream& out) {
					if (!component)
					{
						return;
					}

					Achengine::FRotation componentRotation = component->GetRelativeRotation();
					glm::vec3 rotationAxis = componentRotation.RotationAxis;
					if (glm::length(rotationAxis) < 0.0001f)
					{
						rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
					}

					out << ",\n          \"location\": " << JsonVec3(component->GetRelativeLocation());
					out << ",\n          \"rotationAxis\": " << JsonVec3(glm::normalize(rotationAxis));
					out << ",\n          \"rotationAngle\": " << componentRotation.Angle;
					out << ",\n          \"scale\": " << JsonVec3(component->GetComponentScale());
				},
				[](Achengine::UActorComponent* component, const FJsonValue& componentValue, const std::string&) {
					if (!component)
					{
						return;
					}

					glm::vec3 relativeLocation = component->GetRelativeLocation();
					glm::vec3 relativeScale = component->GetComponentScale();
					Achengine::FRotation relativeRotation = component->GetRelativeRotation();
					glm::vec3 rotationAxis = relativeRotation.RotationAxis;
					if (glm::length(rotationAxis) < 0.0001f)
					{
						rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
					}
					float rotationAngle = relativeRotation.Angle;

					JsonReadVec3Field(componentValue, "location", relativeLocation);
					JsonReadVec3Field(componentValue, "scale", relativeScale);
					JsonReadVec3Field(componentValue, "rotationAxis", rotationAxis);
					JsonReadNumberField(componentValue, "rotationAngle", rotationAngle);

					if (glm::length(rotationAxis) < 0.0001f)
					{
						rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);
					}

					component->SetRelativeLocation(relativeLocation);
					component->SetComponentScale(relativeScale);
					component->SetRelativeRotation(glm::normalize(rotationAxis), rotationAngle);
				}
			},
			{
				"Mesh Settings",
				[](Achengine::UActorComponent* component) {
					return dynamic_cast<Achengine::UMesh*>(component) != nullptr;
				},
				{
					{
						[](Achengine::UActorComponent* component, std::unordered_map<const Achengine::UActorComponent*, std::array<char, 512>>& modelPathBuffers, std::string& status) {
							Achengine::UMesh* mesh = dynamic_cast<Achengine::UMesh*>(component);
							if (!mesh)
							{
								return;
							}

							auto& buffer = modelPathBuffers[component];
							if (buffer[0] == '\0')
							{
								std::strncpy(buffer.data(), mesh->GetModelPath().c_str(), buffer.size() - 1);
								buffer[buffer.size() - 1] = '\0';
							}

							ImGui::InputText("Model Path", buffer.data(), buffer.size());
							if (ImGui::Button("Apply Model Path"))
							{
								if (mesh->ReloadModel(buffer.data()))
								{
									status = "Model reloaded";
								}
								else
								{
									status = "Model reload failed";
								}
							}
						}
					}
				},
				[](const Achengine::UActorComponent* component, std::ostream& out) {
					const Achengine::UMesh* mesh = dynamic_cast<const Achengine::UMesh*>(component);
					if (!mesh)
					{
						return;
					}

					if (!dynamic_cast<const Achengine::UWaterMesh*>(component))
					{
						out << ",\n          \"modelPath\": \"" << JsonEscape(mesh->GetModelPath()) << "\"";
					}
				},
				[](Achengine::UActorComponent* component, const FJsonValue& componentValue, const std::string& mapFilePath) {
					Achengine::UMesh* mesh = dynamic_cast<Achengine::UMesh*>(component);
					if (!mesh)
					{
						return;
					}

					std::string modelPath;
					if (JsonReadStringField(componentValue, "modelPath", modelPath) && !modelPath.empty())
					{
						const std::string resolvedModelPath = ResolvePathRelativeToFile(mapFilePath, modelPath);
						if (FileExists(resolvedModelPath))
						{
							mesh->ReloadModel(resolvedModelPath);
						}
						else if (FileExists(modelPath))
						{
							mesh->ReloadModel(modelPath);
						}
					}
				}
			},
			{
				"Light Settings",
				[](Achengine::UActorComponent* component) {
					return dynamic_cast<Achengine::ULightComponent*>(component) != nullptr;
				},
				{
					{
						[](Achengine::UActorComponent* component, std::unordered_map<const Achengine::UActorComponent*, std::array<char, 512>>&, std::string&) {
							Achengine::ULightComponent* lightComponent = dynamic_cast<Achengine::ULightComponent*>(component);
							if (!lightComponent)
							{
								return;
							}

							Achengine::FLightSource* light = lightComponent->GetLightSource();
							if (!light)
							{
								return;
							}

							ImGui::ColorEdit3("Light Color", &light->color.x);
							ImGui::ColorEdit3("Ambient", &light->ambient.x);
							ImGui::ColorEdit3("Diffuse", &light->diffuse.x);
							ImGui::ColorEdit3("Specular", &light->specular.x);
							ImGui::DragFloat("Constant", &light->constant, 0.01f, 0.0f, 20.0f);
							ImGui::DragFloat("Linear", &light->linear, 0.001f, 0.0f, 10.0f);
							ImGui::DragFloat("Quadratic", &light->quadratic, 0.0001f, 0.0f, 10.0f);
						}
					}
				},
				[](const Achengine::UActorComponent* component, std::ostream& out) {
					const Achengine::ULightComponent* lightComponent = dynamic_cast<const Achengine::ULightComponent*>(component);
					if (!lightComponent)
					{
						return;
					}

					const Achengine::FLightSource* ls = lightComponent->GetLightSource();
					if (!ls)
					{
						return;
					}

					out << ",\n          \"light\": {\n";
					out << "            \"color\": " << JsonVec3(ls->color) << ",\n";
					out << "            \"ambient\": " << JsonVec3(ls->ambient) << ",\n";
					out << "            \"diffuse\": " << JsonVec3(ls->diffuse) << ",\n";
					out << "            \"specular\": " << JsonVec3(ls->specular) << ",\n";
					out << "            \"constant\": " << ls->constant << ",\n";
					out << "            \"linear\": " << ls->linear << ",\n";
					out << "            \"quadratic\": " << ls->quadratic << "\n";
					out << "          }";
				},
				[](Achengine::UActorComponent* component, const FJsonValue& componentValue, const std::string&) {
					Achengine::ULightComponent* lightComponent = dynamic_cast<Achengine::ULightComponent*>(component);
					if (!lightComponent)
					{
						return;
					}

					FJsonValue lightValue;
					if (JsonReadObjectField(componentValue, "light", lightValue) && lightValue.Type == FJsonValue::EType::Object)
					{
						if (Achengine::FLightSource* ls = lightComponent->GetLightSource())
						{
							JsonReadVec3Field(lightValue, "color", ls->color);
							JsonReadVec3Field(lightValue, "ambient", ls->ambient);
							JsonReadVec3Field(lightValue, "diffuse", ls->diffuse);
							JsonReadVec3Field(lightValue, "specular", ls->specular);
							JsonReadNumberField(lightValue, "constant", ls->constant);
							JsonReadNumberField(lightValue, "linear", ls->linear);
							JsonReadNumberField(lightValue, "quadratic", ls->quadratic);
						}
					}
				}
			},
			{
				"Camera Settings",
				[](Achengine::UActorComponent* component) {
					return dynamic_cast<Achengine::UCameraComponent*>(component) != nullptr;
				},
				{
					{
						[](Achengine::UActorComponent* component, std::unordered_map<const Achengine::UActorComponent*, std::array<char, 512>>&, std::string&) {
							Achengine::UCameraComponent* cameraComponent = dynamic_cast<Achengine::UCameraComponent*>(component);
							if (!cameraComponent)
							{
								return;
							}

							float fov = cameraComponent->GetFieldOfView();
							if (ImGui::DragFloat("FOV", &fov, 0.1f, 1.0f, 179.0f))
							{
								cameraComponent->SetFieldOfView(fov);
								cameraComponent->UpdateProjection();
							}

							float nearClip = cameraComponent->GetNearClip();
							float farClip = cameraComponent->GetFarClip();
							bool clipChanged = false;
							if (ImGui::DragFloat("Near Clip", &nearClip, 0.01f, 0.001f, farClip - 0.001f))
							{
								clipChanged = true;
							}
							if (ImGui::DragFloat("Far Clip", &farClip, 1.0f, nearClip + 0.001f, 100000.0f))
							{
								clipChanged = true;
							}

							if (clipChanged)
							{
								if (farClip <= nearClip)
								{
									farClip = nearClip + 0.001f;
								}
								cameraComponent->SetClipPlanes(nearClip, farClip);
								cameraComponent->UpdateProjection();
							}
						}
					}
				},
				[](const Achengine::UActorComponent* component, std::ostream& out) {
					const Achengine::UCameraComponent* cameraComponent = dynamic_cast<const Achengine::UCameraComponent*>(component);
					if (!cameraComponent)
					{
						return;
					}

					out << ",\n          \"camera\": {\n";
					out << "            \"fov\": " << cameraComponent->GetFieldOfView() << ",\n";
					out << "            \"nearClip\": " << cameraComponent->GetNearClip() << ",\n";
					out << "            \"farClip\": " << cameraComponent->GetFarClip() << "\n";
					out << "          }";
				},
				[](Achengine::UActorComponent* component, const FJsonValue& componentValue, const std::string&) {
					Achengine::UCameraComponent* cameraComponent = dynamic_cast<Achengine::UCameraComponent*>(component);
					if (!cameraComponent)
					{
						return;
					}

					FJsonValue cameraValue;
					if (JsonReadObjectField(componentValue, "camera", cameraValue) && cameraValue.Type == FJsonValue::EType::Object)
					{
						float fov = cameraComponent->GetFieldOfView();
						float nearClip = cameraComponent->GetNearClip();
						float farClip = cameraComponent->GetFarClip();

						JsonReadNumberField(cameraValue, "fov", fov);
						JsonReadNumberField(cameraValue, "nearClip", nearClip);
						JsonReadNumberField(cameraValue, "farClip", farClip);

						if (nearClip < 0.001f)
						{
							nearClip = 0.001f;
						}
						if (farClip <= nearClip)
						{
							farClip = nearClip + 1.0f;
						}

						cameraComponent->SetFieldOfView(fov);
						cameraComponent->SetClipPlanes(nearClip, farClip);
						cameraComponent->UpdateProjection();
					}
				}
			}
		};

		return descriptors;
	}

	static void DrawReflectedComponentProperties(
		Achengine::UActorComponent* component,
		std::unordered_map<const Achengine::UActorComponent*, std::array<char, 512>>& modelPathBuffers,
		std::string& status)
	{
		if (!component)
		{
			return;
		}

		for (const FComponentReflectionDescriptor& descriptor : GetComponentReflectionDescriptors())
		{
			if (!descriptor.Matches || !descriptor.Matches(component))
			{
				continue;
			}

			if (descriptor.Properties.empty())
			{
				continue;
			}

			ImGui::Separator();
			ImGui::TextUnformatted(descriptor.SectionName);
			for (const FComponentPropertyDescriptor& property : descriptor.Properties)
			{
				if (property.Draw)
				{
					property.Draw(component, modelPathBuffers, status);
				}
			}
		}
	}

#endif