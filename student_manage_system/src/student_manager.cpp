#include "student_manager.h"
#include <iostream>
using namespace std;



void StudentManager::getAllStudents(const vector<Student>& students)  {
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
    return Student("", "", "", "", "", "", ""); // 返回一个空的学生对象
}

void StudentManager::printStudentInfo(const Student& student) const {
    cout << "Name: " << student.getName() << endl;
    cout << "Email: " << student.getEmail() << endl;
    cout << "User ID: " << student.getUserid() << endl;
    cout << "Major: " << student.getMajor() << endl;
    cout << "Student ID: " << student.getStudentId() << endl;
    cout << "Class Name: " << student.getClassName() << endl;
}