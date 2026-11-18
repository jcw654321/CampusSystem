#include <graphics.h>
#include <windows.h>
#include <string>
#include <vector>
#include <wchar.h>
#include "library_system.h"

// 定义枚举类page表示当前页面类型
enum class Page
{
    Login,
    Select,
    Library
};
Page page = Page::Login;
std::wstring username;
std::wstring password;
std::wstring tip;
COLORREF tipColor = RGB(200, 40, 40);

// 图书管理所用全局变量
LibrarySystem *lib = nullptr;
std::string reason = "";
int selectedBook = -1; // 当前选中书本下标
std::wstring libTip;   // 图书页面底部提示信息
COLORREF libTipColor = RGB(200, 40, 40);
std::vector<int> shownIndex; // 记录表格第i行代表的书在books里面的下标
int scrollFirst = 0;         // 从第几本开始显示(用于滚轮滑动设计)

// 将UTF-8字符转为easyx支持的宽字符类型
std::wstring toWstring(const std::string &s)
{
    if (s.empty())
        return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    std::wstring w(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], n);
    w.pop_back(); // 去掉结尾的\0
    return w;
}

// 将宽字符转为string字符,便于LIb调用
std::string toString(const std::wstring &w)
{
    if (w.empty())
        return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string s(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, &s[0], n, nullptr, nullptr);
    s.pop_back();
    return s;
}

