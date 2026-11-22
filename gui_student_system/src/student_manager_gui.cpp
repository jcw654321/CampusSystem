#include <graphics.h>
#include <windows.h>
#include <string>
#include "student_manager.h"
#include "subject_manager.h"
#include "student_manager_gui.h"
#include "course_manager_gui.h"
#include <sstream>
#include <vector>
#include <filesystem>
using namespace std;

namespace
{
    vector<Student> students;
    StudentManager studentManager("管理员", "", "admin", "");
    SubjectManager subjectManager("管理员", "", "admin", "");

    void showText(const string &text);

    // 将UTF-8字符转为easyx支持的宽字符类型
    wstring toWstring(const string &s)
    {
        if (s.empty())
            return L"";
        int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
        wstring w(n, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], n);
        w.pop_back(); // 去掉结尾的\0
        return w;
    }

    // 将宽字符转为string字符,便于LIb调用
    string toString(const wstring &w)
    {
        if (w.empty())
            return "";
        int n = WideCharToMultiByte(
            CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
        string s(n, '\0');
        WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &s[0], n, nullptr, nullptr);
        s.pop_back();
        return s;
    }

    // 背景绘制相关函数
    void drawHeader()
    {
        setfillcolor(RGB(0, 63, 136));
        fillrectangle(0, 0, 1200, 72);

        settextcolor(WHITE);
        settextstyle(26, 0, L"微软雅黑");
        setbkmode(TRANSPARENT);
        outtextxy(40, 22, L"华南理工大学  校园综合管理系统");
    }

    // 底部绘制
    void drawFooter()
    {
        setlinecolor(RGB(205, 212, 224));
        line(0, 706, 1200, 706); // 在底部画上一条线

        settextcolor(RGB(120, 128, 140));
        settextstyle(16, 0, L"微软雅黑");
        const wchar_t *text = L"2026 华南理工大学 · 校园综合管理系统";
        int w = textwidth(text);
        outtextxy((1200 - w) / 2, 724, text);
    }

    // 菜单绘制
    void drawStudentPage()
    {
        setbkmode(TRANSPARENT);

        // 返回
        settextcolor(RGB(0, 63, 136));
        settextstyle(18, 0, L"微软雅黑");
        outtextxy(60, 92, L"<-返回");

        // 标题
        settextcolor(RGB(33, 37, 41));
        settextstyle(28, 0, L"微软雅黑");
        outtextxy(60, 130, L"学生管理");
    }

    // 绘制学生操作按钮
    void drawStudentActionButtons()
    {
        const wchar_t *labels[] = {
            L"获取所有学生信息",
            L"添加学生",
            L"删除学生",
            L"更新学生信息",
            L"查找学生",
            L"按课程查询学生",
            L"成绩排名",
            L"管理学生课程"};

        const int x = 360;
        const int width = 480;
        const int height = 48;
        const int gap = 8;
        const int startY = 170;

        setbkmode(TRANSPARENT);
        settextstyle(20, 0, L"微软雅黑");
        settextcolor(RGB(0, 63, 136));
        setlinecolor(RGB(205, 212, 224));

        for (int i = 0; i < 8; ++i)
        {
            int y = startY + i * (height + gap);

            setfillcolor(WHITE);
            fillroundrect(x, y, x + width, y + height, 12, 12);

            int textX = x + (width - textwidth(labels[i])) / 2;
            int textY = y + (height - 20) / 2;
            outtextxy(textX, textY, labels[i]);
        }
    }

    // 检测鼠标点击的按钮
    int hitStudentButton(int x, int y)
    {
        const int startY = 170;
        const int height = 48;
        const int pitch = 56;
        if (x < 360 || x > 840)
            return -1;

        for (int i = 0; i < 8; ++i)
        {
            int top = startY + i * pitch;
            if (y >= top && y <= top + height)
                return i;
        }
        return -1;
    }

    string trimWhitespace(const string &value)
    {
        const string whitespace = " \t\n\r\f\v";
        size_t first = value.find_first_not_of(whitespace);
        if (first == string::npos)
            return "";

        size_t last = value.find_last_not_of(whitespace);
        return value.substr(first, last - first + 1);
    }

    // 查找学生索引
    int findStudentIndex(const string &id)
    {
        for (int i = 0; i < static_cast<int>(students.size()); ++i)
            if (students[i].getStudentId() == id)
                return i;

        return -1;
    }

    // 绘制当前页面
    void drawCurrentPage()
    {
        cleardevice();
        drawHeader();
        drawFooter();
        drawStudentPage();
        drawStudentActionButtons();

        FlushBatchDraw(); // 整页画完后一次性显示
    }

    // 输入文本
    bool inputText(const wchar_t *prompt, string &value)
    {
        wchar_t buffer[256] = {};
        if (!InputBox(buffer, 256, prompt, L"学生管理"))
            return false;

        value = trimWhitespace(toString(wstring(buffer)));
        if (!value.empty())
            return true;

        showText("输入不能为空，请重新输入。");
        return false;
    }

    // 输入可选文本
    bool inputOptionalText(const wchar_t *prompt, string &value)
    {
        wchar_t buffer[256] = {};
        if (!InputBox(buffer, 256, prompt, L"学生管理"))
            return false;

        string entered = trimWhitespace(toString(wstring(buffer)));
        if (!entered.empty())
            value = entered;
        return true;
    }

    void showText(const string &text)
    {
        wstring wide = toWstring(text);
        MessageBoxW(GetHWnd(), wide.c_str(), L"提示", MB_OK);
    }

    // 输入多项文本
    bool inputFields(const wchar_t *prompt, size_t expectedCount,
                     vector<string> &fields)
    {
        wchar_t buffer[512] = {};
        if (!InputBox(buffer, 512, prompt, L"学生管理"))
            return false;

        istringstream input(toString(wstring(buffer)));
        vector<string> values;
        string value;
        while (input >> value)
            values.push_back(value);

        if (values.size() != expectedCount)
        {
            showText("输入项数量不正确或包含空项，请按提示重新输入。");
            return false;
        }

        fields.swap(values);
        return true;
    }

    void handleStudentAction(int action)
    {
        switch (action)
        {
        case 0:
        {
            ostringstream result;
            for (const Student &s : students)
                result << s.getStudentId() << "  " << s.getName()
                       << "  " << s.getClassName() << '\n';

            showText(result.str().empty() ? "暂无学生数据。" : result.str());
            break;
        }
        case 1:
        {
            vector<string> fields;
            if (!inputFields(
                    L"请按顺序输入8项，以空格分隔：用户ID 姓名 性别 邮箱 密码 专业 学号 班级",
                    8,
                    fields))
            {
                return;
            }

            string userId = fields[0];
            string name = fields[1];
            string sex = fields[2];
            string email = fields[3];
            string password = fields[4];
            string major = fields[5];
            string id = fields[6];
            string className = fields[7];

            if (id.empty() || findStudentIndex(id) >= 0)
            {
                showText("学号为空或已存在。");
                return;
            }

            DataRecord record{
                id,
                {userId, name, sex, email, password, major, id, className}
            };
            if (studentManager.addRecord(record))
            {
                students = studentManager.getStudents();
                showText("学生已添加并保存。");
            }
            else
                showText("学生已添加，但保存文件失败。");
            break;
        }
        case 2:
        {
            string id;
            if (!inputText(L"请输入要删除的学号", id))
                return;

            if (findStudentIndex(id) < 0)
            {
                showText("没有找到该学生。");
                return;
            }

            if (studentManager.removeRecord(id))
            {
                students = studentManager.getStudents();
                showText("学生已删除并保存。");
            }
            else
                showText("学生已删除，但保存文件失败。");
            break;
        }
        case 3:
        {
            string id;
            if (!inputText(L"请输入要更新的学号", id))
                return;

            int index = findStudentIndex(id);
            if (index < 0)
            {
                showText("没有找到该学生。");
                return;
            }

            Student updatedStudent = students[index];
            string userId = updatedStudent.getUserid();
            string name = updatedStudent.getName();
            string sex = updatedStudent.getSex();
            string email = updatedStudent.getEmail();
            string password = updatedStudent.getPassword();
            string major = updatedStudent.getMajor();
            string studentId = updatedStudent.getStudentId();
            string className = updatedStudent.getClassName();
            if (!inputOptionalText(L"用户ID（留空保持不变）", userId) ||
                !inputOptionalText(L"姓名（留空保持不变）", name) ||
                !inputOptionalText(L"性别（留空保持不变）", sex) ||
                !inputOptionalText(L"邮箱（留空保持不变）", email) ||
                !inputOptionalText(L"密码（留空保持不变）", password) ||
                !inputOptionalText(L"专业（留空保持不变）", major) ||
                !inputOptionalText(L"学号（留空保持不变）", studentId) ||
                !inputOptionalText(L"班级（留空保持不变）", className))
                return;

            updatedStudent = Student(name, email, userId, password, major, studentId, className);
            updatedStudent.setSex(sex);

            DataRecord record{
                id,
                {userId, name, sex, email, password, major, studentId, className}
            };
            if (studentManager.updateRecord(id, record))
            {
                students = studentManager.getStudents();
                showText("学生信息已更新并保存。");
            }
            else
                showText("学生信息已更新，但保存文件失败。");
            break;
        }
        case 4:
        {
            string id;
            if (!inputText(L"请输入学号", id))
                return;

            int index = findStudentIndex(id);
            if (index < 0)
            {
                showText("没有找到该学生。");
                return;
            }

            const Student &s = students[index];
            ostringstream result;
            result << "姓名：" << s.getName()
                   << "\n学号：" << s.getStudentId()
                   << "\n专业：" << s.getMajor()
                   << "\n班级：" << s.getClassName()
                   << "\n邮箱：" << s.getEmail();
            showText(result.str());
            break;
        }
        case 5:
        {
            string courseId;
            if (!inputText(L"请输入课程ID", courseId))
                return;

            vector<const Student *> matched =
                subjectManager.findBysubject(students, courseId);

            ostringstream result;
            for (const Student *student : matched)
            {
                result << student->getStudentId() << " "
                       << student->getName() << " "
                       << student->getMajor() << " "
                       << student->getClassName() << '\n';
            }

            showText(result.str().empty()
                         ? "没有学生选修这门课程。"
                         : result.str());
            break;
        }
        case 6:
        {
            vector<StudentAverage> ranked =
                subjectManager.rankByAverageScore(students);

            ostringstream result;
            int rank = 1;
            for (const StudentAverage &item : ranked)
            {
                result << rank++ << ". "
                       << item.student->getStudentId() << " "
                       << item.student->getName() << " 平均分："
                       << item.average << '\n';
            }

            showText(result.str().empty()
                         ? "暂无已录入成绩的学生。"
                         : result.str());
            break;
        }
        }
    }

}

