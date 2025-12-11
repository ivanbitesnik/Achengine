#pragma once

namespace Achengine
{
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
}