#include "Achenginepch.h"
#include "Utilities.h"

#include <fstream>
#include <cstdlib>
#include <cassert>
#include <cstdarg>
#include <sys/time.h>

#include "Achengine/Core/Application.h"

using namespace std;

namespace Achengine
{

    float clamp(float min, float value, float max)
    {
        if (value < min)
            return min;
        else if (value > max)
            return max;
        else
            return value;
    }

    string format(const string &fmt, ...)
    {
        va_list args;

        va_start(args, fmt);
        char *buffer = NULL;
        int size = vsnprintf(buffer, 0, fmt.c_str(), args);
        va_end(args);
        buffer = new char[size + 1];
        
        va_start(args, fmt);
        vsnprintf(buffer, size + 1, fmt.c_str(), args);
        va_end(args);
        string str(buffer);
        delete [] buffer;

        va_end(args);
        return str;
    }

    double getTime() 
    {
        return Application::GetTimeSeconds();
    }

    bool isPowerOfTwo(int n) 
    {
        return n > 0 && (n & (n - 1)) == 0;
    }

    int padToPowerOfTwo(int n) 
    {
        int r = 1;
        while (r < n)
            r <<= 1;
        return r;
    }

    string readTextFromFile(const string &filename) 
    {
        ifstream fin(filename.c_str(), ios::binary);
        fin.seekg(0, ios::end);
        int size = fin.tellg();
        fin.seekg(0, ios::beg);
        char *buffer = new char[size + 1];
        fin.read(buffer, size);
        fin.close();
        buffer[size] = '\0';
        string str(buffer);
        delete [] buffer;
        return str;
    }

    float uniformRandomInRange(float min, float max) 
    {
        assert(min < max);
        double n = (double) rand() / (double) RAND_MAX;
        double v = min + n * (max - min);
        return v;
    }
}