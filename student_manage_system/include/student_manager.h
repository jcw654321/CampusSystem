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
    void getAllStudents(const vector<Student>& students) ; // 获取所有学生信息
    void printStudentInfo(const Student& student) const; // 打印学生信息
    void addStudent(vector<Student>& students, const Student& student);     // 添加学生
    void removeStudent(vector<Student>& students, const string& studentId);  // 删除学生
    void updateStudent(vector<Student>& students, const Student& student);   // 更新学生信息
    Student getStudent(const vector<Student>& students, const string& studentId) const; // 获取学生信息
private:
    vector<Student> students; // 学生列表
};