// 重置函数
void resetShown()
{
    shownIndex.clear();

    for (int i = 0; i < lib->getBookCount(); i++)
    {
        shownIndex.push_back(i);
    }

    scrollFirst = 0;
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

// 绘制图书馆里系统界面
void drawLibraryPage()
{
    setbkmode(TRANSPARENT);

    // 返回
    settextcolor(RGB(0, 63, 136));
    settextstyle(18, 0, L"微软雅黑");
    outtextxy(60, 92, L"<-返回");

    // 标题
    settextcolor(RGB(33, 37, 41));
    settextstyle(28, 0, L"微软雅黑");
    outtextxy(60, 130, L"图书管理");

    // 表头
    setfillcolor(RGB(255, 255, 255));
    fillrectangle(60, 180, 1140, 216);

    settextcolor(RGB(90, 110, 115));
    settextstyle(16, 0, L"微软雅黑");
    outtextxy(80, 190, L"编号");
    outtextxy(200, 190, L"书名");
    outtextxy(480, 190, L"作者");
    outtextxy(640, 190, L"分类");
    outtextxy(780, 190, L"总/可借");
    outtextxy(940, 190, L"状态");

    // 数据
    int total = static_cast<int>(shownIndex.size());
    int rows = total - scrollFirst; // 除去已经划过的行后剩余的行数,由于最多显示10本书,所以当他大于10要改为10
    if (rows > 10)
        rows = 10; // 最多显示10本书

    for (int row = 0; row < rows; row++) // 最多显示10本书
    {
        int i = shownIndex[scrollFirst + row]; // 本行书对应真实下标
        int y = 216 + row * 38;

        Book b;
        if (!lib->getBookAtIndex(i, b))
            continue;
        if (i == selectedBook)
        {
            setfillcolor(RGB(220, 235, 255));
        } // 选中行以蓝色标记
        else
            setfillcolor(RGB(255, 255, 255));
        fillrectangle(60, y, 1140, y + 38);

        settextcolor(RGB(33, 37, 41));
        settextstyle(16, 0, L"微软雅黑");

        outtextxy(80, y + 10, toWstring(b.getBookId()).c_str());
        outtextxy(200, y + 10, toWstring(b.getTitle()).c_str());
        outtextxy(480, y + 10, toWstring(b.getAuthor()).c_str());
        outtextxy(640, y + 10, toWstring(b.getCategory()).c_str());

        std::wstring num = std::to_wstring(b.getTotalCopies()) + L" / " + std::to_wstring(lib->availableCopies(b.getBookId()));
        outtextxy(780, y + 10, num.c_str());
        outtextxy(940, y + 10, b.isonshelf() ? L"上架" : L"已下架");
    }
    // 底部,三个按钮借书还书刷新
    setfillcolor(RGB(0, 63, 136));
    fillroundrect(60, 620, 180, 660, 8, 8);
    fillroundrect(200, 620, 320, 660, 8, 8);
    fillroundrect(340, 620, 460, 660, 8, 8);
    fillroundrect(480, 620, 600, 660, 8, 8);

    settextcolor(RGB(255, 255, 255));
    settextstyle(18, 0, L"微软雅黑");

    outtextxy(60 + (120 - textwidth(L"借  书")) / 2, 628, L"借  书");
    outtextxy(200 + (120 - textwidth(L"还  书")) / 2, 628, L"还  书");
    outtextxy(340 + (120 - textwidth(L"刷  新")) / 2, 628, L"刷  新");
    outtextxy(480 + (120 - textwidth(L"搜  索")) / 2, 628, L"搜  索");

    // 提示
    if (!libTip.empty())
    {
        settextcolor(libTipColor);
        settextstyle(16, 0, L"微软雅黑");
        outtextxy(60, 675, libTip.c_str());
    }
}

void drawSelectPage()
{
    setbkmode(TRANSPARENT);
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
    else if (page == Page::Select)
    {
        drawSelectPage();
    }
    else if (page == Page::Library)
    {
        drawLibraryPage();
    }

    drawFooter();
}

int main()
{
    initgraph(1200, 760);
    SetWindowTextW(GetHWnd(), L"校园综合管理系统");

    setbkcolor(RGB(230, 236, 245));
    cleardevice();

    LibrarySystem lib_("data/library.txt");
    lib = &lib_;

    if (!lib->loadData(reason))
    {
        MessageBoxA(GetHWnd(), reason.c_str(), "加载失败", MB_OK);
    }
    resetShown();
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

                if (page == Page::Login)
                {
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
                else if (page == Page::Select)
                {
                    RECT libR = {455, 235, 755, 675};

                    if (PtInRect(&libR, p))
                    {
                        page = Page::Library;
                        selectedBook = -1;
                        libTip.clear();
                        resetShown();
                        drawAll();
                    }
                }
                else if (page == Page::Library)
                {
                    RECT backR = {60, 85, 220, 120};
                    RECT tableR = {60, 216, 1140, 216 + 380};
                    RECT borrowR = {60, 620, 180, 660};
                    RECT returnR = {200, 620, 320, 660};
                    RECT refreshR = {340, 620, 460, 660};
                    RECT searchR = {480, 620, 600, 660};
                    if (PtInRect(&backR, p))
                    {
                        page = Page::Select;
                        drawAll();
                    }
                    else if (PtInRect(&tableR, p))
                    {
                        int total = static_cast<int>(shownIndex.size());
                        int rows = total - scrollFirst;
                        if (rows > 10)
                            rows = 10;
                        int row = (p.y - 216) / 38;
                        if (row >= 0 && row < rows)
                        {
                            selectedBook = shownIndex[scrollFirst + row];
                            drawAll();
                        }
                    }
                    else if (PtInRect(&borrowR, p))
                    {
                        if (selectedBook < 0)
                        {
                            libTip = L"请先选择一本书";
                            libTipColor = RGB(200, 40, 40);
                            drawAll();
                        }
                        else
                        {
                            int loanid = lib->getLoanCount();
                            Book b;
                            lib->getBookAtIndex(selectedBook, b);

                            wchar_t buf[64] = L"";
                            if (InputBox(buf, 64, L"请输入学号", L"借书"))
                            {
                                std::string stuId = toString(buf);
                                if (stuId.empty())
                                {
                                    libTip = L"学号不能为空";
                                    libTipColor = RGB(200, 40, 40);
                                }
                                else if (lib->borrowBook(b.getBookId(), stuId, today(), reason))
                                {
                                    lib->saveData(reason);
                                    if (lib->getLoanCount() > loanid) // 说明保存到数据里了
                                    {
                                        LoanRecord r;
                                        lib->getLoanAtIndex(lib->getLoanCount() - 1, r);
                                        libTip = L"借书成功,借书编号: " + toWstring(r.getLoanId());
                                        libTipColor = RGB(30, 140, 70);
                                    }
                                }
                                else
                                {
                                    libTip = toWstring(reason);
                                    libTipColor = RGB(200, 40, 40);
                                }
                                drawAll();
                            }
                        }
                    }
                    else if (PtInRect(&returnR, p)) // 先查看未归还记录再还书
                    {
                        wchar_t buf[64] = L"";
                        if (InputBox(buf, 64, L"请输入学号"))
                        {
                            std::string stuid = toString(buf);
                            if (stuid.empty())
                            {
                                libTip = L"学号不能为空";
                                libTipColor = RGB(200, 40, 40);
                            }
                            else
                            {
                                std::vector<int> loansOfStIndex = lib->loansOfStudent(stuid);
                                std::wstring w;

                                for (size_t i = 0; i < loansOfStIndex.size(); i++)
                                {
                                    LoanRecord l;
                                    if (!lib->getLoanAtIndex(loansOfStIndex[i], l))
                                        continue;
                                    if (l.isreturned())
                                        continue;

                                    w += toWstring(l.getLoanId()) + L"    " + toWstring(l.getBookId()) + L"    " + toWstring(l.getDuedate()) + L"\r\n";
                                }

                                if (w.empty())
                                {
                                    libTip = L"该学生没有未归还的借阅记录";
                                    libTipColor = RGB(200, 40, 40);
                                }
                                else
                                {
                                    MessageBox(GetHWnd(), w.c_str(), L"未归还记录", MB_OK | MB_ICONINFORMATION);
                                    wchar_t buf1[32] = L"";
                                    if (InputBox(buf1, 32, L"请输入借阅编号", L"还书"))
                                    {
                                        std::string loanId = toString(buf1);
                                        bool f = false; // 用于判断这个借阅编号是否属于该学生
                                        for (int idx : loansOfStIndex)
                                        {
                                            LoanRecord lr;
                                            lib->getLoanAtIndex(idx, lr);
                                            if (!lr.isreturned() && lr.getLoanId() == loanId)
                                            {
                                                f = true;
                                                break;
                                            }
                                        }
                                        if (!f)
                                        {
                                            libTip = L"该编号不属于该学生的借阅记录";
                                            libTipColor = RGB(200, 40, 40);
                                        }
                                        else if (lib->returnBook(loanId, today(), reason))
                                        {
                                            lib->saveData(reason);
                                            libTip = L"还书成功";
                                            libTipColor = RGB(30, 140, 70);
                                        }
                                        else
                                        {
                                            libTip = toWstring(reason);
                                            libTipColor = RGB(200, 40, 40);
                                        }
                                    }
                                }
                            }
                            drawAll();
                        }
                    }
                    else if (PtInRect(&refreshR, p))
                    {
                        lib->loadData(reason);
                        resetShown();
                        selectedBook = -1;
                        scrollFirst = 0;
                        libTip = L"已刷新";
                        libTipColor = RGB(30, 140, 70);
                        drawAll();
                    }
                    else if (PtInRect(&searchR, p))
                    {
                        wchar_t buf[64] = L"";
                        if (InputBox(buf, 64, L"请输入搜索关键词", L"搜索"))
                        {
                            std::string key = toString(buf);
                            std::vector<int> matchedBooks = lib->searchByTitle(key);
                            shownIndex.clear();
                            for (size_t i = 0; i < matchedBooks.size(); i++)
                            {
                                shownIndex.push_back(matchedBooks[i]);
                            }
                            scrollFirst = 0;
                            selectedBook = -1;
                            if (matchedBooks.empty())
                            {
                                libTip = L"未找到匹配的书";
                                libTipColor = RGB(200, 40, 40);
                            }
                            else
                            {
                                libTip = L"找到 " + std::to_wstring(matchedBooks.size()) + L" 本";
                                libTipColor = RGB(30, 140, 70);
                            }
                        }
                        drawAll();
                    }
                }
            }

            if (msg.message == WM_MOUSEWHEEL && page == Page::Library)
            {
                int total = static_cast<int>(shownIndex.size()); // 当前列表行数
                int maxTop = total > 10 ? total - 10 : 0;        // 最多能划到的行数,比如列表有23行,0-22,那么最多只能划到第13行,再往下划就显示不满10行了
                if (msg.wheel > 0 && scrollFirst > 0)
                {
                    scrollFirst--;
                }
                else if (msg.wheel < 0 && scrollFirst < maxTop)
                {
                    scrollFirst++;
                }
                drawAll();
            }
        }
        Sleep(10);
    }
    closegraph();
    return 0;
}