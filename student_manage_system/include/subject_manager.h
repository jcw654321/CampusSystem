#include"subject.h"
#include"student.h"
#include<vector>
#include<string>
using namespace std;

// 学科管理类，继承自用户类，包含对学生选课与成绩管理的操作
class SubjectManager : public User {
public:
    SubjectManager(string name, string email, string userid, string password)
        : User(name, email, userid, password) {};
    void getAllstudents(vector<Student>& students); // 获取所有学生信息
    void addSubject(Student& student, const subject& s); // 添加课程
    void removeSubject(Student& student, const string& subjectId); // 删除课程
    void updateScore(Student& student, const string& subjectId, float score); // 更新课程成绩
    void getSubject(const Student& student, const string& subjectId) const; // 打印学生的课程信息
    void getScores(const Student& student) const; // 打印学生成绩信息

private:
    vector<Student> students; // 学生列表
};



