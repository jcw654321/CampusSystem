#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include "student_manager.h"

using namespace std;

string input(const string& prompt) {
    cout << prompt;
    string value;
    getline(cin, value);
    return value;
}

Student* findStudent(vector<Student>& students, const string& id) {
    for (Student& student : students) {
        if (student.getStudentId() == id) {
            return &student;
        }
    }
    return nullptr;
}

void showStudent(const StudentManager& manager, const Student& student) {
    cout << "性别: " << student.getSex() << endl;
    manager.printStudentInfo(student);
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    vector<Student> students;
    StudentManager manager("管理员", "admin@example.com", "admin", "123456");

    if (!manager.loadStudentsFromFile(students)) {
        cerr << "无法加载学生数据文件。\n";
        return 1;
    }

    while (true) {
        cout << "\n===== 学生管理系统 =====\n"
             << "1. 查看所有学生\n"
             << "2. 查询学生\n"
             << "3. 新增学生\n"
             << "4. 修改学生\n"
             << "5. 删除学生\n"
             << "0. 退出\n";

        string choice = input("请选择: ");

        if (choice == "1") {
            for (const Student& s : students) {
                cout << "\n----------------\n";
                showStudent(manager, s);
            }
        } else if (choice == "2") {
            Student* s = findStudent(students, input("请输入学号: "));
            if (s) showStudent(manager, *s);
            else cout << "没有找到该学生。\n";
        } else if (choice == "3") {
            string userId = input("用户ID: ");
            string name = input("姓名: ");
            string sex = input("性别: ");
            string email = input("邮箱: ");
            string password = input("密码: ");
            string major = input("专业: ");
            string id = input("学号: ");
            string className = input("班级: ");

            if (id.empty() || findStudent(students, id)) {
                cout << "学号为空或已存在，未新增。\n";
                continue;
            }

            Student s(name, email, userId, password, major, id, className);
            s.setSex(sex);
            if (manager.addStudent(students, s))
                cout << "新增并保存完成。\n";
            else
                cout << "新增完成，但保存文件失败。\n";
        } else if (choice == "4") {
            Student* s = findStudent(students, input("请输入要修改的学号: "));
            if (!s) {
                cout << "没有找到该学生。\n";
                continue;
            }

            Student updatedStudent = *s;
            string value;
            value = input("姓名 [" + updatedStudent.getName() + "]: ");
            if (!value.empty()) updatedStudent.setName(value);

            value = input("邮箱 [" + updatedStudent.getEmail() + "]: ");
            if (!value.empty()) updatedStudent.setEmail(value);

            value = input("专业 [" + updatedStudent.getMajor() + "]: ");
            if (!value.empty()) updatedStudent.setMajor(value);

            value = input("班级 [" + updatedStudent.getClassName() + "]: ");
            if (!value.empty()) updatedStudent.setClassName(value);

            value = input("性别 [" + updatedStudent.getSex() + "]: ");
            if (!value.empty()) updatedStudent.setSex(value);

            if (manager.updateStudent(students, updatedStudent))
                cout << "修改并保存完成。\n";
            else
                cout << "修改完成，但保存文件失败。\n";
        } else if (choice == "5") {
            string id = input("请输入要删除的学号: ");
            if (!findStudent(students, id)) {
                cout << "没有找到该学生。\n";
                continue;
            }

            if (manager.removeStudent(students, id))
                cout << "删除并保存完成。\n";
            else
                cout << "删除完成，但保存文件失败。\n";
        } else if (choice == "0") {
            cout << "已退出。\n";
            return 0;
        } else {
            cout << "无效选项。\n";
        }
    }
}