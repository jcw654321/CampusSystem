#include"subject.h"
#include"student.h"
#include<iostream>
#include<vector>
#include<string>
using namespace std;

// 学科管理类，继承自用户类，包含对学生选课与成绩管理的操作
class SubjectManager : public User {
public:
    SubjectManager(string name, string email, string userid, string password)
        : User(name, email, userid, password) {};
    void getAllstudents(vector<Student>& students); // 获取所有学生信息
    void addSubject(Student& student, const subject& s); // 添加课程
    void removeSubject(Student& student, const string& subjectId); // 删除课程
    void updateScore(Student& student, const string& subjectId, float score); // 更新课程成绩
    void getSubject(const Student& student, const string& subjectId) const; // 打印学生的课程信息
    void getScores(const Student& student) const; // 打印学生成绩信息

private:
    vector<Student> students; // 学生列表
};

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

