#include "student_account.h"

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

using namespace std;

static vector<string> splitLine(const string &line, char delim)
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
        result.push_back("");
    }

    return result;
}

bool StudentAccountStore::load(const string &csvPath, string &reason)
{
    ifstream in(csvPath);
    if (!in.is_open())
    {
        reason = "无法打开学生账号文件：" + csvPath;
        return false;
    }

    vector<StudentAccount> tmpStu;
    string line;

    getline(in, line); // 跳过表头

    while (getline(in, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty())
            continue;

        vector<string> vec = splitLine(line, ',');
        if (vec.size() < 8)
            continue;

        tmpStu.push_back(StudentAccount{vec[6], vec[1], vec[4]});
    }

    students.swap(tmpStu);
    return true;
}

const StudentAccount *StudentAccountStore::findByStudentId(const string &id) const
{
    for (const auto &stu : students)
    {
        if (stu.stuId == id)
            return &stu;
    }
    return nullptr;
}

bool StudentAccountStore::verify(const string &studentId,
                                 const string &password,
                                 StudentAccount &out) const
{
    const StudentAccount *stu = findByStudentId(studentId);
    if (stu == nullptr)
        return false;

    if (stu->stuPassword != password)
        return false;

    out = *stu;
    return true;
}

int StudentAccountStore::size() const
{
    return static_cast<int>(students.size());
}
