#include "student_manager.h"
#include "fileoperate.h"
#include "user_manager.h"
#include <filesystem>
#include <fstream>
#include <iostream>
using namespace std;

const string filepath = ".\\data\\student_data\\student_data.csv";

bool StudentManager::loadRecords()
{
    ifstream file(filepath, ios::binary);
    if (!file)
        return false;

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
                        fields[5], fields[6], fields[7], fields[2]);
        loadedStudents.push_back(student);
    }

    students.swap(loadedStudents);
    return true;
}

bool StudentManager::saveRecords() const
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

bool StudentManager::addRecord(const DataRecord& record)
{
    if (record.fields.size() != 8)
        return false;

    const vector<string>& f = record.fields;
    Student student(f[1], f[3], f[0], f[4],
                    f[5], f[6], f[7], f[2]);
    students.push_back(student);
    if (!saveRecords())
        return false;

    UserManager userManager("..\\data\\user.csv");
    return userManager.SaveUser(student);
}

bool StudentManager::removeRecord(const string& studentId)
{
    for (auto it = students.begin(); it != students.end(); ++it) {
        if (it->getStudentId() == studentId) {
            const filesystem::path courseFileName(studentId + ".csv");
            if (studentId.empty() || courseFileName.has_parent_path() ||
                courseFileName.has_root_name())
                return false;

            const string userid = it->getUserid();
            students.erase(it);
            if (!saveRecords())
                return false;

            UserManager userManager("..\\data\\user.csv");
            const bool userDeleted = userManager.DeleteUser(userid);

            const filesystem::path courseFile =
                filesystem::path(".\\data\\subject_data") / courseFileName;
            error_code error;
            filesystem::remove(courseFile, error);
            if (error)
                cerr << "Failed to remove course file " << courseFile.string()
                     << ": " << error.message() << '\n';

            return userDeleted && !error;
        }
    }
    return false;
}

bool StudentManager::updateRecord(const string& key, const DataRecord& record)
{
    if (record.fields.size() != 8)
        return false;

    const vector<string>& f = record.fields;
    Student student(f[1], f[3], f[0], f[4],
                    f[5], f[6], f[7], f[2]);
    for (auto& current : students) {
        if (current.getStudentId() == key) {
            const string previousUserid = current.getUserid();
            current.setName(student.getName());
            current.setEmail(student.getEmail());
            current.setPassword(student.getPassword());
            current.setMajor(student.getMajor());
            current.setClassName(student.getClassName());
            current.setSex(student.getSex());
            current.setUserid(student.getUserid());
            if (!saveRecords())
                return false;

            UserManager userManager("..\\data\\user.csv");
            return userManager.SaveUser(current, previousUserid);
        }
    }
    return false;
}

bool StudentManager::findRecord(const string& key, DataRecord& record) const
{
    for (const Student& student : students)
    {
        if (student.getStudentId() == key)
        {
            record.key = key;
            record.fields = {
                student.getUserid(), student.getName(), student.getSex(),
                student.getEmail(), student.getPassword(), student.getMajor(),
                student.getStudentId(), student.getClassName()
            };
            return true;
        }
    }
    return false;
}

const vector<Student>& StudentManager::getStudents() const
{
    return students;
}
void StudentManager::printStudentInfo(const Student& student) const {
    cout << "姓名: " << student.getName() << endl;
    cout << "性别: " << student.getSex() << endl;
    cout << "邮箱: " << student.getEmail() << endl;
    cout << "用户id: " << student.getUserid() << endl;
    cout << "专业: " << student.getMajor() << endl;
    cout << "学号: " << student.getStudentId() << endl;
    cout << "班级名:" << student.getClassName() << endl;
}
