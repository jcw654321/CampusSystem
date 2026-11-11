#include "user.h"
#include "subject.h"
#include<string>
#include<vector>
using namespace std;

// 学生类，继承自用户类，包含学生的基本信息和操作
class Student : public User {
public:
    Student(string name, string email, string userid, string password, string major, string studentId, string className)
        : User(name, email, userid, password), major(major), studentId(studentId), className(className) {};
    string getMajor() const;                      // 获取专业
    string getStudentId() const;                  // 获取学号
    string getClassName() const;                  // 获取班级
    const vector<subject>& getSubjects() const;   // 获取所选课程列表
    void setMajor(const string& major);           // 设置专业
    void setStudentId(const string& studentId);   // 设置学号
    void setClassName(const string& className);   // 设置班级

private:
    string major;            // 专业
    string studentId;        // 学号
    string className;        // 班级
    vector<subject> subjects; // 学生所选课程列表
friend class StudentManager; // 声明StudentManager为友元类，以便访问私有成员
friend class SubjectManager;   // 声明SubjectManager为友元类，以便访问私有成员
};


