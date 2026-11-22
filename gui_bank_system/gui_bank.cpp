#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>
#include <stdexcept>

#define NOMINMAX
#include <graphics.h>
#include <windows.h>

#include "Banksystem.h"
#include "CreditAccount.h"
#include "gui_bank.h"
#include "gui_bank_internal.h"

using namespace std;

// 阅读顺序：从 runBankGui 看角色分流，再看管理员和学生各自的前端文件。
// GUI 只负责输入、显示和调用后端；账户规则在 bank_manage_system 中。

// 一、金额转换：界面输入“元”，业务层和文件一律使用整数“分”。
// 从十进制文本直接解析“分”，避免浮点数舍入；不接受符号、指数或空白。
inline bool parseMoney(const wstring &text, int64_t &cents)
{
    if (text.empty())
    {
        return false;
    }

    int64_t value = 0;
    bool hasDecimalPoint = false;
    int decimalDigits = 0;
    int integerDigits = 0;
    const auto maxCents = (numeric_limits<int64_t>::max)();

    // 逐位累积数字：12.50 先得到 1250；不足两位小数时在循环后补零。
    for (wchar_t c : text)
    {
        if (c == L'.' && !hasDecimalPoint && integerDigits > 0)
        {
            hasDecimalPoint = true;
            continue;
        }

        if (c < L'0' || c > L'9')
        {
            return false;
        }

        if (hasDecimalPoint)
        {
            ++decimalDigits;
            if (decimalDigits > 2)
            {
                return false;
            }
        }
        else
        {
            ++integerDigits;
        }

        int digit = c - L'0';
        // 先检查再乘 10，避免整数溢出。
        if (value > (maxCents - digit) / 10)
        {
            return false;
        }

        value = value * 10 + digit;
    }

    if (hasDecimalPoint && decimalDigits == 0)
    {
        return false;
    }

    for (int i = decimalDigits; i < 2; ++i)
    {
        if (value > maxCents / 10)
        {
            return false;
        }

        value *= 10;
    }

    cents = value;
    return value > 0;
}

inline wstring moneyText(int64_t cents)
{
    // 信用账户可能是负余额。
    // 同样支持 INT64_MIN，不能直接对有符号数取负。
    auto magnitude = cents < 0 ? uint64_t(-(cents + 1)) + 1 : uint64_t(cents);
    auto fraction = magnitude % 100;
    return (cents < 0 ? L"-" : L"") + to_wstring(magnitude / 100) + L"." +
           (fraction < 10 ? L"0" : L"") + to_wstring(fraction);
}

namespace bank_gui
{
// 二、界面配置与状态。以下变量仅在本文件使用。
constexpr COLORREF Blue = RGB(0, 63, 136);
constexpr COLORREF Ink = RGB(33, 37, 41);
constexpr COLORREF Muted = RGB(108, 117, 125);
constexpr array<int, 3> ColumnX = {100, 440, 780};
constexpr array<int, 3> ColumnWidths = {280, 280, 250};

// 每次进入银行，用主系统传入的用户创建业务对象。
unique_ptr<Banksystem> bank;
// 只缓存后端允许当前用户查看的内容，不持有全体用户的可修改账户容器。
vector<const BankAccount*> visibleAccounts;
bool hasUnsavedChanges = false; // 保存失败时保留结果，下次操作或退出时自动重试
int firstVisibleRow = 0;        // 当前页第一条记录在 visibleIndices 中的位置
string selectedAccountId;
wstring searchKeyword;
wstring statusMessage;
COLORREF statusColor = Muted;
// 保存原容器的下标；搜索和分页只改变显示顺序，不移动实际账户。
vector<size_t> visibleIndices;

bool hasSelectedAccount()
{
    return !selectedAccountId.empty();
}

const BankAccount *getSelectedAccount()
{
    return bank->findAccount(selectedAccountId);
}

bool canOperateSelectedAccount()
{
    return bank->canOperateAccount(selectedAccountId);
}

// 三、文字转换、输入对话框与保存。
wstring toWide(const string &text)
{
    if (text.empty())
    {
        return L"";
    }

    // 后端和文件使用 UTF-8 字节串，EasyX 控件使用宽字符串。
    int length =
        MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    wstring result(length, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(),
                        length);
    return result;
}

string toUtf8(const wstring &text)
{
    if (text.empty())
    {
        return "";
    }

    // 输入框返回宽字符串，传给后端前统一转换成 UTF-8。
    int length = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                                     nullptr, 0, nullptr, nullptr);
    string result(length, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(),
                        length, nullptr, nullptr);
    return result;
}

void setStatusMessage(const wstring &text, bool error)
{
    statusMessage = text;
    statusColor = error ? RGB(190, 45, 45) : RGB(30, 125, 80);
}

