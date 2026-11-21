#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include <stdexcept>

#define NOMINMAX
#include <graphics.h>
#include <windows.h>

#include "Banksystem.h"
#include "CreditAccount.h"
#include "gui_bank.h"

using namespace std;

// 阅读顺序：先看文件末尾的 runBankGui，直接看主循环中的点击分支，再看 handleAction。
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

namespace
{
// 二、界面配置与状态。以下变量仅在本文件使用。
constexpr COLORREF Blue = RGB(0, 63, 136);
constexpr COLORREF Ink = RGB(33, 37, 41);
constexpr COLORREF Muted = RGB(108, 117, 125);
constexpr int RowsPerPage = 8;
constexpr array<int, 5> ColumnX = {80, 320, 550, 710, 950};
constexpr array<int, 5> ColumnWidths = {220, 210, 140, 220, 170};

// 按钮顺序与操作一一对应，业务处理中不再使用 0、1、2 等数字判断。
enum class BankAction
{
    OpenSaving,
    OpenCredit,
    Close,
    Deposit,
    Withdraw,
    Transfer,
    Details,
    History
};
struct ActionButton
{
    BankAction action;
    const wchar_t *label;
};
// 两种身份各有自己的按钮列表，绘制和点击共用同一份列表。
const vector<ActionButton> AdminButtons = {
    {BankAction::OpenSaving, L"储蓄开户"}, {BankAction::OpenCredit, L"信用开户"},
    {BankAction::Close, L"销户"},
    {BankAction::Details, L"账户详情"}, {BankAction::History, L"账户流水"}};
const vector<ActionButton> StudentButtons = {
    {BankAction::Deposit, L"存款"}, {BankAction::Withdraw, L"取款"},
    {BankAction::Transfer, L"转账"}, {BankAction::Details, L"账户详情"},
    {BankAction::History, L"账户流水"}};
vector<ActionButton> activeButtons;

// 每次进入银行，用主系统传入的用户创建业务对象。
unique_ptr<Banksystem> bank;
// 只缓存后端允许当前用户查看的内容，不持有全体用户的可修改账户容器。
vector<const BankAccount*> visibleAccounts;
vector<Transaction> visibleTransactions;
bool showingHistory = false;    // false 显示账户，true 显示流水
bool hasUnsavedChanges = false; // 保存失败时保留结果，下次操作或退出时自动重试
int firstVisibleRow = 0;        // 当前页第一条记录在 visibleIndices 中的位置
string selectedAccountId;
wstring searchKeyword;
wstring statusMessage;
COLORREF statusColor = Muted;
// 保存原容器的下标；搜索和分页只改变显示顺序，不移动实际账户或流水。
vector<size_t> visibleIndices;

// 三、文字转换、输入对话框与保存。
wstring toWide(const string &text)
{
    if (text.empty())
    {
        return L"";
    }

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

    int length = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                                     nullptr, 0, nullptr, nullptr);
    string result(length, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(),
                        length, nullptr, nullptr);
    return result;
}

void setStatusMessage(const wstring &text, bool error = true)
{
    statusMessage = text;
    statusColor = error ? RGB(190, 45, 45) : RGB(30, 125, 80);
}

