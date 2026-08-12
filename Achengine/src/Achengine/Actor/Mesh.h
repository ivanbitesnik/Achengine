#pragma once

#include "Achengine/Actor/ActorComponent.h"
#include "Achengine/Core/Utilities.h"
#include "Achengine/Renderer/Buffer.h"

namespace Achengine
{
    class Texture;

    struct FImportedSubMesh
    {
        std::vector<Vector3> Vertices;
        std::vector<Vector3> Normals;
        std::vector<std::pair<float, float>> TexCoords;
        std::vector<Vector3> Tangents;
        std::vector<Vector3> Bitangents;
        std::vector<uint32_t> Indices;

        Texture* DiffuseTexture = nullptr;
        Texture* SpecularTexture = nullptr;
        Texture* NormalTexture = nullptr;
        bool OwnsDiffuseTexture = false;
        bool OwnsSpecularTexture = false;
        bool OwnsNormalTexture = false;
        float Shininess = 64.0f;
        float Roughness = 0.5f;
        float Metallic = 0.0f;
        float AO = 1.0f;
        std::string VertexArrayName;
    };

    struct FMeshMaterial
    {
        public:
            FMeshMaterial(const glm::vec3& color, const glm::vec3& ambient, const glm::vec3& diffuse, const glm::vec3& specular, const float shininess) :
                color(color), ambient(ambient), diffuse(diffuse), specular(specular), shininess(shininess) {}
            glm::vec3 color = glm::vec3(1.0f, 0.5f, 0.31f);
            glm::vec3 ambient = glm::vec3(1.0f, 0.5f, 0.31f);
            glm::vec3 diffuse = glm::vec3(1.0f, 0.5f, 0.31f);
            glm::vec3 specular = glm::vec3(0.5f, 0.5f, 0.5f);
            float shininess = 32.0f;
    };

    class UMesh : public UActorComponent
    {
        public:
            UMesh();
            UMesh(const std::string& modelPath, const std::string& shaderPath = "");
            virtual ~UMesh();

            std::string GetShaderName() const;
            const std::string& GetVertexArrayName() const { return m_VertexArrayName; }
            const std::string& GetModelPath() const { return m_ModelPath; }

            const FMeshMaterial* GetMaterial() const { return m_Material; }
            const Texture* GetTexture() const { return m_Texture; }
            const Texture* GetSpecular() const { return m_Specular; }
            void SetTexture(Texture* NewTexture) { m_Texture = NewTexture; }
            void SetSpecular(Texture* NewSpecular) { m_Specular = NewSpecular; }
            void SetNormal(Texture* NewNormal) { m_Normal = NewNormal; }
            Texture* GetNormal() const { return m_Normal; }
            bool ReloadModel(const std::string& modelPath);
            uint64_t GetBatchSortKey() const;
            
            virtual void DrawMesh();
            virtual void DrawGeometry();
        protected:
            void Initialize();
            void CleanupSubmeshTextures();
            Texture* TryLoadTexturePath(const std::string& texturePath, bool& outWasLoaded);
            bool LoadFromFile(const std::string& modelPath);

            virtual void SetUniforms();
            virtual void GenerateVertexArray();

            std::string m_ShaderPath;
            std::string m_VertexArrayName;
            std::string m_ModelPath;
            std::string m_ModelDirectory;

            FMeshMaterial* m_Material = nullptr;
            Texture* m_Texture = nullptr;
            Texture* m_Specular = nullptr;
            Texture* m_Normal = nullptr;
            std::vector<FImportedSubMesh> m_SubMeshes;
    };
}