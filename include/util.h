#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>
#include <sstream>

using namespace std;

// Split a line on a delimiter. Used by the file loaders so that names and
// categories may contain spaces (the old "file >> name" approach could not).
inline vector<string> splitLine(const string& line, char delimiter)
{
    vector<string> parts;
    string current;
    stringstream ss(line);

    while (getline(ss, current, delimiter))
    {
        // Drop a stray carriage return when a Windows-written file is read
        // on macOS (or the other way round).
        if (!current.empty() && current[current.size() - 1] == '\r')
        {
            current.erase(current.size() - 1);
        }

        parts.push_back(current);
    }

    return parts;
}

inline int toInt(const string& value, int fallback = 0)
{
    try
    {
        return stoi(value);
    }
    catch (...)
    {
        return fallback;
    }
}

inline float toFloat(const string& value, float fallback = 0)
{
    try
    {
        return stof(value);
    }
    catch (...)
    {
        return fallback;
    }
}

#endif
