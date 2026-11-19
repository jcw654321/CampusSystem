#include "subject_manager.h"
#include "fileoperate.h"
#include<iostream>
#include<algorithm>
#include<fstream>
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
            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            if (line.empty())
                continue;

            if (!parseCsvLine(line, fields) || fields.size() != 4)
                continue;

                subject s(
                    fields[0],
                    fields[1],
                    fields[2],
                    stof(fields[3])
                );

                addSubject(student, s);
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

        for (const subject& course : student.getSubjects())
        {
            if (course.getScore() >= 0)
            {
                total += course.getScore();
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

vector<const Student*> SubjectManager::findByCourse(const vector<Student>& students,const string& courseId) const
{
    vector<const Student*> matched;

    for (const Student& student : students)
    {
        const vector<subject>& courses = student.getSubjects();
        bool selected = any_of(
            courses.begin(), courses.end(),
            [&courseId](const subject& course)
            {
                return course.getSubjectId() == courseId;
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
    subjects.erase(remove_if(subjects.begin(), subjects.end(),
                             [&subjectId](const subject& s) { return s.getSubjectId() == subjectId; }),
                   subjects.end());
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