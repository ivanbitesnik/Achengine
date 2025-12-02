#pragma once

#include "Achengine/Actor/StaticMesh.h"

namespace Achengine
{
    class UWaterMesh : public UStaticMesh
    {
        public:
            UWaterMesh();

            virtual void DrawMesh(RendererStorage* RenderData) override;
    };
}