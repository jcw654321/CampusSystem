#pragma once
#include"student_account.h"
#include"library_system.h"
#include<fstream>
#include<filesystem>
using namespace std;
vector<string> splitLine(const string &line, char delim)
{
    vector<string> result;
    string token;
    stringstream ss(line);
    while (getline(ss, token, delim))
    {
        result.push_back(token);
    }
    if (!line.empty() && line.back() == delim)
    {
        result.push_back(""); // 处理最后一个元素为分隔符的情况
    }
    return result;
}
bool StudentAccountStore::load(const string& csvPath,string reason)
{
    ifstream in(csvPath);
    if(!in.is_open())
    {
        if(!filesystem::exists(csvPath))
        return true;
        reason = "无法打开文件"+csvPath;
        return false;
    }

    vector<StudentAccount> tmpStu;
    string line;
    getline(in,line); // 跳过第一行标题行
    while(getline(in,line))
    {
        if(line.empty())
        continue;
        vector<string> vec=splitLine(line,',');
        tmpStu.push_back(StudentAccount(vec[6],vec[1],vec[4]));
    }
    students = tmpStu;
    return true;
}
    
const StudentAccount* StudentAccountStore::findByStudentId(const string id) const//找到返回学生账户,找不到返回空
{
    for(const auto& stu:students)
    {
        if(stu.stuId==id)
        {
            return &stu;
        }
    }
    return nullptr;
}

bool StudentAccountStore::verify(string studentId,string password,StudentAccount& out) const
{
    const StudentAccount *stu1 = findByStudentId(studentId);
    if(stu1==nullptr)
    {
        return false;
    }
    if(stu1->stuPassword==password)
    {
        out=*stu1;
        return true;
    }
    return false;
}