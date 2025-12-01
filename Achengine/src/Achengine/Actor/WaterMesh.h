#pragma once

#include "Achengine/Actor/StaticMesh.h"

namespace Achengine
{
    struct FSineFunction
    {
        public:
            FSineFunction() {};
            FSineFunction(float amplitude, float frequency, float phase) :
                amplitude(amplitude), frequency(frequency), phase(phase) {}
            float amplitude = 0.5f;
            float frequency = 2.0f;
            float phase = 1.0f;
    };

    class UWaterMesh : public UStaticMesh
    {
        public:
            UWaterMesh();

            void AddSineFunction(FSineFunction* NewSineFunction);
            FSineFunction* GetSineFunction() const { return m_SineFunction; }

            virtual void DrawMesh(RendererStorage* RenderData) override;

        private:
            FSineFunction* m_SineFunction = nullptr;
    };
}