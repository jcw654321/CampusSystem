#include "subject_manager.h"
#include "fileoperate.h"
#include "student_manager.h"
#include<iostream>
#include<algorithm>
#include<fstream>
#include <iostream>
using namespace std;

const string student_filepath = ".\\data\\student_data\\student_data.csv";

bool SubjectManager::loadRecords()
{
    StudentManager studentManager("管理员", "", "admin", "");
    if (!studentManager.loadRecords())
        return false;
    records = studentManager.getStudents();
    loadSubjectsFromfile(records);
    return true;
}

bool SubjectManager::saveRecords() const
{
    for (const Student& student : records)
    {
        const string filepath = ".\\data\\subject_data\\" +
                                student.getStudentId() + ".csv";
        ofstream file(filepath, ios::binary | ios::trunc);
        if (!file)
            return false;

        file << "\xEF\xBB\xBF" << "课程名称,课程ID,教师姓名,课程成绩\n";
        for (const subject& subject : student.getSubjects())
        {
            file << escapeCsvField(subject.getName()) << ','
                 << escapeCsvField(subject.getSubjectId()) << ','
                 << escapeCsvField(subject.getTeacherName()) << ','
                 << subject.getScore() << '\n';
        }
        if (!static_cast<bool>(file))
            return false;
    }
    return true;
}

bool SubjectManager::addRecord(const DataRecord& record)
{
    if (record.fields.size() != 5)
        return false;

    const vector<string>& f = record.fields;
    float score = -1.0f;
    if (!f[4].empty() && f[4] != "-1" && !parseScore(f[4], score))
        return false;

    if (!loadRecords())
        return false;

    for (Student& student : records)
    {
        if (student.getStudentId() == f[0])
        {
            for (const subject& existing : student.getSubjects())
            {
                if (existing.getSubjectId() == f[2])
                    return false;
            }

            student.subjects.push_back(subject(f[1], f[2], f[3], score));
            return saveRecords();
        }
    }
    return false;
}

bool SubjectManager::removeRecord(const string& key)
{
    const size_t separator = key.find('|');
    if (separator == string::npos)
        return false;

    const string studentId = key.substr(0, separator);
    const string subjectId = key.substr(separator + 1);
    for (Student& student : records)
    {
        if (student.getStudentId() == studentId)
        {
            student.subjects.erase(
                remove_if(student.subjects.begin(), student.subjects.end(),
                          [&subjectId](const subject& subject)
                          { return subject.getSubjectId() == subjectId; }),
                student.subjects.end());
            return saveRecords();
        }
    }
    return false;
}

bool SubjectManager::updateRecord(const string& key, const DataRecord& record)
{
    if (record.fields.size() != 5)
        return false;

    const vector<string>& f = record.fields;
    float score = -1.0f;
    if (!f[4].empty() && !parseScore(f[4], score))
        return false;

    const size_t separator = key.find('|');
    if (separator == string::npos)
        return false;

    const string studentId = key.substr(0, separator);
    const string subjectId = key.substr(separator + 1);
    for (Student& student : records)
    {
        if (student.getStudentId() == studentId)
        {
            for (subject& subject : student.subjects)
            {
                if (subject.getSubjectId() == subjectId)
                {
                    if (!f[1].empty())
                        subject.setName(f[1]);
                    if (!f[2].empty())
                        subject.setSubjectId(f[2]);
                    if (!f[3].empty())
                        subject.setTeacherName(f[3]);
                    subject.setScore(score);
                    return saveRecords();
                }
            }
        }
    }
    return false;
}

void SubjectManager::syncStudent(Student& student)
{
    for (Student& record : records)
    {
        if (record.getStudentId() == student.getStudentId())
        {
            student = record;
            return;
        }
    }
}

bool SubjectManager::findRecord(const string& key, DataRecord& record) const
{
    const size_t separator = key.find('|');
    if (separator == string::npos)
        return false;

    const string studentId = key.substr(0, separator);
    const string subjectId = key.substr(separator + 1);
    for (const Student& student : records)
    {
        if (student.getStudentId() != studentId)
            continue;

        for (const subject& subject : student.subjects)
        {
            if (subject.getSubjectId() == subjectId)
            {
                record.key = key;
                record.fields = {
                    studentId,
                    subject.getName(),
                    subject.getSubjectId(),
                    subject.getTeacherName(),
                    to_string(subject.getScore())
                };
                return true;
            }
        }
    }
    return false;
}

void SubjectManager::loadSubjectsFromfile(vector<Student>&students)
{
    for (Student& student : students)
    {
        string filepath = ".\\data\\subject_data\\" +
                          student.getStudentId() + ".csv";

        ifstream file(filepath, ios::binary);
        if (!file)
            continue;

        string line;
        getline(file, line); // 跳过表头

        vector<string> fields;

        while (getline(file, line))
        {
            if (!line.empty() && line.back() == '\r')//删掉最后的/r
                line.pop_back();

            if (line.empty())
                continue;

            if (!parseCsvLine(line, fields) || fields.size() != 4 ||
                fields[1].empty())   //如果此行解析失败或缺少某个要素或没有课程名跳过
                continue;

            float score = -1.0f;
            if (!fields[3].empty() && fields[3] != "-1" &&
                !parseScore(fields[3], score))
                continue;

            student.subjects.push_back(
                subject(fields[0], fields[1], fields[2], score));
        
        }
    }
}

void SubjectManager::showStudentsubjects(const Student& student) const
{
    cout << "学生: " << student.getName()
         << " (" << student.getStudentId() << ")\n";
    if (student.getSubjects().empty())
    {
        cout << "该学生暂无课程。\n";
        return;
    }

    for (const subject& subject : student.getSubjects())
    {
        cout << "课程名称: " << subject.getName()
             << ", 课程ID: " << subject.getSubjectId()
             << ", 教师: " << subject.getTeacherName()
             << ", 成绩: ";
        if (subject.getScore() >= 0.0f)
            cout << subject.getScore();
        else
            cout << "未录入";
        cout << '\n';
    }
}

bool SubjectManager::parseScore(const string& text, float& score)
{
        size_t parsedLength = 0;
        score = stof(text, &parsedLength); //把成绩由string转成float
        return parsedLength == text.size()&&score >= 0.0f && score <= 100.0f;    //必须没有多余的字符且不是非法float且在0-100
}

vector<StudentAverage> SubjectManager::rankByAverageScore(const vector<Student>& students) const
{
    vector<StudentAverage> ranked;

    for (const Student& student : students)
    {
        double total = 0;
        int count = 0;

        for (const subject& subject : student.getSubjects())
        {
            if (subject.getScore() >= 0)
            {
                total += subject.getScore();
                ++count;
            }
        }

        // 没有已录入成绩的学生不参与排名
        if (count > 0)
            ranked.push_back({&student, total / count});
    }

    sort(ranked.begin(), ranked.end(),
         [](const StudentAverage& a, const StudentAverage& b)
         {
             if (a.average != b.average)
                 return a.average > b.average;
             return a.student->getStudentId() < b.student->getStudentId();
         });

    return ranked;
}

vector<const Student*> SubjectManager::findBysubject(const vector<Student>& students,const string& subjectId) const
{
    vector<const Student*> matched;

    for (const Student& student : students)
    {
        const vector<subject>& subjects = student.getSubjects();
        bool selected = any_of(
            subjects.begin(), subjects.end(),
            [&subjectId](const subject& subject)
            {
                return subject.getSubjectId() == subjectId;
            });

        if (selected)
            matched.push_back(&student);
    }

    return matched;
}
