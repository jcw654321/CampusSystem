#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include "student_manager.h"
using namespace std;

static vector<string> splitCsvLine(const string& line) {
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

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    vector<Student> students;
    StudentManager manager("管理员", "admin@example.com", "admin", "123456");

    vector<string> candidatePaths = {
        "./data/student_data.csv",
        "../data/student_data.csv",
        "./student_data.csv",
        "../student_data.csv"
    };

    ifstream inputFile;
    bool opened = false;

    for (const auto& candidate : candidatePaths) {
        inputFile.open(candidate);
        if (inputFile.is_open()) {
            opened = true;
            break;
        }
    }

    if (!opened) {
        cerr << "无法打开文件: student_data.csv" << endl;
        return 1;
    }

    string line;
    if (!getline(inputFile, line)) {
        cerr << "CSV 文件为空" << endl;
        inputFile.close();
        return 1;
    }

    if (!line.empty() && static_cast<unsigned char>(line[0]) == 0xEF &&
        line.size() >= 3 && static_cast<unsigned char>(line[1]) == 0xBB &&
        static_cast<unsigned char>(line[2]) == 0xBF) {
        line.erase(0, 3);
    }

    while (getline(inputFile, line)) {
        if (line.empty()) {
            continue;
        }

        auto fields = splitCsvLine(line);
        if (fields.size() != 8) {
            cerr << "数据格式错误: " << line << endl;
            continue;
        }

        string userid = fields[0];
        string name = fields[1];
        string email = fields[3];
        string password = fields[4];
        string major = fields[5];
        string studentId = fields[6];
        string className = fields[7];

        Student student(name, email, userid, password, major, studentId, className);
        manager.addStudent(students, student);
    }

    inputFile.close();
    manager.printStudentInfo(manager.getStudent(students, "20260001"));
    return 0;
}