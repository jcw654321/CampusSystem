#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include "student_manager.h"
#include "subject_manager.h"

using namespace std;

struct InputClosed {};

//用来校验输入
string input(const string& prompt) {
    cout << prompt;
    string value;
    if (!getline(cin, value))
        throw InputClosed{};
    return value;
}

//留空时保持不变
string inputOrCurrent(const string& prompt, const string& currentValue) {
    const string value = input(prompt);
    return value.empty() ? currentValue : value;
}

//查找学生并返回学生类型指针
Student* findStudent(vector<Student>& students, const string& id) {
    for (Student& student : students) {
        if (student.getStudentId() == id) {
            return &student;
        }
    }
    cout << "没有找到该学生。\n";
    return nullptr;
}


//用来判断输入的学号或课程iD是否存在
Student* selectStudentSubject(vector<Student>& students, SubjectManager& subjectManager,const string& subjectPrompt,string& subjectId) {
    Student* student = findStudent(students, input("请输入学号: "));
    if (!student)
        return nullptr;

    subjectId = input(subjectPrompt);
    if (!subjectManager.hasSubject(*student, subjectId)) {
        cout << "该学生未选修此课程。\n";
        return nullptr;
    }
    return student;
}

//
void saveStudentSubjects(Student& student, const Student& updatedStudent, const SubjectManager& subjectManager,const string& successMessage,const string& failureMessage) {
    if (!subjectManager.saveSubjectsToFile(updatedStudent)) {
        cout << failureMessage << '\n';
        return;
    }

    student = updatedStudent;
    cout << successMessage << '\n';
}

void managesubjects(vector<Student>& students,
                   SubjectManager& subjectManager) {
    while (true) {
        cout << "\n===== 课程管理 =====\n"
             << "1. 查看学生课程\n"
             << "2. 添加课程\n"
             << "3. 删除课程\n"
             << "4. 录入/修改成绩\n"
             << "0. 返回主菜单\n";
        const string choice = input("请选择: ");

        switch (choice.size() == 1 ? choice[0] : '\0') {
        //1. 查看学生课程
        case '1': {
            const string studentId = input("请输入学号: ");
            Student* student = findStudent(students, studentId);
            if (student)
                subjectManager.showStudentsubjects(*student);
            break;
        }
        //2. 添加课程
        case '2': {
            const string studentId = input("请输入学号: ");
            Student* student = findStudent(students, studentId);
            if (!student)
                break;

            const string subjectName = input("课程名称: ");
            const string subjectId = input("课程ID: ");
            const string teacherName = input("教师姓名: ");
            if (subjectName.empty() || subjectId.empty() || teacherName.empty()) {
                cout << "课程名称、课程ID和教师姓名不能为空。\n";
                break;
            }

            if (subjectManager.hasSubject(*student, subjectId)) {
                cout << "该学生已存在此课程ID。\n";
                break;
            }

            const string scoreText = input("成绩 (0-100，留空表示未录入): ");
            float score = -1.0f;
            if (!scoreText.empty() &&!SubjectManager::parseScore(scoreText, score))
            {
                cout << "成绩格式无效，必须是 0 到 100 之间的数字。\n";
                break;
            }

            Student updatedStudent = *student;
            subjectManager.addSubject(updatedStudent,subject(subjectName, subjectId, teacherName, score));
            saveStudentSubjects(*student, updatedStudent, subjectManager,
                                "课程添加并保存完成。",
                                "保存课程失败，未更改学生课程。");
            break;
        }
        //3. 删除课程
        case '3': {
            string subjectId;
            Student* student = selectStudentSubject(
                students, subjectManager, "请输入要删除的课程ID: ", subjectId);
            if (!student)
                break;

            Student updatedStudent = *student;
            subjectManager.removeSubject(updatedStudent, subjectId);
            saveStudentSubjects(*student, updatedStudent, subjectManager,
                                "课程删除并保存完成。",
                                "保存课程失败，未更改学生课程。");
            break;
        }
        //4. 录入/修改成绩
        case '4': {
            string subjectId;
            Student* student = selectStudentSubject(
                students, subjectManager, "请输入课程ID: ", subjectId);
            if (!student)
                break;

            const string scoreText = input("请输入成绩 (0-100): ");
            float score = 0.0f;
            if (!SubjectManager::parseScore(scoreText, score)) {
                cout << "成绩格式无效，必须是 0 到 100 之间的数字。\n";
                break;
            }

            Student updatedStudent = *student;
            subjectManager.updateScore(updatedStudent, subjectId, score);
            saveStudentSubjects(*student, updatedStudent, subjectManager,
                                "成绩修改并保存完成。",
                                "保存成绩失败，未更改学生课程。");
            break;
        }
        case '0':
            return;
        default:
            cout << "无效选项。\n";
            break;
        }
    }
}

