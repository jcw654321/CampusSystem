#include <graphics.h>
#include <windows.h>
#include <string>
#include "fileoperate.h"
#include "student_manager.h"
#include "subject_manager.h"
#include <cstdlib>
#include <sstream>
#include <vector>
using namespace std;

vector<Student> students;
StudentManager studentManager("管理员", "", "admin", "");
SubjectManager subjectManager("管理员", "", "admin", "");

void showText(const string& text);

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

//背景绘制相关函数
void drawHeader()
{
    setfillcolor(RGB(0, 63, 136));
    fillrectangle(0, 0, 1200, 72);

    settextcolor(WHITE);
    settextstyle(26, 0, L"微软雅黑");
    setbkmode(TRANSPARENT);
    outtextxy(40, 22, L"华南理工大学  校园综合管理系统");
}

//底部绘制
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

//菜单绘制
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

//绘制学生操作按钮
void drawStudentActionButtons()
{
    const wchar_t* labels[] = {
        L"获取所有学生信息",
        L"添加学生",
        L"删除学生",
        L"更新学生信息",
        L"查找学生",
        L"管理学生课程"
    };

    const int x = 360;
    const int width = 480;
    const int height = 48;
    const int gap = 8;
    const int startY = 170;

    setbkmode(TRANSPARENT);
    settextstyle(20, 0, L"微软雅黑");
    settextcolor(RGB(0, 63, 136));
    setlinecolor(RGB(205, 212, 224));

    for (int i = 0; i < 6; ++i)
    {
        int y = startY + i * (height + gap);

        setfillcolor(WHITE);
        fillroundrect(x, y, x + width, y + height, 12, 12);

        int textX = x + (width - textwidth(labels[i])) / 2;
        int textY = y + (height - 20) / 2;
        outtextxy(textX, textY, labels[i]);
    }
}

//绘制课程管理菜单
void drawCourseMenu()
{
    const wchar_t* labels[] = {
        L"查看课程", L"添加课程", L"删除课程", L"修改成绩"
    };

    setbkmode(TRANSPARENT);
    settextcolor(RGB(33, 37, 41));
    settextstyle(28, 0, L"微软雅黑");
    outtextxy(60, 130, L"学生课程管理");

    settextcolor(RGB(0, 63, 136));
    settextstyle(18, 0, L"微软雅黑");
    outtextxy(60, 92, L"<-返回");

    for (int i = 0; i < 4; ++i)
    {
        int y = 190 + i * 72;

        setfillcolor(WHITE);
        setlinecolor(RGB(205, 212, 224));
        fillroundrect(360, y, 840, y + 60, 12, 12);

        settextstyle(22, 0, L"微软雅黑");
        int textX = 360 + (480 - textwidth(labels[i])) / 2;
        outtextxy(textX, y + 19, labels[i]);
    }
}

enum class Page { StudentMenu, CourseMenu };

Page currentPage = Page::StudentMenu;
int selectedStudentIndex = -1;

//检测点击的按钮
int hitVerticalButton(int x, int y, int count, int startY = 190)
{
    if (x < 360 || x > 840)
        return -1;

    for (int i = 0; i < count; ++i)
    {
        int top = startY + i * 72;
        if (y >= top && y <= top + 60)
            return i;
    }
    return -1;
}

int hitStudentButton(int x, int y)
{
    const int startY = 170;
    const int height = 48;
    const int pitch = 56;
    if (x < 360 || x > 840)
        return -1;

    for (int i = 0; i < 6; ++i)
    {
        int top = startY + i * pitch;
        if (y >= top && y <= top + height)
            return i;
    }
    return -1;
}

// 查找学生索引
int findStudentIndex(const string& id)
{
    for (int i = 0; i < static_cast<int>(students.size()); ++i)
        if (students[i].getStudentId() == id)
            return i;

    return -1;
}