bool inputText(const wchar_t *prompt, wstring &value)
{
    wchar_t buffer[128] = {};
    // 用户取消输入时不修改输出参数，调用方可直接结束当前操作。
    if (!InputBox(buffer, 128, prompt, L"银行管理"))
    {
        return false;
    }

    value = buffer;
    return true;
}

bool inputId(const wchar_t *prompt, string &value)
{
    wstring input;
    if (!inputText(prompt, input))
    {
        return false;
    }

    if (input.empty() || input.size() > 32 ||
        input.find_first_not_of(
            L"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-") !=
            wstring::npos)
    {
        setStatusMessage(L"编号须为 1–32 位字母、数字、下划线或短横线。");
        return false;
    }

    value = toUtf8(input);
    return true;
}

bool inputAmount(const wchar_t *prompt, int64_t &amount)
{
    wstring input;
    if (!inputText(prompt, input))
    {
        return false;
    }

    if (!parseMoney(input, amount))
    {
        setStatusMessage(L"请输入大于 0 的金额，最多两位小数，且不得超出范围。");
        return false;
    }

    return true;
}

// 复用后端文件格式；失败时保留内存结果，修改账户的业务和退出前会重试。
bool saveBankData()
{
    bool accountsOk = bank->saveAccounts();
    hasUnsavedChanges = !accountsOk;

    if (hasUnsavedChanges)
    {
        setStatusMessage(L"自动保存失败，暂勿离开；下次修改账户或退出银行时会重试。");
    }
    else
    {
        setStatusMessage(L"账户已保存。", false);
    }

    return !hasUnsavedChanges;
}

bool retryPendingSave()
{
    return !hasUnsavedChanges || saveBankData();
}

// 四、生成显示列表：账户按关键字筛选。
void refreshVisibleRows()
{
    // 后端先按身份筛出可见账户；此处再按关键字生成显示索引，不改动账户集合。
    visibleAccounts = bank->getAccounts();
    visibleIndices.clear();
    for (size_t i = 0; i < visibleAccounts.size(); ++i)
    {
        const auto &account = visibleAccounts[i];
        bool matchesKeyword =
            searchKeyword.empty() ||
            toWide(account->getAccountId()).find(searchKeyword) != wstring::npos ||
            toWide(account->getOwnerUserId()).find(searchKeyword) != wstring::npos;
        if (matchesKeyword)
        {
            visibleIndices.push_back(i);
        }
    }

    int lastPageStart =
        visibleIndices.empty()
            ? 0
            : static_cast<int>((visibleIndices.size() - 1) / RowsPerPage) * RowsPerPage;
    firstVisibleRow = min(firstVisibleRow, lastPageStart);
}

void scrollPage(int offset)
{
    // 将起始行限制在有效页范围内，避免滚动越过列表首尾。
    int lastPageStart = visibleIndices.empty()
                            ? 0
                            : static_cast<int>((visibleIndices.size() - 1) / RowsPerPage) *
                                  RowsPerPage;
    firstVisibleRow = max(0, min(firstVisibleRow + offset, lastPageStart));
}

// 五、绘图：基础控件 → 表格和按钮 → 完整页面。
void drawText(int x, int y, const wstring &s, int size = 18, COLORREF color = Ink)
{
    settextstyle(size, 0, L"微软雅黑");
    settextcolor(color);
    outtextxy(x, y, s.c_str());
}

void drawCell(int x, int y, int width, wstring value)
{
    settextstyle(16, 0, L"微软雅黑");
    if (textwidth(value.c_str()) > width)
    {
        while (!value.empty() && textwidth((value + L"…").c_str()) > width)
        {
            value.pop_back();
        }

        value += L"…";
    }

    drawText(x, y, value, 16);
}

void drawButton(int x, int y, int width, const wchar_t *label, bool enabled)
{
    setfillcolor(enabled ? Blue : RGB(164, 177, 194));
    setlinecolor(enabled ? Blue : RGB(164, 177, 194));
    fillroundrect(x, y, x + width, y + 40, 8, 8);
    settextstyle(18, 0, L"微软雅黑");
    drawText(x + (width - textwidth(label)) / 2, y + 9, label, 18, WHITE);
}

// 绘图按“表头、数据行、按钮”分块；标题和页脚直接放在 draw 中。

void drawTableHeader()
{
    setfillcolor(RGB(245, 248, 252));
    solidrectangle(60, 222, 1140, 262);

    const wchar_t *accountHeaders[] = {L"账号", L"关联用户号", L"账户类型"};

    for (int column = 0; column < 3; ++column)
    {
        drawText(ColumnX[column], 233, accountHeaders[column], 16, Muted);
    }
}

