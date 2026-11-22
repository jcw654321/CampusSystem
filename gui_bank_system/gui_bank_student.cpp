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

// 学生操作按钮只有选中账户后才可用。
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
    // 学生页面显示本人账户列表和存取款、转账、查询按钮。
    drawBankPageFrame(L"我的银行");
    drawStudentActions();
    FlushBatchDraw();
}

void handleStudentAction(StudentBankAction action)
{
    // 先确认选中的是本人可操作的账户；后端也会再次检查。
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
    // 查询余额不改数据；存款、取款和转账会改数据，所以先处理之前的保存失败。
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

    // 循环等待鼠标事件：返回、选本人账户、翻页或点击业务按钮。
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
            // 根据点击位置判断用户是在返回、选账户还是执行某项业务。
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
