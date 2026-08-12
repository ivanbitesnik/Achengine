#ifndef SANDBOX_COMPONENT_REFLECTION_INL
#define SANDBOX_COMPONENT_REFLECTION_INL

	struct FFloatComponentPropertyDescriptor
	{
		const char* JsonKey = "";
		const char* Label = "";
		float DragSpeed = 0.1f;
		float MinValue = 0.0f;
		float MaxValue = 0.0f;
		std::function<float(const Achengine::UActorComponent*)> Getter;
		std::function<void(Achengine::UActorComponent*, float)> Setter;
	};

	struct FVec3ComponentPropertyDescriptor
	{
		const char* JsonKey = "";
		const char* Label = "";
		float DragSpeed = 0.1f;
		float MinValue = 0.0f;
		float MaxValue = 0.0f;
		std::function<glm::vec3(const Achengine::UActorComponent*)> Getter;
		std::function<void(Achengine::UActorComponent*, const glm::vec3&)> Setter;
	};

	static void SerializeFloatComponentProperties(
		std::ostream& out,
		const Achengine::UActorComponent* component,
		const std::vector<FFloatComponentPropertyDescriptor>& properties,
		const char* objectKey = nullptr)
	{
		if (!component || properties.empty())
		{
			return;
		}

		if (!objectKey)
		{
			for (const FFloatComponentPropertyDescriptor& property : properties)
			{
				if (!property.Getter || !property.JsonKey || property.JsonKey[0] == '\0')
				{
					continue;
				}

				out << ",\n          \"" << property.JsonKey << "\": " << property.Getter(component);
			}
			return;
		}

		bool wroteAny = false;
		std::ostringstream objectStream;
		for (const FFloatComponentPropertyDescriptor& property : properties)
		{
			if (!property.Getter || !property.JsonKey || property.JsonKey[0] == '\0')
			{
				continue;
			}

			if (wroteAny)
			{
				objectStream << ",\n";
			}
			wroteAny = true;
			objectStream << "            \"" << property.JsonKey << "\": " << property.Getter(component);
		}

		if (wroteAny)
		{
			out << ",\n          \"" << objectKey << "\": {\n";
			out << objectStream.str() << "\n";
			out << "          }";
		}
	}

	static void SerializeVec3ComponentProperties(
		std::ostream& out,
		const Achengine::UActorComponent* component,
		const std::vector<FVec3ComponentPropertyDescriptor>& properties,
		const char* objectKey = nullptr)
	{
		if (!component || properties.empty())
		{
			return;
		}

		if (!objectKey)
		{
			for (const FVec3ComponentPropertyDescriptor& property : properties)
			{
				if (!property.Getter || !property.JsonKey || property.JsonKey[0] == '\0')
				{
					continue;
				}

				out << ",\n          \"" << property.JsonKey << "\": " << JsonVec3(property.Getter(component));
			}
			return;
		}

		bool wroteAny = false;
		std::ostringstream objectStream;
		for (const FVec3ComponentPropertyDescriptor& property : properties)
		{
			if (!property.Getter || !property.JsonKey || property.JsonKey[0] == '\0')
			{
				continue;
			}

			if (wroteAny)
			{
				objectStream << ",\n";
			}
			wroteAny = true;
			objectStream << "            \"" << property.JsonKey << "\": " << JsonVec3(property.Getter(component));
		}

		if (wroteAny)
		{
			out << ",\n          \"" << objectKey << "\": {\n";
			out << objectStream.str() << "\n";
			out << "          }";
		}
	}

	static void ApplyFloatComponentPropertiesFromJson(
		Achengine::UActorComponent* component,
		const FJsonValue& componentValue,
		const std::vector<FFloatComponentPropertyDescriptor>& properties,
		const char* objectKey = nullptr)
	{
		if (!component || properties.empty())
		{
			return;
		}

		const FJsonValue* source = &componentValue;
		FJsonValue objectValue;
		if (objectKey)
		{
			if (!JsonReadObjectField(componentValue, objectKey, objectValue) || objectValue.Type != FJsonValue::EType::Object)
			{
				return;
			}
			source = &objectValue;
		}

		for (const FFloatComponentPropertyDescriptor& property : properties)
		{
			if (!property.Setter || !property.Getter || !property.JsonKey || property.JsonKey[0] == '\0')
			{
				continue;
			}

			float value = property.Getter(component);
			if (JsonReadNumberField(*source, property.JsonKey, value))
			{
				property.Setter(component, value);
			}
		}
	}

	static void ApplyVec3ComponentPropertiesFromJson(
		Achengine::UActorComponent* component,
		const FJsonValue& componentValue,
		const std::vector<FVec3ComponentPropertyDescriptor>& properties,
		const char* objectKey = nullptr)
	{
		if (!component || properties.empty())
		{
			return;
		}

		const FJsonValue* source = &componentValue;
		FJsonValue objectValue;
		if (objectKey)
		{
			if (!JsonReadObjectField(componentValue, objectKey, objectValue) || objectValue.Type != FJsonValue::EType::Object)
			{
				return;
			}
			source = &objectValue;
		}

		for (const FVec3ComponentPropertyDescriptor& property : properties)
		{
			if (!property.Setter || !property.Getter || !property.JsonKey || property.JsonKey[0] == '\0')
			{
				continue;
			}

			glm::vec3 value = property.Getter(component);
			if (JsonReadVec3Field(*source, property.JsonKey, value))
			{
				property.Setter(component, value);
			}
		}
	}

	static void DrawFloatComponentProperties(
		Achengine::UActorComponent* component,
		const std::vector<FFloatComponentPropertyDescriptor>& properties)
	{
		if (!component)
		{
			return;
		}

		for (const FFloatComponentPropertyDescriptor& property : properties)
		{
			if (!property.Getter || !property.Setter || !property.Label || property.Label[0] == '\0')
			{
				continue;
			}

			float value = property.Getter(component);
			if (ImGui::DragFloat(property.Label, &value, property.DragSpeed, property.MinValue, property.MaxValue))
			{
				property.Setter(component, value);
			}
		}
	}

	static void DrawVec3ComponentProperties(
		Achengine::UActorComponent* component,
		const std::vector<FVec3ComponentPropertyDescriptor>& properties)
	{
		if (!component)
		{
			return;
		}

		for (const FVec3ComponentPropertyDescriptor& property : properties)
		{
			if (!property.Getter || !property.Setter || !property.Label || property.Label[0] == '\0')
			{
				continue;
			}

			glm::vec3 value = property.Getter(component);
			if (ImGui::DragFloat3(property.Label, &value.x, property.DragSpeed, property.MinValue, property.MaxValue))
			{
				property.Setter(component, value);
			}
		}
	}

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
		if (const Achengine::UActorComponent::FRegisteredComponentClass* descriptor = Achengine::UActorComponent::FindRegisteredComponentClassByInstance(component))
		{
			return descriptor->ClassName ? descriptor->ClassName : "UActorComponent";
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
					static const std::vector<FVec3ComponentPropertyDescriptor> vec3Properties = {
						{
							"location", "Comp Location", 0.1f, -100000.0f, 100000.0f,
							[](const Achengine::UActorComponent* c) { return c->GetRelativeLocation(); },
							[](Achengine::UActorComponent* c, const glm::vec3& v) { c->SetRelativeLocation(v); }
						},
						{
							"rotationAxis", "Comp Rot Axis", 0.01f, -1.0f, 1.0f,
							[](const Achengine::UActorComponent* c) {
								glm::vec3 axis = c->GetRelativeRotation().RotationAxis;
								if (glm::length(axis) < 0.0001f)
								{
									axis = glm::vec3(1.0f, 0.0f, 0.0f);
								}
								return glm::normalize(axis);
							},
							[](Achengine::UActorComponent* c, const glm::vec3& value) {
								glm::vec3 axis = value;
								if (glm::length(axis) < 0.0001f)
								{
									axis = glm::vec3(1.0f, 0.0f, 0.0f);
								}
								c->SetRelativeRotation(glm::normalize(axis), c->GetRelativeRotation().Angle);
							}
						},
						{
							"scale", "Comp Scale", 0.05f, 0.01f, 1000.0f,
							[](const Achengine::UActorComponent* c) { return c->GetComponentScale(); },
							[](Achengine::UActorComponent* c, const glm::vec3& v) { c->SetComponentScale(v); }
						}
					};
					static const std::vector<FFloatComponentPropertyDescriptor> floatProperties = {
						{
							"rotationAngle", "Comp Rot Angle", 0.25f, -360.0f, 360.0f,
							[](const Achengine::UActorComponent* c) { return c->GetRelativeRotation().Angle; },
							[](Achengine::UActorComponent* c, float value) {
								glm::vec3 axis = c->GetRelativeRotation().RotationAxis;
								if (glm::length(axis) < 0.0001f)
								{
									axis = glm::vec3(1.0f, 0.0f, 0.0f);
								}
								c->SetRelativeRotation(glm::normalize(axis), value);
							}
						}
					};

					SerializeVec3ComponentProperties(out, component, vec3Properties);
					SerializeFloatComponentProperties(out, component, floatProperties);
				},
				[](Achengine::UActorComponent* component, const FJsonValue& componentValue, const std::string&) {
					static const std::vector<FVec3ComponentPropertyDescriptor> vec3Properties = {
						{
							"location", "Comp Location", 0.1f, -100000.0f, 100000.0f,
							[](const Achengine::UActorComponent* c) { return c->GetRelativeLocation(); },
							[](Achengine::UActorComponent* c, const glm::vec3& v) { c->SetRelativeLocation(v); }
						},
						{
							"rotationAxis", "Comp Rot Axis", 0.01f, -1.0f, 1.0f,
							[](const Achengine::UActorComponent* c) {
								glm::vec3 axis = c->GetRelativeRotation().RotationAxis;
								if (glm::length(axis) < 0.0001f)
								{
									axis = glm::vec3(1.0f, 0.0f, 0.0f);
								}
								return glm::normalize(axis);
							},
							[](Achengine::UActorComponent* c, const glm::vec3& value) {
								glm::vec3 axis = value;
								if (glm::length(axis) < 0.0001f)
								{
									axis = glm::vec3(1.0f, 0.0f, 0.0f);
								}
								c->SetRelativeRotation(glm::normalize(axis), c->GetRelativeRotation().Angle);
							}
						},
						{
							"scale", "Comp Scale", 0.05f, 0.01f, 1000.0f,
							[](const Achengine::UActorComponent* c) { return c->GetComponentScale(); },
							[](Achengine::UActorComponent* c, const glm::vec3& v) { c->SetComponentScale(v); }
						}
					};
					static const std::vector<FFloatComponentPropertyDescriptor> floatProperties = {
						{
							"rotationAngle", "Comp Rot Angle", 0.25f, -360.0f, 360.0f,
							[](const Achengine::UActorComponent* c) { return c->GetRelativeRotation().Angle; },
							[](Achengine::UActorComponent* c, float value) {
								glm::vec3 axis = c->GetRelativeRotation().RotationAxis;
								if (glm::length(axis) < 0.0001f)
								{
									axis = glm::vec3(1.0f, 0.0f, 0.0f);
								}
								c->SetRelativeRotation(glm::normalize(axis), value);
							}
						}
					};

					ApplyVec3ComponentPropertiesFromJson(component, componentValue, vec3Properties);
					ApplyFloatComponentPropertiesFromJson(component, componentValue, floatProperties);
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

							static const std::vector<FVec3ComponentPropertyDescriptor> vec3Properties = {
								{
									"color", "Light Color", 0.01f, 0.0f, 1.0f,
									[](const Achengine::UActorComponent* c) {
										const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
										const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
										return ls ? ls->color : glm::vec3(1.0f);
									},
									[](Achengine::UActorComponent* c, const glm::vec3& v) {
										Achengine::ULightComponent* lc = dynamic_cast<Achengine::ULightComponent*>(c);
										Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
										if (ls) ls->color = v;
									}
								},
								{
									"ambient", "Ambient", 0.01f, 0.0f, 1.0f,
									[](const Achengine::UActorComponent* c) {
										const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
										const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
										return ls ? ls->ambient : glm::vec3(0.0f);
									},
									[](Achengine::UActorComponent* c, const glm::vec3& v) {
										Achengine::ULightComponent* lc = dynamic_cast<Achengine::ULightComponent*>(c);
										Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
										if (ls) ls->ambient = v;
									}
								},
								{
									"diffuse", "Diffuse", 0.01f, 0.0f, 1.0f,
									[](const Achengine::UActorComponent* c) {
										const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
										const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
										return ls ? ls->diffuse : glm::vec3(0.0f);
									},
									[](Achengine::UActorComponent* c, const glm::vec3& v) {
										Achengine::ULightComponent* lc = dynamic_cast<Achengine::ULightComponent*>(c);
										Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
										if (ls) ls->diffuse = v;
									}
								},
								{
									"specular", "Specular", 0.01f, 0.0f, 1.0f,
									[](const Achengine::UActorComponent* c) {
										const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
										const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
										return ls ? ls->specular : glm::vec3(0.0f);
									},
									[](Achengine::UActorComponent* c, const glm::vec3& v) {
										Achengine::ULightComponent* lc = dynamic_cast<Achengine::ULightComponent*>(c);
										Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
										if (ls) ls->specular = v;
									}
								}
							};
							static const std::vector<FFloatComponentPropertyDescriptor> floatProperties = {
								{
									"constant", "Constant", 0.01f, 0.0f, 20.0f,
									[](const Achengine::UActorComponent* c) {
										const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
										const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
										return ls ? ls->constant : 1.0f;
									},
									[](Achengine::UActorComponent* c, float v) {
										Achengine::ULightComponent* lc = dynamic_cast<Achengine::ULightComponent*>(c);
										Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
										if (ls) ls->constant = v;
									}
								},
								{
									"linear", "Linear", 0.001f, 0.0f, 10.0f,
									[](const Achengine::UActorComponent* c) {
										const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
										const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
										return ls ? ls->linear : 0.0f;
									},
									[](Achengine::UActorComponent* c, float v) {
										Achengine::ULightComponent* lc = dynamic_cast<Achengine::ULightComponent*>(c);
										Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
										if (ls) ls->linear = v;
									}
								},
								{
									"quadratic", "Quadratic", 0.0001f, 0.0f, 10.0f,
									[](const Achengine::UActorComponent* c) {
										const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
										const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
										return ls ? ls->quadratic : 0.0f;
									},
									[](Achengine::UActorComponent* c, float v) {
										Achengine::ULightComponent* lc = dynamic_cast<Achengine::ULightComponent*>(c);
										Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
										if (ls) ls->quadratic = v;
									}
								}
							};

							DrawVec3ComponentProperties(component, vec3Properties);
							DrawFloatComponentProperties(component, floatProperties);
						}
					}
				},
				[](const Achengine::UActorComponent* component, std::ostream& out) {
					static const std::vector<FVec3ComponentPropertyDescriptor> vec3Properties = {
						{
							"color", "Light Color", 0.01f, 0.0f, 1.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
								const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								return ls ? ls->color : glm::vec3(1.0f);
							},
							[](Achengine::UActorComponent*, const glm::vec3&) {}
						},
						{
							"ambient", "Ambient", 0.01f, 0.0f, 1.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
								const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								return ls ? ls->ambient : glm::vec3(0.0f);
							},
							[](Achengine::UActorComponent*, const glm::vec3&) {}
						},
						{
							"diffuse", "Diffuse", 0.01f, 0.0f, 1.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
								const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								return ls ? ls->diffuse : glm::vec3(0.0f);
							},
							[](Achengine::UActorComponent*, const glm::vec3&) {}
						},
						{
							"specular", "Specular", 0.01f, 0.0f, 1.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
								const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								return ls ? ls->specular : glm::vec3(0.0f);
							},
							[](Achengine::UActorComponent*, const glm::vec3&) {}
						}
					};
					static const std::vector<FFloatComponentPropertyDescriptor> floatProperties = {
						{
							"constant", "Constant", 0.01f, 0.0f, 20.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
								const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								return ls ? ls->constant : 1.0f;
							},
							[](Achengine::UActorComponent*, float) {}
						},
						{
							"linear", "Linear", 0.001f, 0.0f, 10.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
								const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								return ls ? ls->linear : 0.0f;
							},
							[](Achengine::UActorComponent*, float) {}
						},
						{
							"quadratic", "Quadratic", 0.0001f, 0.0f, 10.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
								const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								return ls ? ls->quadratic : 0.0f;
							},
							[](Achengine::UActorComponent*, float) {}
						}
					};

					SerializeVec3ComponentProperties(out, component, vec3Properties, "light");
					SerializeFloatComponentProperties(out, component, floatProperties, "light");
				},
				[](Achengine::UActorComponent* component, const FJsonValue& componentValue, const std::string&) {
					static const std::vector<FVec3ComponentPropertyDescriptor> vec3Properties = {
						{
							"color", "Light Color", 0.01f, 0.0f, 1.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
								const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								return ls ? ls->color : glm::vec3(1.0f);
							},
							[](Achengine::UActorComponent* c, const glm::vec3& v) {
								Achengine::ULightComponent* lc = dynamic_cast<Achengine::ULightComponent*>(c);
								Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								if (ls) ls->color = v;
							}
						},
						{
							"ambient", "Ambient", 0.01f, 0.0f, 1.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
								const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								return ls ? ls->ambient : glm::vec3(0.0f);
							},
							[](Achengine::UActorComponent* c, const glm::vec3& v) {
								Achengine::ULightComponent* lc = dynamic_cast<Achengine::ULightComponent*>(c);
								Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								if (ls) ls->ambient = v;
							}
						},
						{
							"diffuse", "Diffuse", 0.01f, 0.0f, 1.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
								const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								return ls ? ls->diffuse : glm::vec3(0.0f);
							},
							[](Achengine::UActorComponent* c, const glm::vec3& v) {
								Achengine::ULightComponent* lc = dynamic_cast<Achengine::ULightComponent*>(c);
								Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								if (ls) ls->diffuse = v;
							}
						},
						{
							"specular", "Specular", 0.01f, 0.0f, 1.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
								const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								return ls ? ls->specular : glm::vec3(0.0f);
							},
							[](Achengine::UActorComponent* c, const glm::vec3& v) {
								Achengine::ULightComponent* lc = dynamic_cast<Achengine::ULightComponent*>(c);
								Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								if (ls) ls->specular = v;
							}
						}
					};
					static const std::vector<FFloatComponentPropertyDescriptor> floatProperties = {
						{
							"constant", "Constant", 0.01f, 0.0f, 20.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
								const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								return ls ? ls->constant : 1.0f;
							},
							[](Achengine::UActorComponent* c, float v) {
								Achengine::ULightComponent* lc = dynamic_cast<Achengine::ULightComponent*>(c);
								Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								if (ls) ls->constant = v;
							}
						},
						{
							"linear", "Linear", 0.001f, 0.0f, 10.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
								const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								return ls ? ls->linear : 0.0f;
							},
							[](Achengine::UActorComponent* c, float v) {
								Achengine::ULightComponent* lc = dynamic_cast<Achengine::ULightComponent*>(c);
								Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								if (ls) ls->linear = v;
							}
						},
						{
							"quadratic", "Quadratic", 0.0001f, 0.0f, 10.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::ULightComponent* lc = dynamic_cast<const Achengine::ULightComponent*>(c);
								const Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								return ls ? ls->quadratic : 0.0f;
							},
							[](Achengine::UActorComponent* c, float v) {
								Achengine::ULightComponent* lc = dynamic_cast<Achengine::ULightComponent*>(c);
								Achengine::FLightSource* ls = lc ? lc->GetLightSource() : nullptr;
								if (ls) ls->quadratic = v;
							}
						}
					};

					ApplyVec3ComponentPropertiesFromJson(component, componentValue, vec3Properties, "light");
					ApplyFloatComponentPropertiesFromJson(component, componentValue, floatProperties, "light");
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

							static const std::vector<FFloatComponentPropertyDescriptor> floatProperties = {
								{
									"fov", "FOV", 0.1f, 1.0f, 179.0f,
									[](const Achengine::UActorComponent* c) {
										const Achengine::UCameraComponent* cc = dynamic_cast<const Achengine::UCameraComponent*>(c);
										return cc ? cc->GetFieldOfView() : 90.0f;
									},
									[](Achengine::UActorComponent* c, float v) {
										Achengine::UCameraComponent* cc = dynamic_cast<Achengine::UCameraComponent*>(c);
										if (!cc)
										{
											return;
										}
										cc->SetFieldOfView(v);
										cc->UpdateProjection();
									}
								},
								{
									"nearClip", "Near Clip", 0.01f, 0.001f, 100000.0f,
									[](const Achengine::UActorComponent* c) {
										const Achengine::UCameraComponent* cc = dynamic_cast<const Achengine::UCameraComponent*>(c);
										return cc ? cc->GetNearClip() : 0.1f;
									},
									[](Achengine::UActorComponent* c, float v) {
										Achengine::UCameraComponent* cc = dynamic_cast<Achengine::UCameraComponent*>(c);
										if (!cc)
										{
											return;
										}
										float nearClip = v;
										if (nearClip < 0.001f)
										{
											nearClip = 0.001f;
										}
										float farClip = cc->GetFarClip();
										if (farClip <= nearClip)
										{
											farClip = nearClip + 0.001f;
										}
										cc->SetClipPlanes(nearClip, farClip);
										cc->UpdateProjection();
									}
								},
								{
									"farClip", "Far Clip", 1.0f, 0.002f, 100000.0f,
									[](const Achengine::UActorComponent* c) {
										const Achengine::UCameraComponent* cc = dynamic_cast<const Achengine::UCameraComponent*>(c);
										return cc ? cc->GetFarClip() : 1000.0f;
									},
									[](Achengine::UActorComponent* c, float v) {
										Achengine::UCameraComponent* cc = dynamic_cast<Achengine::UCameraComponent*>(c);
										if (!cc)
										{
											return;
										}
										float nearClip = cc->GetNearClip();
										float farClip = v;
										if (farClip <= nearClip)
										{
											farClip = nearClip + 0.001f;
										}
										cc->SetClipPlanes(nearClip, farClip);
										cc->UpdateProjection();
									}
								}
							};

							DrawFloatComponentProperties(component, floatProperties);
						}
					}
				},
				[](const Achengine::UActorComponent* component, std::ostream& out) {
					static const std::vector<FFloatComponentPropertyDescriptor> floatProperties = {
						{
							"fov", "FOV", 0.1f, 1.0f, 179.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::UCameraComponent* cc = dynamic_cast<const Achengine::UCameraComponent*>(c);
								return cc ? cc->GetFieldOfView() : 90.0f;
							},
							[](Achengine::UActorComponent*, float) {}
						},
						{
							"nearClip", "Near Clip", 0.01f, 0.001f, 100000.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::UCameraComponent* cc = dynamic_cast<const Achengine::UCameraComponent*>(c);
								return cc ? cc->GetNearClip() : 0.1f;
							},
							[](Achengine::UActorComponent*, float) {}
						},
						{
							"farClip", "Far Clip", 1.0f, 0.002f, 100000.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::UCameraComponent* cc = dynamic_cast<const Achengine::UCameraComponent*>(c);
								return cc ? cc->GetFarClip() : 1000.0f;
							},
							[](Achengine::UActorComponent*, float) {}
						}
					};

					SerializeFloatComponentProperties(out, component, floatProperties, "camera");
				},
				[](Achengine::UActorComponent* component, const FJsonValue& componentValue, const std::string&) {
					static const std::vector<FFloatComponentPropertyDescriptor> floatProperties = {
						{
							"fov", "FOV", 0.1f, 1.0f, 179.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::UCameraComponent* cc = dynamic_cast<const Achengine::UCameraComponent*>(c);
								return cc ? cc->GetFieldOfView() : 90.0f;
							},
							[](Achengine::UActorComponent* c, float v) {
								Achengine::UCameraComponent* cc = dynamic_cast<Achengine::UCameraComponent*>(c);
								if (!cc)
								{
									return;
								}
								cc->SetFieldOfView(v);
								cc->UpdateProjection();
							}
						},
						{
							"nearClip", "Near Clip", 0.01f, 0.001f, 100000.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::UCameraComponent* cc = dynamic_cast<const Achengine::UCameraComponent*>(c);
								return cc ? cc->GetNearClip() : 0.1f;
							},
							[](Achengine::UActorComponent* c, float v) {
								Achengine::UCameraComponent* cc = dynamic_cast<Achengine::UCameraComponent*>(c);
								if (!cc)
								{
									return;
								}
								float nearClip = v;
								if (nearClip < 0.001f)
								{
									nearClip = 0.001f;
								}
								float farClip = cc->GetFarClip();
								if (farClip <= nearClip)
								{
									farClip = nearClip + 0.001f;
								}
								cc->SetClipPlanes(nearClip, farClip);
								cc->UpdateProjection();
							}
						},
						{
							"farClip", "Far Clip", 1.0f, 0.002f, 100000.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::UCameraComponent* cc = dynamic_cast<const Achengine::UCameraComponent*>(c);
								return cc ? cc->GetFarClip() : 1000.0f;
							},
							[](Achengine::UActorComponent* c, float v) {
								Achengine::UCameraComponent* cc = dynamic_cast<Achengine::UCameraComponent*>(c);
								if (!cc)
								{
									return;
								}
								float nearClip = cc->GetNearClip();
								float farClip = v;
								if (farClip <= nearClip)
								{
									farClip = nearClip + 0.001f;
								}
								cc->SetClipPlanes(nearClip, farClip);
								cc->UpdateProjection();
							}
						}
					};

					ApplyFloatComponentPropertiesFromJson(component, componentValue, floatProperties, "camera");
				}
			},
			{
				"Movement Settings",
				[](Achengine::UActorComponent* component) {
					return dynamic_cast<Achengine::UMovementComponent*>(component) != nullptr;
				},
				{
					{
						[](Achengine::UActorComponent* component, std::unordered_map<const Achengine::UActorComponent*, std::array<char, 512>>&, std::string&) {
							static const std::vector<FFloatComponentPropertyDescriptor> floatProperties = {
								{
									"acceleration", "Acceleration", 0.1f, 0.0f, 200.0f,
									[](const Achengine::UActorComponent* c) {
										const Achengine::UMovementComponent* mc = dynamic_cast<const Achengine::UMovementComponent*>(c);
										return mc ? mc->GetAcceleration() : 5.0f;
									},
									[](Achengine::UActorComponent* c, float v) {
										Achengine::UMovementComponent* mc = dynamic_cast<Achengine::UMovementComponent*>(c);
										if (mc)
										{
											mc->SetAcceleration(v);
										}
									}
								},
								{
									"maxSpeed", "Max Speed", 0.1f, 0.0f, 200.0f,
									[](const Achengine::UActorComponent* c) {
										const Achengine::UMovementComponent* mc = dynamic_cast<const Achengine::UMovementComponent*>(c);
										return mc ? mc->GetMaxSpeed() : 10.0f;
									},
									[](Achengine::UActorComponent* c, float v) {
										Achengine::UMovementComponent* mc = dynamic_cast<Achengine::UMovementComponent*>(c);
										if (mc)
										{
											mc->SetMaxSpeed(v);
										}
									}
								},
								{
									"jumpImpulse", "Jump Impulse", 0.1f, 0.0f, 200.0f,
									[](const Achengine::UActorComponent* c) {
										const Achengine::UMovementComponent* mc = dynamic_cast<const Achengine::UMovementComponent*>(c);
										return mc ? mc->GetJumpImpulse() : 12.0f;
									},
									[](Achengine::UActorComponent* c, float v) {
										Achengine::UMovementComponent* mc = dynamic_cast<Achengine::UMovementComponent*>(c);
										if (mc)
										{
											mc->SetJumpImpulse(v);
										}
									}
								},
								{
									"groundTraceDistance", "Ground Trace Distance", 0.05f, 0.01f, 50.0f,
									[](const Achengine::UActorComponent* c) {
										const Achengine::UMovementComponent* mc = dynamic_cast<const Achengine::UMovementComponent*>(c);
										return mc ? mc->GetGroundTraceDistance() : 2.5f;
									},
									[](Achengine::UActorComponent* c, float v) {
										Achengine::UMovementComponent* mc = dynamic_cast<Achengine::UMovementComponent*>(c);
										if (mc)
										{
											mc->SetGroundTraceDistance(v);
										}
									}
								}
							};

							DrawFloatComponentProperties(component, floatProperties);
						}
					}
				},
				[](const Achengine::UActorComponent* component, std::ostream& out) {
					static const std::vector<FFloatComponentPropertyDescriptor> floatProperties = {
						{
							"acceleration", "Acceleration", 0.1f, 0.0f, 200.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::UMovementComponent* mc = dynamic_cast<const Achengine::UMovementComponent*>(c);
								return mc ? mc->GetAcceleration() : 5.0f;
							},
							[](Achengine::UActorComponent*, float) {}
						},
						{
							"maxSpeed", "Max Speed", 0.1f, 0.0f, 200.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::UMovementComponent* mc = dynamic_cast<const Achengine::UMovementComponent*>(c);
								return mc ? mc->GetMaxSpeed() : 10.0f;
							},
							[](Achengine::UActorComponent*, float) {}
						},
						{
							"jumpImpulse", "Jump Impulse", 0.1f, 0.0f, 200.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::UMovementComponent* mc = dynamic_cast<const Achengine::UMovementComponent*>(c);
								return mc ? mc->GetJumpImpulse() : 12.0f;
							},
							[](Achengine::UActorComponent*, float) {}
						},
						{
							"groundTraceDistance", "Ground Trace Distance", 0.05f, 0.01f, 50.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::UMovementComponent* mc = dynamic_cast<const Achengine::UMovementComponent*>(c);
								return mc ? mc->GetGroundTraceDistance() : 2.5f;
							},
							[](Achengine::UActorComponent*, float) {}
						}
					};

					SerializeFloatComponentProperties(out, component, floatProperties, "movement");
				},
				[](Achengine::UActorComponent* component, const FJsonValue& componentValue, const std::string&) {
					static const std::vector<FFloatComponentPropertyDescriptor> floatProperties = {
						{
							"acceleration", "Acceleration", 0.1f, 0.0f, 200.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::UMovementComponent* mc = dynamic_cast<const Achengine::UMovementComponent*>(c);
								return mc ? mc->GetAcceleration() : 5.0f;
							},
							[](Achengine::UActorComponent* c, float v) {
								Achengine::UMovementComponent* mc = dynamic_cast<Achengine::UMovementComponent*>(c);
								if (mc)
								{
									mc->SetAcceleration(v);
								}
							}
						},
						{
							"maxSpeed", "Max Speed", 0.1f, 0.0f, 200.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::UMovementComponent* mc = dynamic_cast<const Achengine::UMovementComponent*>(c);
								return mc ? mc->GetMaxSpeed() : 10.0f;
							},
							[](Achengine::UActorComponent* c, float v) {
								Achengine::UMovementComponent* mc = dynamic_cast<Achengine::UMovementComponent*>(c);
								if (mc)
								{
									mc->SetMaxSpeed(v);
								}
							}
						},
						{
							"jumpImpulse", "Jump Impulse", 0.1f, 0.0f, 200.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::UMovementComponent* mc = dynamic_cast<const Achengine::UMovementComponent*>(c);
								return mc ? mc->GetJumpImpulse() : 12.0f;
							},
							[](Achengine::UActorComponent* c, float v) {
								Achengine::UMovementComponent* mc = dynamic_cast<Achengine::UMovementComponent*>(c);
								if (mc)
								{
									mc->SetJumpImpulse(v);
								}
							}
						},
						{
							"groundTraceDistance", "Ground Trace Distance", 0.05f, 0.01f, 50.0f,
							[](const Achengine::UActorComponent* c) {
								const Achengine::UMovementComponent* mc = dynamic_cast<const Achengine::UMovementComponent*>(c);
								return mc ? mc->GetGroundTraceDistance() : 2.5f;
							},
							[](Achengine::UActorComponent* c, float v) {
								Achengine::UMovementComponent* mc = dynamic_cast<Achengine::UMovementComponent*>(c);
								if (mc)
								{
									mc->SetGroundTraceDistance(v);
								}
							}
						}
					};

					ApplyFloatComponentPropertiesFromJson(component, componentValue, floatProperties, "movement");
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