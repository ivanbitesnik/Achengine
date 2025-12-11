#pragma once

#include "Achengine/Core/Core.h"
#include "Achengine/Renderer/Buffer.h"

#include <string>

namespace Achengine
{
    class UMesh;

    class UMeshDrawable
    {
        public:
            UMeshDrawable(const std::string& Id,
            const std::vector<glm::vec3>& vertices,
            const std::vector<glm::vec3>& normals,
            const std::vector<std::pair<float, float>>& texCoords,
            const std::vector<uint32_t>& indices,
            BufferLayout Layout);

            void generateNormals();
            void generateVbos();

            int getNumVertices() const;
            std::vector<glm::vec3>& getVertices();

            bool hasNormals() const;
            std::vector<glm::vec3>& getNormals();

            bool hasTexCoords() const;
            std::vector<std::pair<float, float>>& getTexCoords();

            int getNumTriangles() const;
            bool hasIndices() const;
            std::vector<uint32_t>& getIndices();

            void DrawMesh(UMesh* Mesh);
        private:
            std::vector<glm::vec3> vertices;
            std::vector<glm::vec3> normals;
            std::vector<std::pair<float, float>> texCoords;
            std::vector<uint32_t> indices;

            BufferLayout Layout;

            std::string DrawableId;
    };
}