#include "gui_bank_internal.h"

#include <graphics.h>
#include <vector>

namespace bank_gui
{
namespace
{
const std::vector<ActionButton<StudentBankAction>> StudentButtons = {
    {StudentBankAction::Deposit, L"存款"},
    {StudentBankAction::Withdraw, L"取款"},
    {StudentBankAction::Transfer, L"转账"},
    {StudentBankAction::QueryBalance, L"查询余额"}};

void drawStudentActions()
{
    for (int i = 0; i < static_cast<int>(StudentButtons.size()); ++i)
    {
        drawButton(60 + i * 137, 625, 121, StudentButtons[i].label,
                   hasSelectedAccount());
    }
}

void drawStudentBankPage()
{
    drawBankPageFrame(L"我的银行");
    drawStudentActions();
    FlushBatchDraw();
}

void handleStudentAction(StudentBankAction action)
{
    // 后端检查账户归属和状态；管理员不能通过学生界面代办业务。
    if (!canOperateSelectedAccount())
    {
        setStatusMessage(L"请先选择本人的银行账户。");
        return;
    }

    const BankAccount *account = getSelectedAccount();
    if (action == StudentBankAction::QueryBalance)
    {
        showAccountBalance(*account);
        return;
    }
    // 只读查询不依赖保存；会修改余额的业务必须先重试之前的保存。
    if (!retryPendingSave())
        return;

    finishBankAction(performTransaction(action), false, false);
}
}

void runStudentBankFrontend()
{
    drawStudentBankPage();
    const RECT backRect = {60, 90, 260, 120};
    const RECT tableRect = {60, 262, 1140, 262 + RowsPerPage * 38};
    HWND window = GetHWnd();
    ExMessage message;

    // 学生端不提供搜索，只响应本人账户选择、操作按钮和滚轮翻页。
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
            else
            {
                for (int i = 0; i < static_cast<int>(StudentButtons.size()); ++i)
                {
                    int left = 60 + i * 137;
                    RECT actionRect = {left, 625, left + 121, 665};
                    if (PtInRect(&actionRect, point))
                    {
                        handleStudentAction(StudentButtons[i].action);
                        break;
                    }
                }
            }
        }
        drawStudentBankPage();
    }
}
}
