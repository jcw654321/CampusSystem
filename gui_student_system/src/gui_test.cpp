#include <graphics.h>
#include "student_manager_gui.h"

int main()
{
    initgraph(1200, 760);
    BeginBatchDraw();
    SetWindowTextW(GetHWnd(), L"校园综合管理系统 - GUI测试");

    runStudentManagerGui();

    EndBatchDraw();
    closegraph();
    return 0;
}
