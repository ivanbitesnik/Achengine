#pragma once

#include "Achengine/Actor/StaticMesh.h"

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