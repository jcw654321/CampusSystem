#include "gui_bank_internal.h"

#include <graphics.h>
#include <vector>

namespace bank_gui
{
namespace
{
const std::vector<ActionButton<AdminBankAction>> AdminButtons = {
    {AdminBankAction::OpenSaving, L"储蓄开户"},
    {AdminBankAction::OpenCredit, L"信用开户"},
    {AdminBankAction::Close, L"销户"},
    {AdminBankAction::QueryBalance, L"查询余额"}};

// 管理员可以随时开户；销户和余额查询需要先选中一条账户。
void drawAdminActions()
{
    for (int i = 0; i < static_cast<int>(AdminButtons.size()); ++i)
    {
        auto action = AdminButtons[i].action;
        bool opening = action == AdminBankAction::OpenSaving ||
                       action == AdminBankAction::OpenCredit;
        // 开户无需先选账户；销户和余额查询必须针对已选账户。
        drawButton(60 + i * 137, 625, 121, AdminButtons[i].label,
                   opening || hasSelectedAccount());
    }
}

void drawAdminBankPage()
{
    // 管理员页面使用共用表格，再加搜索框和管理员专属按钮。
    drawBankPageFrame(L"银行管理");
    drawButton(680, 125, 140, L"搜索账户");
    drawAdminActions();
    FlushBatchDraw();
}

void handleAdminAction(AdminBankAction action)
{
    bool openingAccount = action == AdminBankAction::OpenSaving ||
                          action == AdminBankAction::OpenCredit;
    const BankAccount *account = getSelectedAccount();
    if (!openingAccount && !account)
    {
        setStatusMessage(L"请先点击表格选择账户。");
        return;
    }
    // 查询只是读取，不需要保存；其他操作开始前先补存上次失败的数据。
    if (action != AdminBankAction::QueryBalance && !retryPendingSave())
        return;

    bool changed = false;
    switch (action)
    {
    case AdminBankAction::OpenSaving:
        changed = openAccount(false);
        break;
    case AdminBankAction::OpenCredit:
        changed = openAccount(true);
        break;
    case AdminBankAction::Close:
        changed = closeSelectedAccount();
        break;
    case AdminBankAction::QueryBalance:
        showAccountBalance(*account);
        return;
    }
    finishBankAction(changed, openingAccount, action == AdminBankAction::Close);
}
}

void runAdminBankFrontend()
{
    drawAdminBankPage();
    const RECT backRect = {60, 90, 260, 120};
    const RECT tableRect = {60, 262, 1140, 262 + RowsPerPage * 38};
    const RECT searchRect = {680, 125, 820, 165};
    HWND window = GetHWnd();
    ExMessage message;

    // 循环等待鼠标事件：返回、选账户、搜索、翻页或点击业务按钮。
    while (IsWindow(window))
    {
        if (!peekmessage(&message, EM_MOUSE))
        {
            Sleep(10);
            continue;
        }

        if (message.message == WM_MOUSEWHEEL)
        {
            scrollPage(message.wheel > 0 ? -RowsPerPage : RowsPerPage);
        }
        else if (message.message == WM_LBUTTONDOWN)
        {
            // 先判断点击的位置属于哪个区域，再执行对应操作。
            POINT point = {message.x, message.y};
            if (PtInRect(&backRect, point))
            {
                if (canLeaveBank())
                    return;
            }
            else if (PtInRect(&tableRect, point))
            {
                selectAccountAt(point.y, tableRect);
            }
            else if (PtInRect(&searchRect, point))
            {
                std::wstring input;
                if (inputText(L"请输入账号或用户号关键字（留空显示全部）", input))
                    applyAccountSearch(input);
            }
            else
            {
                for (int i = 0; i < static_cast<int>(AdminButtons.size()); ++i)
                {
                    int left = 60 + i * 137;
                    RECT actionRect = {left, 625, left + 121, 665};
                    if (PtInRect(&actionRect, point))
                    {
                        handleAdminAction(AdminButtons[i].action);
                        break;
                    }
                }
            }
        }
        drawAdminBankPage();
    }
}
}
