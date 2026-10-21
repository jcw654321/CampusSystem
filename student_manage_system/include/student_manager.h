#pragma once

#include "user.h"
#include "student.h"
#include<string>
#include<vector>
#include<iostream>
using namespace std;
// 学生管理类，继承自用户类，包含对学生的增删改查操作
class StudentManager : public User {
public:
    StudentManager(string name, string email, string userid, string password)
        : User(name, email, userid, password) {};
    void getAllStudents(const vector<Student>& students) const; // 获取所有学生信息
    void printStudentInfo(const Student& student) const; // 打印学生信息
    void addStudent(vector<Student>& students, const Student& student);     // 添加学生
    void removeStudent(vector<Student>& students, const string& studentId);  // 删除学生
    void updateStudent(vector<Student>& students, const Student& student);   // 更新学生信息
    Student getStudent(const vector<Student>& students, const string& studentId) const; // 获取学生信息
private:
    vector<Student> students; // 学生列表
};

void StudentManager::getAllStudents(const vector<Student>& students) const {
    this->students = students;
}

void StudentManager::addStudent(vector<Student>& students, const Student& student) {
    students.push_back(student);
}

void StudentManager::removeStudent(vector<Student>& students, const string& studentId) {
    for (auto it = students.begin(); it != students.end(); ++it) {
        if (it->getStudentId() == studentId) {
            students.erase(it);
            return;
        }
    }
}

void StudentManager::updateStudent(vector<Student>& students, const Student& student) {
    for (auto& s : students) {
        if (s.getStudentId() == student.getStudentId()) {
            s.setName(student.getName());
            s.setEmail(student.getEmail());
            s.setPassword(student.getPassword());
            s.setMajor(student.getMajor());
            s.setClassName(student.getClassName());
            s.setGrade(student.getGrade());
            return;
        }
    }
}

Student StudentManager::getStudent(const vector<Student>& students, const string& studentId) const {
    for (const auto& s : students) {
        if (s.getStudentId() == studentId) {
            return s;
        }
    }
    cout << "Student with ID " << studentId << " not found." << endl;
    return Student("", "", "", "", "", "", "", ""); // 返回一个空的学生对象
}