int runApplication() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    StudentManager manager("管理员", "admin@example.com", "admin", "123456");
    SubjectManager subjectManager("管理员", "admin@example.com", "admin", "123456");
    vector<Student> students = manager.loadStudentsFromFile();
    subjectManager.loadSubjectsFromfile(students);

    while (true) {
        cout << "\n===== 学生管理系统 =====\n"
             << "1. 查看所有学生\n"
             << "2. 查询学生\n"
             << "3. 新增学生\n"
             << "4. 修改学生\n"
             << "5. 删除学生\n"
             << "6. 按平均成绩排名\n"
             << "7. 按课程筛选学生\n"
             << "8. 课程管理\n"
             << "0. 退出\n";

        const string choice = input("请选择: ");

        switch (choice.size() == 1 ? choice[0] : '\0') {
        //1. 查看所有学生
        case '1':
            if (students.empty())
                cout << "当前没有学生。\n";
            for (const Student& student : students) {
                cout << "\n----------------\n";
                manager.printStudentInfo(student);
            }
            break;
        //2. 查询学生
        case '2': {
            const string id = input("请输入学号: ");
            Student* student = findStudent(students, id);
            if (student)
                manager.printStudentInfo(*student);
            break;
        }
        //3. 新增学生
        case '3': {
            const string userId = input("用户ID: ");
            const string name = input("姓名: ");
            const string sex = input("性别: ");
            const string email = input("邮箱: ");
            const string password = input("密码: ");
            const string major = input("专业: ");
            const string id = input("学号: ");
            const string className = input("班级: ");

            const bool studentIdExists = any_of(
                students.begin(), students.end(),
                [&id](const Student& student) {
                    return student.getStudentId() == id;
                });
            if (studentIdExists) {
                cout << "学号已存在，未新增。\n";
                break;
            }

            Student student(name, email, userId, password, major, id, className, sex);
            if (manager.addStudent(students, student))
                cout << "新增并保存完成。\n";
            else
                cout << "新增完成，但保存文件失败。\n";
            break;
        }
        case '4': {
        //4. 修改学生
            const string id = input("请输入要修改的学号: ");

            Student* currentStudent = findStudent(students, id);
            if (currentStudent == nullptr)
                break;

            const string userId = inputOrCurrent("用户ID（留空保持不变）: ", currentStudent->getUserid());
            const string name = inputOrCurrent("姓名（留空保持不变）: ", currentStudent->getName());
            const string sex = inputOrCurrent("性别（留空保持不变）: ", currentStudent->getSex());
            const string email = inputOrCurrent("邮箱（留空保持不变）: ", currentStudent->getEmail());
            const string password = inputOrCurrent("密码（留空保持不变）: ", currentStudent->getPassword());
            const string major = inputOrCurrent("专业（留空保持不变）: ", currentStudent->getMajor());
            const string className = inputOrCurrent("班级（留空保持不变）: ", currentStudent->getClassName());
             
            Student updatedStudent(name, email, userId, password, major, id, className, sex);
            if (manager.updateStudent(students, updatedStudent))
                cout << "修改并保存完成。\n";
            else
                cout << "修改完成，但保存文件失败。\n";
            break;
        }
        //5. 删除学生
        case '5': {
            const string id = input("请输入要删除的学号: ");
            if (!findStudent(students, id))
                break;

            if (manager.removeStudent(students, id))
                cout << "删除并保存完成。\n";
            else
                cout << "删除完成，但保存文件失败。\n";
            break;
        }
        //6. 按平均成绩排名
        case '6': {
            const vector<StudentAverage> ranked =
                subjectManager.rankByAverageScore(students);
            if (ranked.empty()) {
                cout << "目前没有可参与排名的成绩。\n";
                break;
            }

            cout << fixed << setprecision(2);
            for (size_t i = 0; i < ranked.size(); ++i) {
                const Student& student = *ranked[i].student;
                cout << i + 1 << ". " << student.getName()
                     << " (学号: " << student.getStudentId()
                     << "), 平均成绩: " << ranked[i].average << '\n';
            }
            break;
        }
        case '7': {
        //7. 按课程筛选学生
            const string subjectId = input("请输入课程ID: ");
            const vector<const Student*> matched =
                subjectManager.findBysubject(students, subjectId);
            if (matched.empty()) {
                cout << "没有学生选修该课程。\n";
                break;
            }

            for (const Student* student : matched) {
                for (const subject& course : student->getSubjects()) {
                    if (course.getSubjectId() == subjectId) {
                        cout << "学生: " << student->getName()
                             << " (学号: " << student->getStudentId() << ")"
                             << ", 课程: " << course.getName()
                             << ", 教师: " << course.getTeacherName()
                             << ", 成绩: ";
                        if (course.getScore() >= 0.0f)
                            cout << course.getScore();
                        else
                            cout << "未录入";
                        cout << '\n';
                        break;
                    }
                }
            }
            break;
        }
        case '8':
        //8. 课程管理
            managesubjects(students, subjectManager);
            break;
        case '0':
        //0. 退出
            cout << "已退出。\n";
            return 0;
        default:
            cout << "无效选项。\n";
            break;
        }
    }
}

int main() {
    return runApplication();
}