void drawRows()
{
    for (int row = 0; row < RowsPerPage; ++row)
    {
        int y = 262 + row * 38;
        int position = firstVisibleRow + row;
        bool exists = position < static_cast<int>(visibleIndices.size());
        bool active = exists &&
                      visibleAccounts[visibleIndices[position]]->getAccountId() == selectedAccountId;

        setfillcolor(active ? RGB(220, 235, 255) : WHITE);
        solidrectangle(60, y, 1140, y + 38);
        if (!exists)
        {
            continue;
        }

        size_t index = visibleIndices[position];
        // 在这里直接准备本行三列的数据，不再经过多个行绘制函数。
        array<wstring, 3> values;
        const auto& account = visibleAccounts[index];
        values =
        {
            toWide(account->getAccountId()), toWide(account->getOwnerUserId()),
            dynamic_cast<const CreditAccount*>(account) ? L"信用账户" : L"储蓄账户"
        };
        for (size_t column = 0; column < values.size(); ++column)
        {
            drawCell(ColumnX[column], y + 10, ColumnWidths[column], values[column]);
        }
    }
}

void drawBankPageFrame(const wchar_t *title)
{
    cleardevice();
    setbkmode(TRANSPARENT);
    setfillcolor(Blue);
    solidrectangle(0, 0, 1200, 72);
    drawText(40, 22, L"华南理工大学  校园综合管理系统", 26, WHITE);
    drawText(60, 92, L"< 退出银行管理", 18, Blue);
    drawText(60, 130, title, 28);
    drawTableHeader();
    drawRows();
    if (!statusMessage.empty())
        drawText(60, 678, statusMessage, 16, statusColor);
    setlinecolor(RGB(205, 212, 224));
    line(0, 706, 1200, 706);
    drawText(405, 724, L"2026 华南理工大学 · 校园综合管理系统", 16, RGB(120, 128, 140));
}

// 六、业务按钮：收集输入后调用 Banksystem，不在界面里计算余额。
bool openAccount(bool creditAccount)
{
    string ownerUserId;
    if (!inputId(L"请输入关联用户号（账号自动生成）", ownerUserId)) return false;
    // 前端提前给出明确错误；后端开户函数仍会再次校验用户和管理员权限。
    if (!bank->userExists(ownerUserId))
    {
        setStatusMessage(L"未找到该用户，无法开户。");
        return false;
    }

    string accountId;
    if (creditAccount)
    {
        accountId = bank->openCreditAccount(ownerUserId);
    }
    else
    {
        accountId = bank->openSavingAccount(ownerUserId);
    }
    if (accountId.empty())
    {
        setStatusMessage(L"开户失败：该用户已有同类账户，或账号不符合要求。");
        return false;
    }

    selectedAccountId = accountId;
    searchKeyword.clear();
    return true;
}

bool closeSelectedAccount()
{
    const auto* account = bank->findAccount(selectedAccountId);
    if (!account) return false;
    // 提前提示余额不为零的原因；后端仍独立强制执行销户条件。
    if (account->getBalance() != 0)
    {
        setStatusMessage(L"余额必须为 0 才能销户，请先取出余额或还清欠款。");
        return false;
    }
    wstring prompt = L"确定注销账户 " + toWide(selectedAccountId) +
                     L" 吗？\n销户后不能交易。";
    if (MessageBoxW(GetHWnd(), prompt.c_str(), L"确认销户", MB_YESNO | MB_ICONQUESTION) != IDYES)
        return false;
    if (!bank->closeAccount(selectedAccountId))
    {
        setStatusMessage(L"销户失败，请检查权限和账户余额。");
        return false;
    }
    selectedAccountId.clear();
    return true;
}

void showAccountBalance(const BankAccount &account)
{
    wstring info = L"账号：" + toWide(account.getAccountId()) +
                   L"\n余额：" + moneyText(account.getBalance()) + L" 元";
    if (const auto *credit = dynamic_cast<const CreditAccount *>(&account))
        info += L"\n信用额度：" + moneyText(credit->getCreditLimit()) + L" 元";
    MessageBoxW(GetHWnd(), info.c_str(), L"账户余额", MB_OK | MB_ICONINFORMATION);
}

