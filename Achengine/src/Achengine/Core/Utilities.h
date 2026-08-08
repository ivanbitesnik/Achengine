#pragma once

namespace Achengine
{
    struct Vector3
    {
        Vector3() {}
        Vector3(float X, float Y, float Z) : X(X), Y(Y), Z(Z) {} 
        float X;
        float Y;
        float Z;

        operator glm::vec3() const { return glm::vec3(X, Y, Z); }
        Vector3& operator=(const glm::vec3& other)
        {
            X = other.x;
            Y = other.y;
            Z = other.z;
            return *this;
        }
    };

    float clamp(float min, float value, float max);
    std::string format(const std::string &fmt, ...);
    double getTime();
    bool isPowerOfTwo(int n);
    int padToPowerOfTwo(int n);
    std::string readTextFromFile(const std::string &filename);
    float uniformRandomInRange(float min, float max);
    std::string GetObjectNameFromFilePath(const std::string& filePath);
    std::vector<float> FlattenVector(const std::vector<glm::vec3>& vector);
    std::vector<float> FlattenVector(const std::vector<std::pair<float, float>>& vector);

    struct FBounds
    {
        glm::vec3 Center = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 Extents = glm::vec3(0.0f, 0.0f, 0.0f);
        float SphereRadius = 0.0f;
        bool IsValid = false;
    };
    
    struct FRotation
    {
        public:
            FRotation() {}
            FRotation(const glm::vec3& RotationAxis, float Angle) : RotationAxis(RotationAxis), Angle(Angle) {}
            glm::vec3 RotationAxis = {1.0f, 1.0f, 1.0f};
            float Angle = 0.0f;
    };
}