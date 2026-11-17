#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include "student_manager.h"

using namespace std;

const string DATA_FILE = ".\\data\\student_data\\student_data.csv";

string input(const string& prompt) {
    cout << prompt;
    string value;
    getline(cin, value);
    return value;
}

vector<string> split(const string& line) {
    vector<string> fields;
    string field;

    for (char ch : line) {
        if (ch == ',') {
            fields.push_back(field);
            field.clear();
        } else {
            field += ch;
        }
    }
    fields.push_back(field);
    return fields;
}

Student* findStudent(vector<Student>& students, const string& id) {
    for (Student& student : students) {
        if (student.getStudentId() == id) {
            return &student;
        }
    }
    return nullptr;
}

bool loadStudents(vector<Student>& students, StudentManager& manager) {
    ifstream file(DATA_FILE);
    if (!file) {
        cerr << "无法打开数据文件: " << DATA_FILE << endl;
        return false;
    }

    string line;
    getline(file, line); // 跳过表头

    while (getline(file, line)) {
        vector<string> f = split(line);
        if (f.size() != 8) {
            cerr << "跳过格式错误的数据行。\n";
            continue;
        }

        Student student(f[1], f[3], f[0], f[4], f[5], f[6], f[7]);
        student.setSex(f[2]);
        manager.addStudent(students, student);
    }
    return true;
}

bool saveStudents(const vector<Student>& students) {
    ofstream file(DATA_FILE);
    if (!file) {
        cerr << "无法保存数据文件: " << DATA_FILE << endl;
        return false;
    }

    file << "用户ID,用户名,性别,邮箱,密码,专业,学号,班级\n";
    for (const Student& s : students) {
        file << s.getUserid() << ','
             << s.getName() << ','
             << s.getSex() << ','
             << s.getEmail() << ','
             << s.getPassword() << ','
             << s.getMajor() << ','
             << s.getStudentId() << ','
             << s.getClassName() << '\n';
    }
    return true;
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

    if (!loadStudents(students, manager)) {
        return 1;
    }

    while (true) {
        cout << "\n===== 学生管理系统 =====\n"
             << "1. 查看所有学生\n"
             << "2. 查询学生\n"
             << "3. 新增学生\n"
             << "4. 修改学生\n"
             << "5. 删除学生\n"
             << "0. 保存退出\n";

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
            manager.addStudent(students, s);
            saveStudents(students);
            cout << "新增完成。\n";
        } else if (choice == "4") {
            Student* s = findStudent(students, input("请输入要修改的学号: "));
            if (!s) {
                cout << "没有找到该学生。\n";
                continue;
            }

            string value;
            value = input("姓名 [" + s->getName() + "]: ");
            if (!value.empty()) s->setName(value);

            value = input("邮箱 [" + s->getEmail() + "]: ");
            if (!value.empty()) s->setEmail(value);

            value = input("专业 [" + s->getMajor() + "]: ");
            if (!value.empty()) s->setMajor(value);

            value = input("班级 [" + s->getClassName() + "]: ");
            if (!value.empty()) s->setClassName(value);

            value = input("性别 [" + s->getSex() + "]: ");
            if (!value.empty()) s->setSex(value);

            saveStudents(students);
            cout << "修改完成。\n";
        } else if (choice == "5") {
            string id = input("请输入要删除的学号: ");
            if (!findStudent(students, id)) {
                cout << "没有找到该学生。\n";
                continue;
            }

            manager.removeStudent(students, id);
            saveStudents(students);
            cout << "删除完成。\n";
        } else if (choice == "0") {
            if (saveStudents(students)) {
                cout << "已保存并退出。\n";
                return 0;
            }
        } else {
            cout << "无效选项。\n";
        }
    }
}