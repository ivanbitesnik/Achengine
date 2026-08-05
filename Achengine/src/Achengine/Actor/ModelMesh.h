#pragma once

#include "Achengine/Actor/Mesh.h"
#include "Achengine/Core/Utilities.h"

namespace Achengine
{
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

    class UModelMesh : public UMesh
    {
        public:
            UModelMesh(const std::string& modelPath, const std::string& shaderPath = "");
            virtual ~UModelMesh();

            bool ReloadModel(const std::string& modelPath);

            void SetTexture(Texture* NewTexture) { m_Texture = NewTexture; }
            Texture* GetTexture() const { return m_Texture; }
            void SetSpecular(Texture* NewSpecular) { m_Specular = NewSpecular; }
            Texture* GetSpecular() const { return m_Specular; }
            void SetNormal(Texture* NewNormal) { m_Normal = NewNormal; }
            Texture* GetNormal() const { return m_Normal; }

            virtual FMeshBounds GetBounds() const override { return m_Bounds; }
            virtual void DrawGeometry() override;

        protected:
            virtual void SetUniforms() override;
            virtual void GenerateVertexArray() override;

        private:
            void CleanupSubmeshTextures();
            Texture* TryLoadTexturePath(const std::string& texturePath, bool& outWasLoaded);
            bool LoadFromFile(const std::string& modelPath);

            std::vector<FImportedSubMesh> m_SubMeshes;
            std::string m_ModelDirectory;
            Texture* m_Normal = nullptr;
    };
}
