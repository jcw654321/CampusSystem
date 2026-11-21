#include <graphics.h>
#include <windows.h>
#include <string>
#include <sstream>
#include "student.h"
#include "subject.h"
#include "subject_manager.h"
#include "course_manager_gui.h"

using namespace std;

namespace
{
wstring toWstring(const string &s)
{
    if (s.empty())
        return L"";

    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], n);
    w.pop_back();
    return w;
}

string toString(const wstring &w)
{
    if (w.empty())
        return "";

    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &s[0], n, nullptr, nullptr);
    s.pop_back();
    return s;
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

void drawHeader()
{
    setfillcolor(RGB(0, 63, 136));
    fillrectangle(0, 0, 1200, 72);

    settextcolor(WHITE);
    settextstyle(26, 0, L"微软雅黑");
    setbkmode(TRANSPARENT);
    outtextxy(40, 22, L"华南理工大学  校园综合管理系统");
}

void drawFooter()
{
    setlinecolor(RGB(205, 212, 224));
    line(0, 706, 1200, 706);

    settextcolor(RGB(120, 128, 140));
    settextstyle(16, 0, L"微软雅黑");
    const wchar_t *text = L"2026 华南理工大学 · 校园综合管理系统";
    int w = textwidth(text);
    outtextxy((1200 - w) / 2, 724, text);
}

void showText(const string &text)
{
    wstring wide = toWstring(text);
    MessageBoxW(GetHWnd(), wide.c_str(), L"提示", MB_OK);
}

bool inputText(const wchar_t *prompt, string &value)
{
    while (true)
    {
        wchar_t buffer[256] = {};
        if (!InputBox(buffer, 256, prompt, L"课程管理"))
            return false;

        value = trimWhitespace(toString(wstring(buffer)));
        if (!value.empty())
            return true;

        showText("输入不能为空，请重新输入。");
    }
}

bool parseScore(const string &text, float &score)
{
    char *end = nullptr;
    score = strtof(text.c_str(), &end);
    return end != text.c_str() && *end == '\0' && score >= 0 && score <= 100;
}

void saveStudentChanges(Student &student, const Student &updatedStudent,
                        const SubjectManager &subjectManager,
                        const string &successMessage, const string &failureMessage)
{
    if (!subjectManager.saveSubjectsToFile(updatedStudent))
    {
        showText(failureMessage);
        return;
    }

    student = updatedStudent;
    showText(successMessage);
}

void drawCoursePage(const Student &student)
{
    cleardevice();
    drawHeader();
    drawFooter();

    setbkmode(TRANSPARENT);
    settextcolor(RGB(0, 63, 136));
    settextstyle(18, 0, L"微软雅黑");
    outtextxy(60, 92, L"<-返回");

    settextcolor(RGB(33, 37, 41));
    settextstyle(28, 0, L"微软雅黑");
    outtextxy(60, 130, L"学生课程管理");

    const wchar_t *labels[] = {
        L"查看课程", L"添加课程", L"删除课程", L"修改成绩"};

    settextcolor(RGB(0, 63, 136));
    setlinecolor(RGB(205, 212, 224));
    for (int i = 0; i < 4; ++i)
    {
        int y = 190 + i * 72;
        setfillcolor(WHITE);
        fillroundrect(360, y, 840, y + 60, 12, 12);

        settextstyle(22, 0, L"微软雅黑");
        int textX = 360 + (480 - textwidth(labels[i])) / 2;
        outtextxy(textX, y + 19, labels[i]);
    }

    FlushBatchDraw();
}

int hitCourseButton(int x, int y)
{
    if (x < 360 || x > 840)
        return -1;

    for (int i = 0; i < 4; ++i)
    {
        int top = 190 + i * 72;
        if (y >= top && y <= top + 60)
            return i;
    }
    return -1;
}

void handleCourseAction(int action, Student &student, SubjectManager &subjectManager)
{
    if (action == 0)
    {
        ostringstream result;
        for (const subject &course : student.getSubjects())
            result << course.getName() << "（" << course.getSubjectId()
                   << "），教师：" << course.getTeacherName()
                   << "，成绩：" << course.getScore() << '\n';

        showText(result.str().empty() ? "该学生还没有课程。" : result.str());
    }
    else if (action == 1)
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

        if (subjectManager.hasSubject(student, id))
        {
            showText("该学生已存在此课程编号。");
            return;
        }

        Student updatedStudent = student;
        subjectManager.addSubject(updatedStudent, subject(name, id, teacher, score));
        saveStudentChanges(student, updatedStudent, subjectManager,
                           "课程已添加并保存。", "保存课程失败，未更改学生课程。");
    }
    else if (action == 2)
    {
        string id;
        if (!inputText(L"请输入要删除的课程编号", id))
            return;

        if (!subjectManager.hasSubject(student, id))
        {
            showText("该学生未选修此课程。");
            return;
        }

        Student updatedStudent = student;
        subjectManager.removeSubject(updatedStudent, id);
        saveStudentChanges(student, updatedStudent, subjectManager,
                           "课程已删除并保存。", "保存课程失败，未更改学生课程。");
    }
    else if (action == 3)
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

        if (!subjectManager.hasSubject(student, id))
        {
            showText("该学生未选修此课程。");
            return;
        }

        Student updatedStudent = student;
        subjectManager.updateScore(updatedStudent, id, score);
        saveStudentChanges(student, updatedStudent, subjectManager,
                           "成绩已更新并保存。", "保存成绩失败，未更改学生课程。");
    }
}
}

void runCourseManagerGui(Student &student, SubjectManager &subjectManager)
{
    drawCoursePage(student);

    ExMessage msg;
    HWND hwnd = GetHWnd();
    while (IsWindow(hwnd))
    {
        if (!peekmessage(&msg, EM_MOUSE | EM_KEY))
        {
            drawCoursePage(student);
            Sleep(10);
            continue;
        }

        if (msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE)
            return;

        if (msg.message != WM_LBUTTONDOWN)
            continue;

        if (msg.x >= 60 && msg.x <= 180 &&
            msg.y >= 92 && msg.y <= 120)
        {
            return;
        }

        int action = hitCourseButton(msg.x, msg.y);
        if (action >= 0)
        {
            handleCourseAction(action, student, subjectManager);
            drawCoursePage(student);
        }
    }
}
