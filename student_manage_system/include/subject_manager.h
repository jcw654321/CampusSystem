#pragma once

#include "student.h"
#include "manager.h"
#include <vector>
#include <string>
using namespace std;

struct StudentAverage
{
    const Student *student;
    double average;
};

// 学科管理类，继承自管理类，包含对学生选课与成绩管理的操作
class SubjectManager : public Manager
{
public:
    SubjectManager(string name, string email, string userid, string password)
        : Manager(name, email, userid, password) {};

    bool loadRecords() override; // 加载学生课程记录
    bool saveRecords() const override; // 保存学生课程记录
    bool addRecord(const DataRecord& record) override; // 添加学生课程记录
    bool removeRecord(const string& key) override; // 删除学生课程记录
    bool updateRecord(const string& key, const DataRecord& record) override; // 更新学生课程记录
    bool findRecord(const string& key, DataRecord& record) const override; // 查找学生课程记录
    void loadSubjectsFromfile(vector<Student> &students);                     // 把学生的课程信息加载到学生对象中
    void syncStudent(Student& student);
    void showStudentsubjects(const Student &student) const;                   // 打印学生的所有课程信息
    static bool parseScore(const string &text, float &score);                 //解析输入的分数
    vector<StudentAverage> rankByAverageScore(const vector<Student> &students) const; //按课程平均分排列
    vector<const Student *> findBysubject(const vector<Student> &students,const string &subjectId) const;//按课程筛选
private:
    vector<Student> records;
};
