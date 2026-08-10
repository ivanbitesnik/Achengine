#include "Achenginepch.h"
#include "Achengine/Actor/Mesh.h"

#include "Achengine/Core/Utilities.h"
#include "Achengine/Renderer/Renderer.h"
#include "Achengine/Renderer/Texture.h"

#include <glad/glad.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <limits>

#ifdef ACHENGINE_ENABLE_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#endif

namespace Achengine
{
    namespace
    {
        std::string GetDirectoryFromPath(const std::string& filePath)
        {
            const size_t slash = filePath.find_last_of("/\\");
            if (slash == std::string::npos)
            {
                return "";
            }

            return filePath.substr(0, slash);
        }

        bool IsAbsolutePath(const std::string& path)
        {
            if (path.empty())
            {
                return false;
            }

            if (path[0] == '/' || path[0] == '\\')
            {
                return true;
            }

            if (path.size() > 1 && path[1] == ':')
            {
                return true;
            }

            return false;
        }

        bool FileExists(const std::string& path)
        {
            std::ifstream stream(path);
            return stream.good();
        }

        std::string NormalizePath(std::string path)
        {
            std::replace(path.begin(), path.end(), '\\', '/');
            while (path.find("//") != std::string::npos)
            {
                path.replace(path.find("//"), 2, "/");
            }

            return path;
        }

        std::string GetFileName(const std::string& path)
        {
            const std::string normalized = NormalizePath(path);
            const size_t slash = normalized.find_last_of('/');
            if (slash == std::string::npos)
            {
                return normalized;
            }

            return normalized.substr(slash + 1);
        }
    }

    UMesh::UMesh(const std::string& modelPath, const std::string& shaderPath)
    {
#ifdef ACHENGINE_PLATFORM_LINUX
        m_ShaderPath = shaderPath.empty() ? "/home/acheto/Desktop/projects/Achengine/Sandbox/assets/shaders/ModelPBR.glsl" : shaderPath;
#else
        m_ShaderPath = shaderPath.empty() ? "assets/shaders/ModelPBR.glsl" : shaderPath;
#endif

        if (LoadFromFile(modelPath))
        {
            Initialize();
        }
        else
        {
            ACHENGINE_CORE_ERROR("Failed to load model: {0}", modelPath);
        }
    }

    UMesh::~UMesh()
    {
        CleanupSubmeshTextures();
    }

    bool UMesh::ReloadModel(const std::string& modelPath)
    {
        if (!LoadFromFile(modelPath))
        {
            return false;
        }

        m_ModelPath = modelPath;
        GenerateVertexArray();
        SetUniforms();
        return true;
    }

    void UMesh::SetUniforms()
    {
        const std::string& ShaderName = GetObjectNameFromFilePath(m_ShaderPath);

        Renderer::SetShaderUniform(ShaderName, "u_Material.diffuse", 0);
        Renderer::SetShaderUniform(ShaderName, "u_Material.specular", 1);
        Renderer::SetShaderUniform(ShaderName, "u_Material.normal", 2);
        Renderer::SetShaderUniform(ShaderName, "u_Material.roughness", 0.5f);
        Renderer::SetShaderUniform(ShaderName, "u_Material.metallic", 0.0f);
        Renderer::SetShaderUniform(ShaderName, "u_Material.ao", 1.0f);
        Renderer::SetShaderUniform(ShaderName, "u_HasDiffuseMap", 0);
        Renderer::SetShaderUniform(ShaderName, "u_HasSpecularMap", 0);
        Renderer::SetShaderUniform(ShaderName, "u_HasNormalMap", 0);
    }

