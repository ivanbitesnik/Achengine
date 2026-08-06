#pragma once

#include "Achengine/Actor/Mesh.h"

namespace Achengine
{
    class UWaterMesh : public UMesh
    {
        public:
            UWaterMesh();
            
        protected:
            virtual void SetUniforms() override;
            virtual void GenerateVertexArray() override;
        };
}