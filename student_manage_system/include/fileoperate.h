#pragma once

#include <fstream>
#include <iostream>
#include<vector>
using namespace std;

// 解析CSV行，将其拆分为字段
bool parseCsvLine(const string& line, vector<string>& fields);

// 转义CSV字段，如果字段中包含逗号、引号或换行符，则需要加引号并转义引号
string escapeCsvField(const string& value);