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
    // 查询余额不修改账户文件；其他操作先解决上次未保存的数据。
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

    // 管理员只在本循环处理自己的按钮、搜索和账户列表事件。
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