    void UMesh::GenerateVertexArray()
    {
        for (size_t subMeshIndex = 0; subMeshIndex < m_SubMeshes.size(); ++subMeshIndex)
        {
            FImportedSubMesh& subMesh = m_SubMeshes[subMeshIndex];
            subMesh.VertexArrayName = format("%s_sub_%llu", GetVertexArrayName().c_str(), (unsigned long long)subMeshIndex);

            std::vector<float> packedVertices;
            packedVertices.reserve(subMesh.Vertices.size() * 14);
            for (size_t i = 0; i < subMesh.Vertices.size(); ++i)
            {
                packedVertices.push_back(subMesh.Vertices[i].X);
                packedVertices.push_back(subMesh.Vertices[i].Y);
                packedVertices.push_back(subMesh.Vertices[i].Z);

                packedVertices.push_back(subMesh.Normals[i].X);
                packedVertices.push_back(subMesh.Normals[i].Y);
                packedVertices.push_back(subMesh.Normals[i].Z);

                packedVertices.push_back(subMesh.TexCoords[i].first);
                packedVertices.push_back(subMesh.TexCoords[i].second);

                packedVertices.push_back(subMesh.Tangents[i].X);
                packedVertices.push_back(subMesh.Tangents[i].Y);
                packedVertices.push_back(subMesh.Tangents[i].Z);

                packedVertices.push_back(subMesh.Bitangents[i].X);
                packedVertices.push_back(subMesh.Bitangents[i].Y);
                packedVertices.push_back(subMesh.Bitangents[i].Z);
            }

            BufferLayout layout = {
                { ShaderDataType::Float3, "a_Position" },
                { ShaderDataType::Float3, "a_Normal" },
                { ShaderDataType::Float2, "a_TexCoords" },
                { ShaderDataType::Float3, "a_Tangent" },
                { ShaderDataType::Float3, "a_Bitangent" }
            };

            Renderer::AddVertexArray(subMesh.VertexArrayName, packedVertices, layout);
            Renderer::AddIndexBufferToArray(subMesh.VertexArrayName, subMesh.Indices);
        }
    }

    void UMesh::CleanupSubmeshTextures()
    {
        for (FImportedSubMesh& subMesh : m_SubMeshes)
        {
            if (subMesh.OwnsDiffuseTexture && subMesh.DiffuseTexture)
            {
                delete subMesh.DiffuseTexture;
                subMesh.DiffuseTexture = nullptr;
            }
            if (subMesh.OwnsSpecularTexture && subMesh.SpecularTexture)
            {
                delete subMesh.SpecularTexture;
                subMesh.SpecularTexture = nullptr;
            }
            if (subMesh.OwnsNormalTexture && subMesh.NormalTexture)
            {
                delete subMesh.NormalTexture;
                subMesh.NormalTexture = nullptr;
            }
        }
    }

    Texture* UMesh::TryLoadTexturePath(const std::string& texturePath, bool& outWasLoaded)
    {
        outWasLoaded = false;

        if (texturePath.empty())
        {
            return nullptr;
        }

        std::string normalized = NormalizePath(texturePath);
        if (IsAbsolutePath(normalized) && FileExists(normalized))
        {
            outWasLoaded = true;
            return Texture2D::Create(normalized);
        }

        while (normalized.rfind("./", 0) == 0)
        {
            normalized = normalized.substr(2);
        }

        std::vector<std::string> candidates;
        if (!m_ModelDirectory.empty())
        {
            candidates.push_back(NormalizePath(m_ModelDirectory + "/" + normalized));

            const std::string fileName = GetFileName(normalized);
            candidates.push_back(NormalizePath(m_ModelDirectory + "/" + fileName));
            candidates.push_back(NormalizePath(m_ModelDirectory + "/textures/" + fileName));
            candidates.push_back(NormalizePath(m_ModelDirectory + "/Textures/" + fileName));
        }

        for (const std::string& candidate : candidates)
        {
            if (FileExists(candidate))
            {
                outWasLoaded = true;
                return Texture2D::Create(candidate);
            }
        }

        return nullptr;
    }

    bool UMesh::LoadFromFile(const std::string& modelPath)
    {
#ifdef ACHENGINE_ENABLE_ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(modelPath,
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_GenSmoothNormals |
            aiProcess_CalcTangentSpace |
            aiProcess_FlipUVs);

        if (!scene || !scene->mRootNode)
        {
            ACHENGINE_CORE_ERROR("Assimp failed loading {0}: {1}", modelPath, importer.GetErrorString());
            return false;
        }