void runStudentManagerGui()
{
    std::filesystem::path oldDir = std::filesystem::current_path();

    std::filesystem::path dataDir = "data/student_data/student_data.csv";
    if (!std::filesystem::exists(dataDir))
    {
        for (const std::filesystem::path &candidate :
             {"../student_manage_system", "student_manage_system"})
        {
            if (std::filesystem::exists(candidate /
                                        "data/student_data/student_data.csv"))
            {
                std::filesystem::current_path(candidate);
                break;
            }
        }
    }

    if (!std::filesystem::exists(
            "data/student_data/student_data.csv"))
    {
        std::filesystem::current_path(oldDir);
        showText("未找到学生数据目录，课程信息无法加载。");
        return;
    }

    if (!studentManager.loadRecords())
        showText("学生数据加载失败。");
    if (!subjectManager.loadRecords())
        showText("课程数据加载失败。");

    students = studentManager.getStudents();
    subjectManager.loadSubjectsFromfile(students);

    drawCurrentPage();

    ExMessage msg;
    HWND hwnd = GetHWnd();
    while (IsWindow(hwnd))
    {
        if (!peekmessage(&msg, EM_MOUSE | EM_KEY))
        {
            drawCurrentPage();
            Sleep(10);
            continue;
        }

        if (msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE)
            break;

        if (msg.message != WM_LBUTTONDOWN)
            continue;

        if (msg.x >= 60 && msg.x <= 180 &&
            msg.y >= 85 && msg.y <= 120)
        {
            break;
        }
        int button = hitStudentButton(msg.x, msg.y);

        if (button == 7)
        {
            string studentId;
            if (!inputText(L"请输入要管理的学生学号", studentId))
                continue;

            int selectedStudentIndex = findStudentIndex(studentId);
            if (selectedStudentIndex < 0)
            {
                MessageBoxW(GetHWnd(), L"没有找到该学生。", L"提示", MB_OK);
                continue;
            }

            runCourseManagerGui(students[selectedStudentIndex], subjectManager);
            drawCurrentPage();
        }
        else if (button >= 0 && button < 7)
        {
            handleStudentAction(button);
            drawCurrentPage();
        }
    }

    std::filesystem::current_path(oldDir);
}
