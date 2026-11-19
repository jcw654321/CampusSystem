#include "student.h"
using namespace std;

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
