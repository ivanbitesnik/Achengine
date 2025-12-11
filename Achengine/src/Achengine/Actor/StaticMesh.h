#pragma once

#include "Achengine/Actor/Mesh.h"

namespace Achengine
{
    class UStaticMesh : public UMesh
    {
        // TODO: Remove dependency
        friend class ULightMesh;
        public:
            UStaticMesh();

            void SetMaterial(FMeshMaterial* NewMaterial) { m_Material = NewMaterial; }
            FMeshMaterial* GetMaterial() const { return m_Material; }

            void SetTexture(Texture* NewTexture) { m_Texture = NewTexture; }
            Texture* GetTexture() const { return m_Texture; }
            void SetSpecular(Texture* NewSpecular) { m_Specular = NewSpecular; }
            Texture* GetSpecular() const { return m_Specular; }

        protected:
            virtual void SetUniforms() override;
            virtual void GenerateVertexArray() override;
    };
}