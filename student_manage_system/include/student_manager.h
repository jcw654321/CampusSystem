#pragma once

#include "user.h"
#include "student.h"
#include<string>
#include<vector>
using namespace std;
// 学生管理类，继承自用户类，包含对学生的增删改查操作
class StudentManager : public User {
public:
    StudentManager(string name, string email, string userid, string password)
        : User(name, email, userid, password) {};
    vector<Student> loadStudentsFromFile(); //从文件中加载学生信息
    void getAllStudents(const vector<Student>& students) ; // 获取所有学生信息
    void printStudentInfo(const Student& student) const; // 打印学生信息
    bool saveStudentsToFile(const vector<Student>& students) const;//保存学生信息到文件
    bool addStudent(vector<Student>& students, const Student& student);     // 添加学生
    bool removeStudent(vector<Student>& students, const string& studentId);  // 删除学生
    bool updateStudent(vector<Student>& students, const Student& student);   // 更新学生信息
    Student getStudent(const vector<Student>& students, const string& studentId) const; // 获取学生信息
    const Student* findStudentByUserid(const vector<Student>& students,
                                       const string& userid) const;
private:
    vector<Student> students; // 学生列表
};

