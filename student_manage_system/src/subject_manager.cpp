#include "subject_manager.h"
#include<iostream>
#include<algorithm>
using namespace std;

void SubjectManager::getAllstudents(vector<Student>& students) {
    students = this->students;
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