        m_ModelDirectory = GetDirectoryFromPath(modelPath);
        CleanupSubmeshTextures();
        m_SubMeshes.clear();
        m_Bounds.IsValid = false;
        glm::vec3 minBounds(std::numeric_limits<float>::max());
        glm::vec3 maxBounds(-std::numeric_limits<float>::max());
        bool hasVertices = false;

        for (unsigned int meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
        {
            const aiMesh* mesh = scene->mMeshes[meshIndex];
            FImportedSubMesh subMesh;

            for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
            {
                const aiVector3D& position = mesh->mVertices[i];
                subMesh.Vertices.push_back(Vector3(position.x, position.y, position.z));
                hasVertices = true;
                minBounds.x = std::min(minBounds.x, position.x);
                minBounds.y = std::min(minBounds.y, position.y);
                minBounds.z = std::min(minBounds.z, position.z);
                maxBounds.x = std::max(maxBounds.x, position.x);
                maxBounds.y = std::max(maxBounds.y, position.y);
                maxBounds.z = std::max(maxBounds.z, position.z);

                if (mesh->HasNormals())
                {
                    const aiVector3D& normal = mesh->mNormals[i];
                    subMesh.Normals.push_back(Vector3(normal.x, normal.y, normal.z));
                }
                else
                {
                    subMesh.Normals.push_back(Vector3(0.0f, 1.0f, 0.0f));
                }

                if (mesh->HasTextureCoords(0))
                {
                    const aiVector3D& tex = mesh->mTextureCoords[0][i];
                    subMesh.TexCoords.push_back({ tex.x, tex.y });
                }
                else
                {
                    subMesh.TexCoords.push_back({ 0.0f, 0.0f });
                }

                if (mesh->HasTangentsAndBitangents())
                {
                    const aiVector3D& tangent = mesh->mTangents[i];
                    const aiVector3D& bitangent = mesh->mBitangents[i];
                    subMesh.Tangents.push_back(Vector3(tangent.x, tangent.y, tangent.z));
                    subMesh.Bitangents.push_back(Vector3(bitangent.x, bitangent.y, bitangent.z));
                }
                else
                {
                    subMesh.Tangents.push_back(Vector3(1.0f, 0.0f, 0.0f));
                    subMesh.Bitangents.push_back(Vector3(0.0f, 1.0f, 0.0f));
                }
            }

            for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
            {
                const aiFace& face = mesh->mFaces[f];
                for (unsigned int i = 0; i < face.mNumIndices; ++i)
                {
                    subMesh.Indices.push_back(face.mIndices[i]);
                }
            }

            if (scene->HasMaterials() && mesh->mMaterialIndex < scene->mNumMaterials)
            {
                aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

                float shininess = 64.0f;
                if (material->Get(AI_MATKEY_SHININESS, shininess) == aiReturn_SUCCESS && shininess > 1.0f)
                {
                    subMesh.Shininess = shininess;
                    const float roughnessFromShininess = sqrtf(2.0f / (shininess + 2.0f));
                    subMesh.Roughness = clamp(0.05f, roughnessFromShininess, 1.0f);
                }

                aiString path;
                if (material->GetTexture(aiTextureType_BASE_COLOR, 0, &path) == aiReturn_SUCCESS ||
                    material->GetTexture(aiTextureType_DIFFUSE, 0, &path) == aiReturn_SUCCESS)
                {
                    std::string diffusePath = path.C_Str();
                    subMesh.DiffuseTexture = TryLoadTexturePath(diffusePath, subMesh.OwnsDiffuseTexture);
                }
                if (material->GetTexture(aiTextureType_SPECULAR, 0, &path) == aiReturn_SUCCESS ||
                    material->GetTexture(aiTextureType_METALNESS, 0, &path) == aiReturn_SUCCESS)
                {
                    std::string specularPath = path.C_Str();
                    subMesh.SpecularTexture = TryLoadTexturePath(specularPath, subMesh.OwnsSpecularTexture);
                }
                if (material->GetTexture(aiTextureType_NORMALS, 0, &path) == aiReturn_SUCCESS ||
                    material->GetTexture(aiTextureType_HEIGHT, 0, &path) == aiReturn_SUCCESS)
                {
                    std::string normalPath = path.C_Str();
                    subMesh.NormalTexture = TryLoadTexturePath(normalPath, subMesh.OwnsNormalTexture);
                }
            }

            if (!subMesh.Vertices.empty() && !subMesh.Indices.empty())
            {
                m_SubMeshes.push_back(subMesh);
            }
        }

        if (m_SubMeshes.empty())
        {
            ACHENGINE_CORE_ERROR("Model {0} has no drawable mesh data", modelPath);
            return false;
        }

        if (hasVertices)
        {
            m_Bounds.Center = (minBounds + maxBounds) * 0.5f;
            m_Bounds.Extents = (maxBounds - minBounds) * 0.5f;
            m_Bounds.SphereRadius = glm::length(m_Bounds.Extents);
            m_Bounds.IsValid = true;
        }

        m_ModelPath = modelPath;
        return true;
#else
        (void)modelPath;
        ACHENGINE_CORE_ERROR("Model loading requires Assimp. Regenerate with: premake5 gmake");
        return false;
#endif
    }

