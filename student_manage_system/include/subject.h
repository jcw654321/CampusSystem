#pragma once

#include<string>
using namespace std;

//学科类用于管理学生的课程信息，包括课程名称、课程ID、教师姓名和成绩
class subject {
public:
    subject(string name, string subjectId, string teacherName, float score)
        : name(name), subjectId(subjectId), teacherName(teacherName), score(score) {};
    string getName() const;                       // 获取课程名称
    string getSubjectId() const;                  // 获取课程ID
    string getTeacherName() const;                // 获取教师姓名
    float getScore() const;                       // 获取课程成绩
    void setName(const string& name);             // 设置课程名称
    void setSubjectId(const string& subjectId);   // 设置课程ID
    void setTeacherName(const string& teacherName); // 设置教师姓名
    void setScore(float score);                   // 设置课程成绩

private:
    string name;           // 课程名称
    string subjectId;      // 课程ID
    string teacherName;    // 教师姓名
    float score=-1.0;           // 课程成绩(若该课程还没有成绩，则为-1.0)
};



