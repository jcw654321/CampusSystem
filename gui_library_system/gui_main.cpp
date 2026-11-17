#include <graphics.h>
#include <windows.h>
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
    line(0, 706, 1200, 706); // 在底部画上一条线

    settextcolor(RGB(120, 128, 140));
    settextstyle(16, 0, L"微软雅黑");
    const wchar_t *text = L"2026 华南理工大学 · 校园综合管理系统";
    int w = textwidth(text);
    outtextxy((1200 - w) / 2, 724, text);
}

int main()
{
    initgraph(1200, 760);
    SetWindowTextW(GetHWnd(), L"校园综合管理系统");

    setbkcolor(RGB(230, 236, 245));
    cleardevice();

    drawHeader();
    drawFooter();

    ExMessage msg;
    HWND hwnd = GetHWnd();
    while (IsWindow(hwnd))
    {
        if (peekmessage(&msg, EM_MOUSE | EM_KEY))
        {
            if (msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE)
                break;
        }
        Sleep(10);
    }

    closegraph();
    return 0;
}