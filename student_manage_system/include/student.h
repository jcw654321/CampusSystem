#include "user.h"
#include "subject.h"
#include<string>
#include<vector>
using namespace std;

// 学生类，继承自用户类，包含学生的基本信息和操作
class Student : public User {
public:
    Student(string name, string email, string userid, string password, string major, string studentId, string className, string grade)
        : User(name, email, userid, password), major(major), studentId(studentId), className(className), grade(grade) {};
    string getMajor() const;                      // 获取专业
    string getStudentId() const;                  // 获取学号
    string getClassName() const;                  // 获取班级
    string getGrade() const;                      // 获取年级
    const vector<subject>& getSubjects() const;   // 获取所选课程列表
    void setMajor(const string& major);           // 设置专业
    void setStudentId(const string& studentId);   // 设置学号
    void setClassName(const string& className);   // 设置班级
    void setGrade(const string& grade);           // 设置年级
    void addSubject(const subject& s);            // 添加课程

private:
    string major;            // 专业
    string studentId;        // 学号
    string className;        // 班级
    string grade;            // 年级
    vector<subject> subjects; // 学生所选课程列表
};


string Student::getMajor() const {
    return major;
}

string Student::getStudentId() const {
    return studentId;
}

string Student::getClassName() const {
    return className;
}

string Student::getGrade() const {
    return grade;
}

void Student::setMajor(const string& major) {
    this->major = major;
}

void Student::setStudentId(const string& studentId) {
    this->studentId = studentId;
}

void Student::setClassName(const string& className) {
    this->className = className;
}

void Student::setGrade(const string& grade) {
    this->grade = grade;
}

const vector<subject>& Student::getSubjects() const {
    return subjects;
}

void Student::addSubject(const subject& s) {
    subjects.push_back(s);
}