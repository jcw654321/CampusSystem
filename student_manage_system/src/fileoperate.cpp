#include "fileoperate.h"

//用于解析文件中的一行(如果有两个""则减去一个)
bool parseCsvLine(const string& line, vector<string>& fields)
{
    fields.clear();
    string field;
    bool inQuotes = false;

    for (size_t i = 0; i < line.size(); ++i)
    {
        char ch = line[i];
        if (ch == '"')
        {
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"')
            {
                field += '"';
                ++i;
            }
            else
                inQuotes = !inQuotes;
        }
        else if (ch == ',' && !inQuotes)
        {
            fields.push_back(field);
            field.clear();
        }
        else
            field += ch;
    }

    fields.push_back(field);
    return !inQuotes;
}

//用于将字段值转义为CSV格式
string escapeCsvField(const string& value)
{
    if (value.find_first_of(",\"\r\n") == string::npos)
        return value;

    string escaped = "\"";
    for (char ch : value)
    {
        if (ch == '"')
            escaped += '"';
        escaped += ch;
    }
    return escaped + '"';
}