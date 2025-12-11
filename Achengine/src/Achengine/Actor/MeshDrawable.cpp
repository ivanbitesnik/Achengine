#include "Achenginepch.h"
#include "Achengine/Actor/MeshDrawable.h"

#include "Achengine/Core/Utilities.h"
#include "Achengine/Renderer/Renderer.h"

#include <glad/glad.h>

namespace Achengine
{
    UMeshDrawable::UMeshDrawable(const std::string& Id,
            const std::vector<glm::vec3>& vertices,
            const std::vector<glm::vec3>& normals,
            const std::vector<std::pair<float, float>>& texCoords,
            const std::vector<uint32_t>& indices,
            BufferLayout Layout) :
        DrawableId(Id),
        vertices(vertices),
        normals(normals),
        texCoords(texCoords),
        indices(indices),
        Layout(Layout)
    {
        WorldActorCache::Get()->AddDrawableToCache(Id, this);
        if (!hasNormals())
        {
            generateNormals();
        }
        generateVbos();
    }

    void UMeshDrawable::generateNormals()
    {
        normals = std::vector<glm::vec3>(vertices.size());
        for (int i = 0; i < indices.size(); i += 3) {
            uint32_t tri[3] = { indices[i], indices[i + 1], indices[i + 2] };
            glm::vec3 a = vertices[tri[0]], 
            b = vertices[tri[1]],
            c = vertices[tri[2]];
            glm::vec3 ab = b - a, ac = c - a;
            glm::vec3 n = glm::normalize(glm::cross(ab, ac));
            for (int j = 0; j < 3; ++j)
            normals[tri[j]] = normals[tri[j]] + n;
        }

        for (int i = 0; i < vertices.size(); ++i)
            normals[i] = glm::normalize(normals[i]);
    }

    void UMeshDrawable::generateVbos()
    {
        std::vector<float> vertexArray;
        for (int i = 0; i < vertices.size(); ++i)
        {
            vertexArray.push_back(vertices[i].x);
            vertexArray.push_back(vertices[i].y);
            vertexArray.push_back(vertices[i].z);
            if (hasNormals())
            {
                vertexArray.push_back(normals[i].x);
                vertexArray.push_back(normals[i].y);
                vertexArray.push_back(normals[i].z);
            }
            if (hasTexCoords())
            {
                vertexArray.push_back(texCoords[i].first);
                vertexArray.push_back(texCoords[i].second);
            }
        }

        Renderer::AddVertexArray(DrawableId, vertexArray, Layout);
        if (hasIndices())
        {
            Renderer::AddIndexBufferToArray(DrawableId, indices);
        }
    }

    int UMeshDrawable::getNumVertices() const {
        return vertices.size();
    }

    std::vector<glm::vec3>& UMeshDrawable::getVertices() {
        return vertices;
    }

    bool UMeshDrawable::hasNormals() const {
        return normals.size() != 0;
    }

    std::vector<glm::vec3>& UMeshDrawable::getNormals() {
        return normals;
    }

    bool UMeshDrawable::hasTexCoords() const {
        return texCoords.size() != 0;
    }

    std::vector<std::pair<float, float>>& UMeshDrawable::getTexCoords() {
        return texCoords;
    }

    int UMeshDrawable::getNumTriangles() const {
        return indices.size() / 3;
    }

    std::vector<uint32_t>& UMeshDrawable::getIndices() {
        return indices;
    }

    bool UMeshDrawable::hasIndices() const
    {
        return indices.size() != 0;
    }

    void UMeshDrawable::DrawMesh(UMesh* Mesh)
    {
        Renderer::DrawMesh(Mesh, DrawableId);
    }
}