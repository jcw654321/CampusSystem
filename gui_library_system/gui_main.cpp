#include <graphics.h>
#include <windows.h>
#include <string>
#include <wchar.h>

// 定义枚举类page表示当前页面类型
enum class Page
{
    Login,
    Select
};
Page page = Page::Login;
std::wstring username;
std::wstring password;
std::wstring tip;
COLORREF tipColor = RGB(200, 40, 40);
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

// 菜单绘制相关函数
void drawLoginPage()
{
    setbkmode(TRANSPARENT);

    // 左侧
    settextcolor(RGB(0, 63, 136));
    settextstyle(40, 0, L"微软雅黑");
    outtextxy(100, 210, L"博学慎思 · 明辨笃行");

    // 阴影
    setfillcolor(RGB(214, 221, 232));
    fillroundrect(764, 164, 1124, 524, 16, 16);

    // 主体
    setfillcolor(RGB(255, 255, 255));
    fillroundrect(760, 160, 1120, 520, 16, 16);

    // 标题
    settextcolor(RGB(33, 37, 41));
    settextstyle(24, 0, L"微软雅黑");
    const wchar_t *title = L"账号登录";
    outtextxy(760 + (360 - textwidth(title)) / 2, 190, title);

    // 输入框1
    setfillcolor(RGB(248, 249, 251));
    fillroundrect(790, 250, 1090, 294, 8, 8);
    setlinecolor(RGB(214, 219, 228));
    roundrect(790, 250, 1090, 294, 8, 8); // 先填充再画边框
    // 输入框2
    fillroundrect(790, 315, 1090, 359, 8, 8);
    roundrect(790, 315, 1090, 359, 8, 8);

    // 输入框中文字
    settextcolor(RGB(150, 156, 168));
    settextstyle(18, 0, L"微软雅黑");
    if (username.empty())
    {
        settextcolor(RGB(150, 156, 168));
        outtextxy(804, 262, L"请输入用户名");
    }
    else
    {
        settextcolor(RGB(33, 37, 41));
        outtextxy(804, 262, username.c_str());
    }

    if (password.empty())
    {
        settextcolor(RGB(150, 156, 168));
        outtextxy(804, 327, L"请输入密码");
    }
    else
    {
        settextcolor(RGB(33, 37, 41));
        std::wstring mask(password.size(), L'*');
        outtextxy(804, 327, mask.c_str());
    }

    // 登录按钮绘制
    setfillcolor(RGB(0, 63, 136));
    fillroundrect(790, 395, 1090, 441, 8, 8);

    settextcolor(RGB(255, 255, 255));
    settextstyle(20, 0, L"微软雅黑");
    const wchar_t *text = L"登  录";
    outtextxy(790 + (300 - textwidth(text)) / 2, 405, text);

    if (!tip.empty()) // 提示登录是否成功,成功绿色失败红色
    {
        settextcolor(tipColor);
        settextstyle(16, 0, L"微软雅黑");
        outtextxy(790, 455, tip.c_str());
    }
}

// 绘制选择界面
void drawCard(int x, int y, const wchar_t *title)
{
   /*  // 阴影,但是加上之后反而看起来不那么好看了,所以暂时不加了qaq
    setfillcolor(RGB(214, 221, 232));
    fillroundrect(x + 4, y + 4, x + 304, y + 244, 16, 16); */

    // 卡片
    setfillcolor(RGB(255, 255, 255));
    fillroundrect(x, y, x + 300, y + 440, 16, 16);

    // 图标(暂时没找到合适的图标就先不做了)

    // 标题
    settextcolor(RGB(0, 63, 136));
    settextstyle(26, 0, L"微软雅黑");
    int twid = textwidth(title);
    outtextxy(x + (300 - twid) / 2, y + 60, title);

    // 进入按钮的绘制
    setfillcolor(RGB(0, 63, 136));
    fillroundrect(x + 70, y + 295, x + 230, y + 331, 8, 8);

    settextcolor(RGB(255, 255, 255));
    settextstyle(18, 0, L"微软雅黑");
    const wchar_t *go = L"进  入";
    int gwid = textwidth(go);
    outtextxy(x + 70 + (160 - gwid) / 2, y + 301, go);
}

void drawSelectPage()
{
    setbkcolor(TRANSPARENT);
    settextcolor(RGB(108, 117, 125));
    settextstyle(20, 0, L"微软雅黑");
    const wchar_t *title = L"请选择要进入的系统";
    outtextxy((1200 - textwidth(title)) / 2, 140, title);

    // 三个系统选择卡片的绘制
    drawCard(110, 235, L"学生管理");
    drawCard(455, 235, L"图书管理");
    drawCard(800, 235, L"银行管理");
}

void drawAll()
{
    cleardevice();
    drawHeader();
    if (page == Page::Login)
    {
        drawLoginPage();
    }
    else if(page==Page::Select)
    {
        drawSelectPage();
    }

    drawFooter();
}

int main()
{
    initgraph(1200, 760);
    SetWindowTextW(GetHWnd(), L"校园综合管理系统");

    setbkcolor(RGB(230, 236, 245));
    cleardevice();

    drawAll();

    ExMessage msg;
    HWND hwnd = GetHWnd();
    while (IsWindow(hwnd))
    {
        if (peekmessage(&msg, EM_MOUSE | EM_KEY))
        {
            if (msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE)
                break;

            if (msg.message == WM_LBUTTONDOWN)
            {
                POINT p = {msg.x, msg.y};
                RECT userR = {790, 250, 1090, 294};
                RECT passR = {790, 315, 1090, 359};
                RECT loginR = {790, 395, 1090, 441};

                if (PtInRect(&userR, p)) // 判断点击是否再用户名输入框里面
                {
                    wchar_t buf[128] = L"";
                    wcsncpy_s(buf, 128, username.c_str(), _TRUNCATE);

                    if (InputBox(buf, 128, L"请输入用户名", L"登录"))
                    {
                        username = buf;
                        tip.clear();
                        drawAll(); // 更新用户名输入栏状态
                    }
                }
                else if (PtInRect(&passR, p)) // 判断点击是否在密码输入框里面
                {
                    wchar_t buf[128] = L"";
                    wcsncpy_s(buf, 128, password.c_str(), _TRUNCATE);

                    if (InputBox(buf, 128, L"请输入密码", L"登录"))
                    {
                        password = buf;
                        tip.clear();
                        drawAll(); // 更新密码输入栏状态
                    }
                }
                else if (PtInRect(&loginR, p))
                {
                    if (username.empty())
                    {
                        tip = L"用户名不能为空";
                        tipColor = RGB(200, 40, 40);
                    }
                    else if (password.empty())
                    {
                        tip = L"密码不能为空";
                        tipColor = RGB(200, 40, 40);
                    }
                    else
                    {
                        page = Page::Select;
                        tip.clear();
                    }
                    drawAll();
                }
            }
        }

        Sleep(10);
    }

    closegraph();
    return 0;
}