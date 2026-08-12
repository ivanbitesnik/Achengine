#pragma once

namespace Achengine
{
    class ACHENGINE_API GameGlobals
    {
        public:
            float Gravity = 9.81f;
            float CollisionOctreeSize = 10000.0f;
            float CollisionOctreeMinNodeSize = 1.0f;

            static void SetActive(GameGlobals* globals)
            {
                ActiveGlobalsStorage() = globals;
            }

            static GameGlobals* GetActive()
            {
                return ActiveGlobalsStorage();
            }

            static const GameGlobals* GetActiveConst()
            {
                return ActiveGlobalsStorage();
            }

        private:
            static GameGlobals*& ActiveGlobalsStorage()
            {
                static GameGlobals* s_ActiveGlobals = nullptr;
                return s_ActiveGlobals;
            }
    };
}