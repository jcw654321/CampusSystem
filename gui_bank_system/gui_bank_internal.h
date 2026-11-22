#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include "Banksystem.h"
#include <string>

namespace bank_gui
{
// 管理员页面和学生页面共用的设置、操作类型和函数声明。
constexpr int RowsPerPage = 8;

// 管理员页面上的四种操作。
enum class AdminBankAction
{
    OpenSaving,
    OpenCredit,
    Close,
    QueryBalance
};

// 普通用户页面上的四种操作。
enum class StudentBankAction
{
    Deposit,
    Withdraw,
    Transfer,
    QueryBalance
};

// 把“实际操作”和“按钮文字”放在一起，方便用同一套代码绘制和处理按钮。
template <typename Action>
struct ActionButton
{
    Action action;        // 点击按钮后要执行的操作
    const wchar_t *label; // 按钮上显示的文字
};

// 页面绘制、账户列表、输入保存和业务操作的共用函数。
void drawButton(int x, int y, int width, const wchar_t *label, bool enabled = true);
void drawBankPageFrame(const wchar_t *title);
void refreshVisibleRows();
void applyAccountSearch(const std::wstring &keyword);
void scrollPage(int offset);
void selectAccountAt(int y, const RECT &tableRect);
void setStatusMessage(const std::wstring &text, bool error = true);
bool inputText(const wchar_t *prompt, std::wstring &value);
bool saveBankData();
bool retryPendingSave();
bool hasSelectedAccount();
const BankAccount *getSelectedAccount();
bool canOperateSelectedAccount();
void finishBankAction(bool changed, bool openingAccount, bool closingAccount);
bool openAccount(bool creditAccount);
bool closeSelectedAccount();
void showAccountBalance(const BankAccount &account);
bool performTransaction(StudentBankAction type);
bool canLeaveBank();

void runAdminBankFrontend();
void runStudentBankFrontend();
}