// 存取款和转账共用金额输入；转账额外确认收款账号和金额。
bool performTransaction(StudentBankAction type)
{
    string target;
    if (type == StudentBankAction::Transfer)
    {
        // 先验证收款账号并展示针对性提示，最终转账仍由后端重新校验。
        if (!inputId(L"请输入收款账号", target))
        {
            return false;
        }
        if (target == selectedAccountId)
        {
            setStatusMessage(L"不能向同一账户转账。");
            return false;
        }
        if (!bank->accountExists(target))
        {
            setStatusMessage(L"收款账户不存在。");
            return false;
        }
    }
    int64_t amount = 0;
    if (!inputAmount(L"请输入金额（元，大于 0，最多两位小数）", amount))
    {
        return false;
    }

    if (type == StudentBankAction::Transfer)
    {
        wstring summary = L"账号：" + toWide(selectedAccountId) +
                          L"\n金额：" + moneyText(amount) + L" 元\n收款账号：" + toWide(target);
        if (MessageBoxW(GetHWnd(), summary.c_str(), L"确认转账",
                        MB_OKCANCEL | MB_ICONQUESTION) != IDOK)
            return false;
    }

    bool succeeded = false;
    switch (type)
    {
    case StudentBankAction::Deposit:
        succeeded = bank->deposit(selectedAccountId, amount);
        break;
    case StudentBankAction::Withdraw:
        succeeded = bank->withdraw(selectedAccountId, amount);
        break;
    case StudentBankAction::Transfer:
        succeeded = bank->Transfer(selectedAccountId, target, amount);
        break;
    default:
        return false;
    }
    if (!succeeded)
    {
        setStatusMessage(L"操作失败：无权操作、金额不足或超出额度范围。");
    }
    return succeeded;
}

void finishBankAction(bool changed, bool openingAccount, bool closingAccount)
{
    // 取消、校验失败或业务失败都不保存；只有成功改变账户状态才持久化。
    if (!changed)
        return;

    hasUnsavedChanges = true;
    if (saveBankData())
    {
        if (openingAccount)
            setStatusMessage(L"开户成功，账号：" + toWide(selectedAccountId), false);
        else if (closingAccount)
            setStatusMessage(L"销户成功。", false);
        else
            setStatusMessage(L"业务办理成功，账户已保存。", false);
    }

    refreshVisibleRows();
    if (openingAccount)
    {
        firstVisibleRow =
            (static_cast<int>(visibleAccounts.size()) - 1) / RowsPerPage * RowsPerPage;
    }
}

bool canLeaveBank()
{
    // 退出前补救保存失败的业务结果；失败时留在页面以免静默丢失数据。
    if (!hasUnsavedChanges)
    {
        return true;
    }
    if (saveBankData())
    {
        return true;
    }

    MessageBoxW(GetHWnd(), L"数据尚未保存，请检查目录写入权限后重试。", L"保存失败",
                MB_OK | MB_ICONERROR);
    return false;
}

bool loadBankData()
{
    error_code ec;
    filesystem::create_directories(bank->getDataDirectory(), ec);
    if (ec || !filesystem::is_directory(bank->getDataDirectory(), ec) || ec)
    {
        return false;
    }
    bool hasAccounts = filesystem::exists(bank->getDataDirectory() / "accounts.txt", ec);
    if (ec)
    {
        return false;
    }

    if (!hasAccounts)
    {
        // 数据目录存在但尚未开户时按空系统启动。
        return true;
    }

    return bank->loadAccounts();
}

void applyAccountSearch(const wstring &keyword)
{
    // 搜索会改变可见行，因此清除旧选择，避免误操作被筛掉的账户。
    searchKeyword = keyword;
    selectedAccountId.clear();
    firstVisibleRow = 0;
    refreshVisibleRows();
}

void selectAccountAt(int y, const RECT &tableRect)
{
    // 把鼠标纵坐标换算成当前页对应的列表位置。
    int position = firstVisibleRow + (y - tableRect.top) / 38;
    if (position < static_cast<int>(visibleIndices.size()))
        selectedAccountId = visibleAccounts[visibleIndices[position]]->getAccountId();
}

} // namespace bank_gui

using namespace bank_gui;

void runBankGui(const User& currentUser, const filesystem::path& dataDirectory)
{
    // 银行角色只由用户号前缀决定：B 为管理员，其余已登录用户均为学生。
    const string userId = currentUser.getUserid();
    const bool isAdministrator = !userId.empty() && userId.front() == 'B';
    // 每次进入重新加载数据；窗口和双缓冲仍由主系统管理。
    try
    {
        bank = make_unique<Banksystem>(currentUser, isAdministrator, dataDirectory);
    }
    catch (const invalid_argument&)
    {
        MessageBoxW(GetHWnd(), L"银行身份信息不完整，无法进入。", L"无法进入", MB_OK);
        return;
    }
    // 银行界面可能被反复进入；每次都重置会话状态，避免沿用上次的筛选或选中项。
    visibleAccounts.clear();
    hasUnsavedChanges = false;
    firstVisibleRow = 0;
    selectedAccountId.clear();
    searchKeyword.clear();
    visibleIndices.clear();
    statusMessage.clear();
    statusColor = Muted;
    if (!loadBankData())
    {
        MessageBoxW(GetHWnd(),
                    L"银行数据目录不可用，或账户文件缺失、格式错误。\n原文件未修改。",
                    L"加载失败", MB_OK | MB_ICONERROR);
        return;
    }

    refreshVisibleRows();
    if (bank->isAdministrator())
        runAdminBankFrontend();
    else
        runStudentBankFrontend();
}


