#include <graphics.h>
#include <windows.h>
#include <sstream>
#include <string>
#include <vector>
#include "student_manager.h"
#include "subject_manager.h"
#include "student_gui.h"
using namespace std;

namespace
{
enum class StudentPage
{
    Home,
    Courses,
    Scores
};

wstring toWide(const string& text)
{
    if (text.empty())
        return L"";

    int length = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    wstring result(length, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &result[0], length);
    result.pop_back();
    return result;
}

void drawButton(int top, const wchar_t* label)
{
    setfillcolor(WHITE);
    setlinecolor(RGB(205, 212, 224));
    fillroundrect(360, top, 840, top + 60, 12, 12);

    settextstyle(22, 0, L"微软雅黑");
    settextcolor(RGB(0, 63, 136));
    outtextxy(600 - textwidth(label) / 2, top + 19, label);
}

void drawStudentPage(const Student& student, StudentPage page)
{
    setbkcolor(RGB(230, 236, 245));
    cleardevice();

    setfillcolor(RGB(0, 63, 136));
    fillrectangle(0, 0, 1200, 72);
    settextcolor(WHITE);
    settextstyle(26, 0, L"微软雅黑");
    outtextxy(40, 22, L"学生个人中心");

    settextcolor(RGB(0, 63, 136));
    settextstyle(18, 0, L"微软雅黑");
    outtextxy(60, 92, L"<-返回");

    wstring identity =
        toWide(student.getName() + "（" + student.getStudentId() + "）");
    settextcolor(RGB(33, 37, 41));
    settextstyle(24, 0, L"微软雅黑");
    outtextxy(60, 130, identity.c_str());

    if (page == StudentPage::Home)
    {
        drawButton(230, L"查看所选课程");
        drawButton(310, L"查看成绩");
        FlushBatchDraw();
        return;
    }

    const vector<subject>& courses = student.getSubjects();
    if (courses.empty())
    {
        settextstyle(22, 0, L"微软雅黑");
        outtextxy(450, 230, L"暂无课程信息");
        FlushBatchDraw();
        return;
    }

    settextcolor(RGB(33, 37, 41));
    settextstyle(20, 0, L"微软雅黑");
    int y = 195;
    for (const subject& course : courses)
    {
        ostringstream line;
        line << course.getName() << "（" << course.getSubjectId() << "）";
        if (page == StudentPage::Courses)
        {
            line << "  教师：" << course.getTeacherName();
        }
        else if (course.getScore() >= 0)
        {
            line << "  成绩：" << course.getScore();
        }
        else
        {
            line << "  成绩：未录入";
        }

        wstring wideLine = toWide(line.str());
        outtextxy(120, y, wideLine.c_str());
        y += 48;
    }

    FlushBatchDraw();
}
}

void runStudentGui(const string& userid)
{
    StudentManager studentManager("", "", "", "");
    SubjectManager subjectManager("", "", "", "");
    vector<Student> students = studentManager.loadStudentsFromFile();
    subjectManager.loadSubjectsFromfile(students);

    const Student* found = studentManager.findStudentByUserid(students, userid);
    if (found == nullptr)
    {
        MessageBoxW(GetHWnd(), L"没有找到对应的学生。", L"提示", MB_OK);
        return;
    }

    const Student& student = *found;
    StudentPage page = StudentPage::Home;
    drawStudentPage(student, page);

    ExMessage message;
    HWND window = GetHWnd();
    while (IsWindow(window))
    {
        if (!peekmessage(&message, EM_MOUSE | EM_KEY))
        {
            drawStudentPage(student, page);
            Sleep(10);
            continue;
        }

        if (message.message == WM_KEYDOWN && message.vkcode == VK_ESCAPE)
            return;

        if (message.message != WM_LBUTTONDOWN)
            continue;

        if (message.x >= 60 && message.x <= 180 &&
            message.y >= 92 && message.y <= 120)
        {
            if (page == StudentPage::Home)
                return;

            page = StudentPage::Home;
            drawStudentPage(student, page);
            continue;
        }

        if (page != StudentPage::Home ||
            message.x < 360 || message.x > 840)
            continue;

        if (message.y >= 230 && message.y <= 290)
            page = StudentPage::Courses;
        else if (message.y >= 310 && message.y <= 370)
            page = StudentPage::Scores;
        else
            continue;

        drawStudentPage(student, page);
    }
}