//绘制当前页面
void drawCurrentPage()
{
    cleardevice();
    drawHeader();
    drawFooter();

    if (currentPage == Page::StudentMenu)
    {
        drawStudentPage();
        drawStudentActionButtons();
    }
    else
    {
        drawCourseMenu();
    }
}

bool inputText(const wchar_t* prompt, string& value)
{
    wchar_t buffer[256] = {};
    if (!InputBox(buffer, 256, prompt, L"学生管理"))
        return false;

    value = toString(wstring(buffer));
    return true;
}

void showText(const string& text)
{
    wstring wide = toWstring(text);
    MessageBoxW(GetHWnd(), wide.c_str(), L"提示", MB_OK);
}

bool parseScore(const string& text, float& score)
{
    char* end = nullptr;
    score = strtof(text.c_str(), &end);
    return end != text.c_str() && *end == '\0' && score >= 0 && score <= 100;
}

void handleStudentAction(int action)
{
    if (action == 0) // 获取所有学生
    {
        ostringstream result;
        for (const Student& s : students)
            result << s.getStudentId() << "  " << s.getName()
                   << "  " << s.getClassName() << '\n';

        showText(result.str().empty() ? "暂无学生数据。" : result.str());
    }
    else if (action == 1) // 添加学生
    {
        string userId, name, sex, email, password, major, id, className;
        if (!inputText(L"用户ID", userId) ||
            !inputText(L"姓名", name) ||
            !inputText(L"性别", sex) ||
            !inputText(L"邮箱", email) ||
            !inputText(L"密码", password) ||
            !inputText(L"专业", major) ||
            !inputText(L"学号", id) ||
            !inputText(L"班级", className))
            return;

        if (id.empty() || findStudentIndex(id) >= 0)
        {
            showText("学号为空或已存在。");
            return;
        }

        Student s(name, email, userId, password, major, id, className);
        s.setSex(sex);
        if (studentManager.addStudent(students, s))
            showText("学生已添加并保存。");
        else
            showText("学生已添加，但保存文件失败。");
    }
    else if (action == 2) // 删除学生
    {
        string id;
        if (!inputText(L"请输入要删除的学号", id)) return;

        if (findStudentIndex(id) < 0)
        {
            showText("没有找到该学生。");
            return;
        }

        if (studentManager.removeStudent(students, id))
            showText("学生已删除并保存。");
        else
            showText("学生已删除，但保存文件失败。");
    }
    else if (action == 3) // 更新学生
    {
        string id;
        if (!inputText(L"请输入要更新的学号", id)) return;

        int index = findStudentIndex(id);
        if (index < 0)
        {
            showText("没有找到该学生。");
            return;
        }

        Student updatedStudent = students[index];
        string value;

        if (!inputText(L"新姓名（留空不变）", value)) return;
        if (!value.empty()) updatedStudent.setName(value);

        if (!inputText(L"新邮箱（留空不变）", value)) return;
        if (!value.empty()) updatedStudent.setEmail(value);

        if (!inputText(L"新专业（留空不变）", value)) return;
        if (!value.empty()) updatedStudent.setMajor(value);

        if (!inputText(L"新班级（留空不变）", value)) return;
        if (!value.empty()) updatedStudent.setClassName(value);

        if (!inputText(L"新性别（留空不变）", value)) return;
        if (!value.empty()) updatedStudent.setSex(value);

        if (studentManager.updateStudent(students, updatedStudent))
            showText("学生信息已更新并保存。");
        else
            showText("学生信息已更新，但保存文件失败。");
    }
    else if (action == 4) // 查找学生
    {
        string id;
        if (!inputText(L"请输入学号", id)) return;

        int index = findStudentIndex(id);
        if (index < 0)
        {
            showText("没有找到该学生。");
            return;
        }

        const Student& s = students[index];
        ostringstream result;
        result << "姓名：" << s.getName()
               << "\n学号：" << s.getStudentId()
               << "\n专业：" << s.getMajor()
               << "\n班级：" << s.getClassName()
               << "\n邮箱：" << s.getEmail();
        showText(result.str());
    }
}