bool inputText(const wchar_t *prompt, wstring &value)
{
    wchar_t buffer[128] = {};
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

// 复用后端文件格式；失败时保留内存结果；下次操作或退出时自动重试，成功后才能继续交易。
bool saveBankData()
{
    bool accountsOk = bank->saveAccounts();
    bool transactionsOk = false;
    if (accountsOk)
    {
        transactionsOk = bank->saveTransactions();
    }
    hasUnsavedChanges = !(accountsOk && transactionsOk);

    if (hasUnsavedChanges)
    {
        setStatusMessage(L"自动保存失败，暂勿关闭程序；下次操作或退出银行时会重试。");
    }
    else
    {
        setStatusMessage(L"账户和交易流水已保存。", false);
    }

    return !hasUnsavedChanges;
}

// 四、生成显示列表：账户按关键字筛选，流水按所选账号筛选。
void refreshVisibleRows()
{
    visibleAccounts = bank->getAccounts();
    visibleTransactions = bank->getTransactions();
    visibleIndices.clear();
    if (showingHistory)
    {
        // 最近的流水排在前面；序号取原始位置，不虚构后端没有记录的时间。
        for (size_t i = visibleTransactions.size(); i > 0; --i)
        {
            const auto &transaction = visibleTransactions[i - 1];
            if (selectedAccountId.empty() || transaction.fromAccountId == selectedAccountId ||
                transaction.toAccountId == selectedAccountId)
            {
                visibleIndices.push_back(i - 1);
            }
        }
    }
    else
    {
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
    }

    int lastPageStart =
        visibleIndices.empty()
            ? 0
            : static_cast<int>((visibleIndices.size() - 1) / RowsPerPage) * RowsPerPage;
    firstVisibleRow = min(firstVisibleRow, lastPageStart);
}

// 页面切换后统一回到第一页，并重新生成显示列表。
void showPage(bool showHistory)
{
    showingHistory = showHistory;
    firstVisibleRow = 0;
    refreshVisibleRows();
}

void scrollPage(int offset)
{
    firstVisibleRow = max(0, firstVisibleRow + offset);
    refreshVisibleRows();
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

void drawButton(int x, int y, int width, const wchar_t *label, bool enabled = true)
{
    setfillcolor(enabled ? Blue : RGB(164, 177, 194));
    setlinecolor(enabled ? Blue : RGB(164, 177, 194));
    fillroundrect(x, y, x + width, y + 40, 8, 8);
    settextstyle(18, 0, L"微软雅黑");
    drawText(x + (width - textwidth(label)) / 2, y + 9, label, 18, WHITE);
}

// 绘图按“表头、数据行、分页、按钮”分块；标题和页脚直接放在 draw 中。

void drawTableHeader()
{
    setfillcolor(RGB(245, 248, 252));
    solidrectangle(60, 222, 1140, 262);

    const wchar_t *accountHeaders[] = {L"账号", L"关联用户号", L"账户类型", L"余额（元）",
                                       L"信用额度（元）"};
    const wchar_t *historyHeaders[] = {L"流水序号", L"交易类型", L"金额（元）", L"转出账号",
                                       L"转入账号"};

    for (int column = 0; column < 5; ++column)
    {
        const wchar_t *header = showingHistory ? historyHeaders[column] : accountHeaders[column];
        drawText(ColumnX[column], 233, header, 16, Muted);
    }
}

void drawRows()
{
    for (int row = 0; row < RowsPerPage; ++row)
    {
        int y = 262 + row * 38;
        int position = firstVisibleRow + row;
        bool exists = position < static_cast<int>(visibleIndices.size());
        bool active = exists && !showingHistory &&
                      visibleAccounts[visibleIndices[position]]->getAccountId() == selectedAccountId;

        setfillcolor(active ? RGB(220, 235, 255) : WHITE);
        solidrectangle(60, y, 1140, y + 38);
        if (!exists)
        {
            continue;
        }

        size_t index = visibleIndices[position];
        // 在这里直接准备本行五列的数据，不再经过多个行绘制函数。
        array<wstring, 5> values;
        if (showingHistory)
        {
            const auto& transaction = visibleTransactions[index];
            values = {
                to_wstring(index + 1), toWide(transaction.type), moneyText(transaction.amount),
                transaction.fromAccountId.empty() ? L"—" : toWide(transaction.fromAccountId),
                transaction.toAccountId.empty() ? L"—" : toWide(transaction.toAccountId)
            };
        }
        else
        {
            const auto& account = visibleAccounts[index];
            auto credit = dynamic_cast<const CreditAccount*>(account);
            values = {
                toWide(account->getAccountId()), toWide(account->getOwnerUserId()),
                credit ? L"信用账户" : L"储蓄账户", moneyText(account->getBalance()),
                credit ? moneyText(credit->getCreditLimit()) : L"—"
            };
        }
        for (size_t column = 0; column < values.size(); ++column)
        {
            drawCell(ColumnX[column], y + 10, ColumnWidths[column], values[column]);
        }
    }
}

void drawActions()
{
    if (showingHistory)
    {
        drawButton(60, 625, 160, L"返回账户");
        return;
    }
    for (int i = 0; i < static_cast<int>(activeButtons.size()); ++i)
    {
        BankAction type = activeButtons[i].action;
        bool opening = type == BankAction::OpenSaving || type == BankAction::OpenCredit;
        bool enabled = opening || !selectedAccountId.empty();
        drawButton(60 + i * 137, 625, 121, activeButtons[i].label, enabled);
    }
}

void drawBankPage()
{
    cleardevice();
    setbkmode(TRANSPARENT);
    setfillcolor(Blue);
    solidrectangle(0, 0, 1200, 72);
    drawText(40, 22, L"华南理工大学  校园综合管理系统", 26, WHITE);
    drawText(60, 92, showingHistory ? L"< 返回账户" : L"< 退出银行管理", 18, Blue);
    wstring title = L"我的银行";
    if (bank->isAdministrator())
        title = L"银行管理";
    if (showingHistory)
        title = L"交易流水";
    drawText(60, 130, title, 28);
    drawText(60, 180,
             (bank->isAdministrator() ? L"账户总数  " : L"我的账户数  ") + to_wstring(visibleAccounts.size()) + L"    交易笔数  " +
                 to_wstring(visibleTransactions.size()) +
                 (hasUnsavedChanges ? L"    待保存" : L"    数据已同步"),
             18, Blue);
    if (!showingHistory)
    {
        drawButton(680, 125, 140, L"搜索账户");
        drawButton(840, 125, 140, L"显示全部");
        drawButton(1000, 125, 140, bank->isAdministrator() ? L"全部流水" : L"我的流水");
    }
    drawTableHeader();
    drawRows();
    if (visibleIndices.empty())
    {
        wstring emptyMessage = L"暂无本人账户，请联系银行管理员";
        if (bank->isAdministrator())
            emptyMessage = L"暂无匹配账户，可开户或显示全部";
        if (showingHistory)
            emptyMessage = L"暂无交易流水";
        drawText(400, 390, emptyMessage, 20, Muted);
    }
    drawButton(900, 575, 110, L"上一页", firstVisibleRow > 0);
    drawButton(1030, 575, 110, L"下一页",
               firstVisibleRow + RowsPerPage < static_cast<int>(visibleIndices.size()));
    drawActions();
    if (!statusMessage.empty())
        drawText(60, 678, statusMessage, 16, statusColor);
    setlinecolor(RGB(205, 212, 224));
    line(0, 706, 1200, 706);
    drawText(405, 724, L"2026 华南理工大学 · 校园综合管理系统", 16, RGB(120, 128, 140));
    FlushBatchDraw();
}

// 六、业务按钮：收集输入后调用 Banksystem，不在界面里计算余额。
bool openAccount(bool creditAccount)
{
    string ownerUserId;
    if (!inputId(L"请输入关联用户号（账号自动生成）", ownerUserId)) return false;
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
    refreshVisibleRows();
    firstVisibleRow = (static_cast<int>(visibleAccounts.size()) - 1) / RowsPerPage * RowsPerPage;
    return true;
}

bool closeSelectedAccount()
{
    const auto* account = bank->findAccount(selectedAccountId);
    if (!account) return false;
    if (account->getBalance() != 0)
    {
        setStatusMessage(L"余额必须为 0 才能销户，请先取出余额或还清欠款。");
        return false;
    }
    wstring prompt = L"确定注销账户 " + toWide(selectedAccountId) +
                     L" 吗？\n销户后不能交易，历史流水保留。";
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

void showAccountDetails(const BankAccount &account)
{
    wstring info = L"账号：" + toWide(account.getAccountId()) + L"\n用户号：" +
                        toWide(account.getOwnerUserId()) + L"\n余额：" + moneyText(account.getBalance()) +
                        L" 元";

    auto credit = dynamic_cast<const CreditAccount *>(&account);
    if (credit)
    {
        info += L"\n类型：信用账户\n信用额度：" + moneyText(credit->getCreditLimit()) + L" 元";
    }
    else
    {
        info += L"\n类型：储蓄账户";
    }

    MessageBoxW(GetHWnd(), info.c_str(), L"账户详情", MB_OK | MB_ICONINFORMATION);
}

// 存取款和转账共用输入、确认流程，具体业务通过有名字的操作类型区分。
bool performTransaction(BankAction type)
{
    string target;
    if (type == BankAction::Transfer)
    {
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

    const wchar_t *title = L"确认转账";
    if (type == BankAction::Deposit)
    {
        title = L"确认存款";
    }
    if (type == BankAction::Withdraw)
    {
        title = L"确认取款";
    }
    wstring summary =
        L"账号：" + toWide(selectedAccountId) + L"\n金额：" + moneyText(amount) + L" 元";
    if (type == BankAction::Transfer)
    {
        summary += L"\n收款账号：" + toWide(target);
    }
    if (MessageBoxW(GetHWnd(), summary.c_str(), title, MB_OKCANCEL | MB_ICONQUESTION) != IDOK)
    {
        return false;
    }

    bool succeeded = false;
    switch (type)
    {
    case BankAction::Deposit:
        succeeded = bank->deposit(selectedAccountId, amount);
        break;
    case BankAction::Withdraw:
        succeeded = bank->withdraw(selectedAccountId, amount);
        break;
    case BankAction::Transfer:
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

// 一次按钮操作：检查条件 → 执行业务 → 保存 → 更新列表。
void handleAction(BankAction type)
{
    bool openingAccount = type == BankAction::OpenSaving || type == BankAction::OpenCredit;
    bool movingMoney = type == BankAction::Deposit || type == BankAction::Withdraw || type == BankAction::Transfer;
    if (((openingAccount || type == BankAction::Close) && !bank->isAdministrator()) || (movingMoney && bank->isAdministrator()))
    {
        setStatusMessage(L"当前身份无权执行此操作。");
        return;
    }
    if (movingMoney && !bank->canOperateAccount(selectedAccountId))
    {
        setStatusMessage(L"请先选择本人的银行账户。");
        return;
    }
    // 上一笔未保存时先自动重试，失败则不执行新的业务。
    if (hasUnsavedChanges && !saveBankData())
        return;
    const BankAccount *account = bank->findAccount(selectedAccountId);
    if (!openingAccount && !account)
    {
        setStatusMessage(L"请先点击表格选择账户。");
        return;
    }

    bool changed = false;
    switch (type)
    {
    case BankAction::Close:
        changed = closeSelectedAccount();
        break;
    case BankAction::OpenSaving:
        changed = openAccount(false);
        break;
    case BankAction::OpenCredit:
        changed = openAccount(true);
        break;
    case BankAction::Deposit:
    case BankAction::Withdraw:
    case BankAction::Transfer:
        changed = performTransaction(type);
        break;
    case BankAction::Details:
        showAccountDetails(*account);
        return;
    case BankAction::History:
        showPage(true);
        return;
    }
    if (changed)
    {
        hasUnsavedChanges = true;
        if (saveBankData())
        {
            if (openingAccount)
                setStatusMessage(L"开户成功，账号：" + toWide(selectedAccountId), false);
            else if (type == BankAction::Close)
                setStatusMessage(L"销户成功，历史流水已保留。", false);
            else
                setStatusMessage(L"业务办理成功，账户与流水已保存。", false);
        }
        refreshVisibleRows();
    }
}

bool canLeaveBank()
{
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
    if (!filesystem::is_directory(bank->getDataDirectory(), ec) || ec)
    {
        return false;
    }
    bool hasAccounts = filesystem::exists(bank->getDataDirectory() / "accounts.txt", ec);
    if (ec)
    {
        return false;
    }

    bool hasTransactions = filesystem::exists(bank->getDataDirectory() / "transactions.txt", ec);
    if (ec || hasAccounts != hasTransactions)
    {
        return false;
    }

    if (!hasAccounts)
    {
        return true;
    }

    // 流水会检查关联账号，因此必须先加载账户，再加载流水。
    if (!bank->loadAccounts())
    {
        return false;
    }
    return bank->loadTransactions();
}

} // namespace

void runBankGui(const User& currentUser, bool isAdministrator,
                const filesystem::path& dataDirectory)
{
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
    activeButtons = bank->isAdministrator() ? AdminButtons : StudentButtons;
    visibleAccounts.clear();
    visibleTransactions.clear();
    showingHistory = false;
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
                    L"银行数据目录不可用，或账户/"
                    L"流水文件缺失、格式错误。\n请确保两份文件配套，原文件未修改。",
                    L"加载失败", MB_OK | MB_ICONERROR);
        return;
    }

    refreshVisibleRows();
    drawBankPage();
    // 与主界面保持一致：RECT 定义按钮区域，PtInRect 判断鼠标是否落在区域内。
    const RECT backR = {60, 90, 260, 120};
    const RECT historyBackR = {60, 625, 220, 665};
    const RECT previousR = {900, 575, 1010, 615};
    const RECT nextR = {1030, 575, 1140, 615};
    const RECT tableR = {60, 262, 1140, 262 + RowsPerPage * 38};
    const RECT searchR = {680, 125, 820, 165};
    const RECT allAccountsR = {840, 125, 980, 165};
    const RECT allHistoryR = {1000, 125, 1140, 165};

    ExMessage msg;
    HWND window = GetHWnd();
    while (IsWindow(window))
    {
        if (!peekmessage(&msg, EM_MOUSE | EM_KEY))
        {
            drawBankPage();
            Sleep(10);
            continue;
        }

        if (msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE)
        {
            if (showingHistory)
                showPage(false);
            else if (canLeaveBank())
                return;
        }
        else if (msg.message == WM_MOUSEWHEEL)
        {
            scrollPage(msg.wheel > 0 ? -RowsPerPage : RowsPerPage);
        }
        else if (msg.message == WM_LBUTTONDOWN)
        {
            POINT mouse = {msg.x, msg.y};
            if (PtInRect(&backR, mouse) || (showingHistory && PtInRect(&historyBackR, mouse)))
            {
                if (showingHistory)
                    showPage(false);
                else if (canLeaveBank())
                    return;
            }
            else if (PtInRect(&previousR, mouse))
            {
                scrollPage(-RowsPerPage);
            }
            else if (PtInRect(&nextR, mouse))
            {
                scrollPage(RowsPerPage);
            }
            else if (!showingHistory)
            {
                if (PtInRect(&tableR, mouse))
                {
                    int position = firstVisibleRow + (mouse.y - tableR.top) / 38;
                    if (position < static_cast<int>(visibleIndices.size()))
                        selectedAccountId = visibleAccounts[visibleIndices[position]]->getAccountId();
                }
                else if (PtInRect(&searchR, mouse))
                {
                    wstring input;
                    if (inputText(L"请输入账号或用户号关键字（留空显示全部）", input))
                    {
                        searchKeyword = input;
                        selectedAccountId.clear();
                        showPage(false);
                    }
                }
                else if (PtInRect(&allAccountsR, mouse))
                {
                    searchKeyword.clear();
                    selectedAccountId.clear();
                    showPage(false);
                }
                else if (PtInRect(&allHistoryR, mouse))
                {
                    selectedAccountId.clear();
                    showPage(true);
                }
                else
                {
                    // 按当前角色的按钮列表处理点击，位置与绘制保持一致。
                    for (int i = 0; i < static_cast<int>(activeButtons.size()); ++i)
                    {
                        int left = 60 + i * 137;
                        RECT actionR = {left, 625, left + 121, 665};
                        if (PtInRect(&actionR, mouse))
                        {
                            handleAction(activeButtons[i].action);
                            break;
                        }
                    }
                }
            }
        }
        drawBankPage();
    }
}


