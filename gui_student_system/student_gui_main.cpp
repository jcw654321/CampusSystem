#include <graphics.h>
#include <string>
#include <vector>
#include "student_manager.h"
#include "subject_manager.h"
#include "student_gui.h"

#ifdef STUDENT_GUI_STANDALONE
int main()
{
    initgraph(1200, 760);
    BeginBatchDraw();
    SetWindowTextW(GetHWnd(), L"校园综合管理系统");

    StudentManager studentManager("管理员", "", "admin", "");
    SubjectManager subjectManager("管理员", "", "admin", "");
    std::vector<Student> students = studentManager.loadStudentsFromFile();
    subjectManager.loadSubjectsFromfile(students);

    const Student previewStudent = students.empty()
        ? Student("学生预览", "", "preview", "", "", "未绑定", "")
        : students.front();
    runStudentGui(previewStudent);

    EndBatchDraw();
    closegraph();
    return 0;
}
#endif