void handleCourseAction(int action, Student& student)
{
    if (action == 0) // 查看课程
    {
        ostringstream result;
        for (const subject& course : student.getSubjects())
            result << course.getName() << "（" << course.getSubjectId()
                   << "），教师：" << course.getTeacherName()
                   << "，成绩：" << course.getScore() << '\n';

        showText(result.str().empty() ? "该学生还没有课程。" : result.str());
    }
    else if (action == 1) // 添加课程
    {
        string name, id, teacher, scoreText;
        if (!inputText(L"课程名称", name) ||
            !inputText(L"课程编号", id) ||
            !inputText(L"教师姓名", teacher) ||
            !inputText(L"成绩（0 到 100）", scoreText))
            return;

        float score;
        if (!parseScore(scoreText, score))
        {
            showText("成绩格式无效，应为 0 到 100。");
            return;
        }

        subjectManager.addSubject(student, subject(name, id, teacher, score));
        showText("课程已添加。");
    }
    else if (action == 2) // 删除课程
    {
        string id;
        if (!inputText(L"请输入要删除的课程编号", id)) return;

        subjectManager.removeSubject(student, id);
        showText("已执行删除。");
    }
    else if (action == 3) // 修改成绩
    {
        string id, scoreText;
        if (!inputText(L"课程编号", id) ||
            !inputText(L"新成绩（0 到 100）", scoreText))
            return;

        float score;
        if (!parseScore(scoreText, score))
        {
            showText("成绩格式无效，应为 0 到 100。");
            return;
        }

        subjectManager.updateScore(student, id, score);
        showText("已执行成绩更新。");
    }
}
int main()
{
    initgraph(1200, 760);
    SetWindowTextW(GetHWnd(), L"校园综合管理系统");

    setbkcolor(RGB(230, 236, 245));
    cleardevice();
    drawHeader();
    drawFooter();
    drawStudentPage();
    drawStudentActionButtons();
    if (!studentManager.loadStudentsFromFile(students))
        showText("无法加载学生数据文件。");
    drawCurrentPage();
    ExMessage msg;
    HWND hwnd = GetHWnd();
    while (IsWindow(hwnd))
    {
        if (!peekmessage(&msg, EM_MOUSE | EM_KEY))
        {
            Sleep(10);
            continue;
        }

        if (msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE)
            break;

        if (msg.message != WM_LBUTTONDOWN)
            continue;

        if (currentPage == Page::StudentMenu)
        {
            int button = hitStudentButton(msg.x, msg.y);

            if (button == 5) // 管理学生课程
            {
                wchar_t buffer[64] = {};
                if (!InputBox(buffer, 64, L"请输入要管理的学生学号"))
                    continue;

                selectedStudentIndex = findStudentIndex(toString(buffer));
                if (selectedStudentIndex < 0)
                {
                    MessageBoxW(GetHWnd(), L"没有找到该学生。", L"提示", MB_OK);
                    continue;
                }

                currentPage = Page::CourseMenu;
                drawCurrentPage();
            }
            else if (button >= 0 && button < 5)
            {
                handleStudentAction(button); // 学生的前五项操作
                drawCurrentPage();
            }
        }
        else
        {
            // 返回按钮
            if (msg.x >= 60 && msg.x <= 180 &&
                msg.y >= 92 && msg.y <= 120)
            {
                currentPage = Page::StudentMenu;
                selectedStudentIndex = -1;
                drawCurrentPage();
                continue;
            }

            int action = hitVerticalButton(msg.x, msg.y, 4);
            if (action >= 0 && selectedStudentIndex >= 0)
            {
                handleCourseAction(action, students[selectedStudentIndex]);
                drawCurrentPage();
            }
        }
    }

    closegraph();
    return 0;
}
