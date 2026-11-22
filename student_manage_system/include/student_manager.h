#pragma once

#include "manager.h"
#include "student.h"
#include<string>
#include<vector>
using namespace std;
// 学生管理类，继承自管理类，包含对学生的增删改查操作
class StudentManager : public Manager {
public:
    StudentManager(string name, string email, string userid, string password)
        : Manager(name, email, userid, password) {};

    bool loadRecords() override;  // 加载学生记录
    bool saveRecords() const override;  // 保存学生记录
    bool addRecord(const DataRecord& record) override;  // 添加学生记录
    bool removeRecord(const string& key) override;  // 删除学生记录
    bool updateRecord(const string& key, const DataRecord& record)override;// 更新学生记录
    bool findRecord(const string& key, DataRecord& record) const override; // 查找学生记录
    const vector<Student>& getStudents() const; // 获取学生列表
    void printStudentInfo(const Student& student) const; // 打印学生信息
private:
    vector<Student> students; // 学生列表
};

