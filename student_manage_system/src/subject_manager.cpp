#include "subject_manager.h"
#include "fileoperate.h"
#include<iostream>
#include<algorithm>
#include<fstream>
#include <iomanip>
#include <iostream>
#include<stdexcept>
using namespace std;

const string student_filepath = ".\\data\\student_data\\student_data.csv";

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

                float score = 0.0;
                if(!parseScore(fields[3],score))

                addSubject(student, subject(fields[0],fields[1],fields[2],score));
        
        }
    }
}

bool SubjectManager::saveSubjectsToFile(const Student& student) const
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

    file.flush();
    return  true;
}

bool SubjectManager::hasSubject(const Student& student, const string& subjectId) const
{
    const vector<subject>& subjects = student.getSubjects();
    return any_of(subjects.begin(), subjects.end(),
                  [&subjectId](const subject& current)
                  {
                      return current.getSubjectId() == subjectId;
                  });
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

void SubjectManager::showScoreRanking(const vector<Student>& students) const
{
    const vector<StudentAverage> ranked = rankByAverageScore(students);
    if (ranked.empty())
    {
        cout << "目前没有可参与排名的成绩。\n";
        return;
    }

    cout << fixed << setprecision(2);
    for (size_t i = 0; i < ranked.size(); ++i)
    {
        const Student& student = *ranked[i].student;
        cout << i + 1 << ". " << student.getName()
             << " (学号: " << student.getStudentId()
             << "), 平均成绩: " << ranked[i].average << '\n';
    }
}

void SubjectManager::filterStudentsBysubject(
    const vector<Student>& students, const string& subjectId) const
{
    const vector<const Student*> matched = findBysubject(students, subjectId);
    if (matched.empty())
    {
        cout << "没有学生选修该课程。\n";
        return;
    }

    for (const Student* student : matched)
    {
        for (const subject& subject : student->getSubjects())
        {
            if (subject.getSubjectId() == subjectId)
            {
                cout << "学生: " << student->getName()
                     << " (学号: " << student->getStudentId() << ")"
                     << ", 课程: " << subject.getName()
                     << ", 教师: " << subject.getTeacherName()
                     << ", 成绩: ";
                if (subject.getScore() >= 0.0f)
                    cout << subject.getScore();
                else
                    cout << "未录入";
                cout << '\n';
                break;
            }
        }
    }
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

void SubjectManager::addSubject(Student& student, const subject& s) {
    student.subjects.push_back(s);
}

void SubjectManager::removeSubject(Student& student, const string& subjectId) {
    auto& subjects = student.subjects;
    for (auto it = subjects.begin(); it != subjects.end(); ) {
        if (it->getSubjectId() == subjectId) {
            it = subjects.erase(it); // erase 返回下一个有效迭代器
        } else {
            ++it;
        }
    }
}

void SubjectManager::updateScore(Student& student, const string& subjectId, float score) {
    for (auto& s : student.subjects) {
        if (s.getSubjectId() == subjectId) {
            s.setScore(score);
            break;
        }
    }
}

void SubjectManager::getSubject(const Student& student, const string& subjectId) const {
    for (const auto& s : student.subjects) {
        if (s.getSubjectId() == subjectId) {
            cout << "课程名称: " << s.getName() << ", 课程ID: " << s.getSubjectId()
                 << ", 教师姓名: " << s.getTeacherName() << ", 成绩: " << s.getScore() << endl;
            return;
        }
    }
    cout << "未找到该课程信息。" << endl;
}

void SubjectManager::getScores(const Student& student) const {
    cout << "学生: " << student.getName() << " 的成绩信息:" << endl;
    for (const auto& s : student.subjects) {
        if(s.getScore() >= 0.0) { // 只打印有成绩的课程
            cout << "课程名称: " << s.getName() << ", 课程ID: " << s.getSubjectId()
                 << ", 教师姓名: " << s.getTeacherName() << ", 成绩: " << s.getScore() << endl;
        } else {
            cout << "课程名称: " << s.getName() << ", 课程ID: " << s.getSubjectId()
                 << ", 教师姓名: " << s.getTeacherName() << ", 成绩: 未录入" << endl;
        }
    }
}