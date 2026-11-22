#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

#include "Banksystem.h"
#include <string>

namespace bank_gui
{
constexpr int RowsPerPage = 8;

enum class AdminBankAction
{
    OpenSaving,
    OpenCredit,
    Close,
    QueryBalance
};

enum class StudentBankAction
{
    Deposit,
    Withdraw,
    Transfer,
    QueryBalance
};

template <typename Action>
struct ActionButton
{
    Action action;
    const wchar_t *label;
};

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