    void UMesh::Initialize()
    {
        Renderer::AddShader(m_ShaderPath);
        m_VertexArrayName = format("%s_%llu", GetShaderName().c_str(), (unsigned long long)(uintptr_t)this);

        if (!Renderer::GetVertexArray(m_VertexArrayName))
        {
            GenerateVertexArray();
            SetUniforms();
        }
    }

    std::string UMesh::GetShaderName() const
    {
        return GetObjectNameFromFilePath(m_ShaderPath);
    }

    uint64_t UMesh::GetBatchSortKey() const
    {
        uint64_t key = std::hash<std::string>{}(m_ModelPath);

        auto hashCombine = [](uint64_t seed, uint64_t value)
        {
            return seed ^ (value + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
        };

        key = hashCombine(key, static_cast<uint64_t>(reinterpret_cast<uintptr_t>(m_Texture)));
        key = hashCombine(key, static_cast<uint64_t>(reinterpret_cast<uintptr_t>(m_Specular)));
        key = hashCombine(key, static_cast<uint64_t>(reinterpret_cast<uintptr_t>(m_Normal)));

        return key;
    }

    void UMesh::DrawMesh()
    {
        Renderer::DrawMesh(this);
    }

    void UMesh::DrawGeometry()
    {
        if (!m_SubMeshes.empty())
        {
            const std::string shaderName = GetShaderName();
            for (FImportedSubMesh& subMesh : m_SubMeshes)
            {
                Texture* diffuseTexture = m_Texture ? m_Texture : subMesh.DiffuseTexture;
                Texture* specularTexture = m_Specular ? m_Specular : subMesh.SpecularTexture;
                Texture* normalTexture = m_Normal ? m_Normal : subMesh.NormalTexture;

                Renderer::SetShaderUniform(shaderName, "u_HasDiffuseMap", diffuseTexture ? 1 : 0);
                Renderer::SetShaderUniform(shaderName, "u_HasSpecularMap", specularTexture ? 1 : 0);
                Renderer::SetShaderUniform(shaderName, "u_HasNormalMap", normalTexture ? 1 : 0);
                Renderer::SetShaderUniform(shaderName, "u_Material.roughness", subMesh.Roughness);
                Renderer::SetShaderUniform(shaderName, "u_Material.metallic", subMesh.Metallic);
                Renderer::SetShaderUniform(shaderName, "u_Material.ao", subMesh.AO);

                if (diffuseTexture)
                {
                    diffuseTexture->Bind(0);
                }

                if (specularTexture)
                {
                    specularTexture->Bind(1);
                }

                if (normalTexture)
                {
                    normalTexture->Bind(2);
                }

                Renderer::DrawVertexArray(subMesh.VertexArrayName);
            }
            return;
        }

        Renderer::DrawVertexArray(GetVertexArrayName());
    }
}