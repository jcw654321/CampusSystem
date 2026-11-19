#include "student_manager.h"
#include "fileoperate.h"
#include <fstream>
#include <iostream>
using namespace std;

const string filepath = ".\\data\\student_data\\student_data.csv";

vector<Student> StudentManager::loadStudentsFromFile()
{
    ifstream file(filepath, ios::binary);
    if (!file)
        return {};

    string line;
    getline(file, line);

    vector<Student> loadedStudents;
    vector<string> fields;
    while (getline(file, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty())
            continue;
        if (!parseCsvLine(line, fields) || fields.size() != 8)
           continue;

        Student student(fields[1], fields[3], fields[0], fields[4],
                        fields[5], fields[6], fields[7]);
        student.setSex(fields[2]);
        loadedStudents.push_back(student);
    }

    students.swap(loadedStudents);
    this->students = students;
    return students;
}

void StudentManager::getAllStudents(const vector<Student>& students) {
    this->students = students;
}

bool StudentManager::saveStudentsToFile(const vector<Student>& students) const
{
    ofstream file(filepath, ios::binary | ios::trunc);
    if (!file)
        return false;

    file << "\xEF\xBB\xBF" << "用户ID,用户名,性别,邮箱,密码,专业,学号,班级\n";
    for (const Student& student : students)
    {
        file << escapeCsvField(student.getUserid()) << ','
             << escapeCsvField(student.getName()) << ','
             << escapeCsvField(student.getSex()) << ','
             << escapeCsvField(student.getEmail()) << ','
             << escapeCsvField(student.getPassword()) << ','
             << escapeCsvField(student.getMajor()) << ','
             << escapeCsvField(student.getStudentId()) << ','
             << escapeCsvField(student.getClassName()) << '\n';
    }
    return static_cast<bool>(file);
}

bool StudentManager::addStudent(vector<Student>& students, const Student& student) {
    students.push_back(student);
    return saveStudentsToFile(students);
}

bool StudentManager::removeStudent(vector<Student>& students, const string& studentId) {
    for (auto it = students.begin(); it != students.end(); ++it) {
        if (it->getStudentId() == studentId) {
            students.erase(it);
            return saveStudentsToFile(students);
        }
    }
    return false;
}

bool StudentManager::updateStudent(vector<Student>& students, const Student& student) {
    for (auto& current : students) {
        if (current.getStudentId() == student.getStudentId()) {
            current.setName(student.getName());
            current.setEmail(student.getEmail());
            current.setPassword(student.getPassword());
            current.setMajor(student.getMajor());
            current.setClassName(student.getClassName());
            current.setSex(student.getSex());
            return saveStudentsToFile(students);
        }
    }
    return false;
}

Student StudentManager::getStudent(const vector<Student>& students, const string& studentId) const {
    for (const auto& s : students) {
        if (s.getStudentId() == studentId) {
            return s;
        }
    }
    cout << "Student with ID " << studentId << " not found." << endl;
    return Student("", "", "", "", "", "", "");
}

const Student* StudentManager::findStudentByUserid(
    const vector<Student>& students, const string& userid) const
{
    for (const Student& student : students)
    {
        if (student.getUserid() == userid)
            return &student;
    }
    return nullptr;
}

void StudentManager::printStudentInfo(const Student& student) const {
    cout << "Name: " << student.getName() << endl;
    cout << "Email: " << student.getEmail() << endl;
    cout << "User ID: " << student.getUserid() << endl;
    cout << "Major: " << student.getMajor() << endl;
    cout << "Student ID: " << student.getStudentId() << endl;
    cout << "Class Name: " << student.getClassName() << endl;
}
