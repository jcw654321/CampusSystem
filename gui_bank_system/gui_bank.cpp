#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

#define NOMINMAX
#include <graphics.h>
#include <windows.h>

#include "Banksystem.h"
#include "CreditAccount.h"
#include "gui_bank.h"

// 阅读顺序：先看文件末尾的 runBankGui，再看 handleMouseDown 和 handleAction。
// GUI 只负责输入、显示和调用后端；账户规则在 bank_manage_system 中。

// 一、金额转换：界面输入“元”，业务层和文件一律使用整数“分”。
// 从十进制文本直接解析“分”，避免浮点数舍入；不接受符号、指数或空白。
inline bool parseMoney(const std::wstring &text, std::int64_t &cents)
{
    if (text.empty())
    {
        return false;
    }

    std::int64_t value = 0;
    bool hasDecimalPoint = false;
    int decimalDigits = 0;
    int integerDigits = 0;
    const auto maxCents = (std::numeric_limits<std::int64_t>::max)();

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

inline std::wstring moneyText(std::int64_t cents)
{
    // 信用账户可能是负余额。
    // 同样支持 INT64_MIN，不能直接对有符号数取负。
    auto magnitude = cents < 0 ? std::uint64_t(-(cents + 1)) + 1 : std::uint64_t(cents);
    auto fraction = magnitude % 100;
    return (cents < 0 ? L"-" : L"") + std::to_wstring(magnitude / 100) + L"." +
           (fraction < 10 ? L"0" : L"") + std::to_wstring(fraction);
}

namespace
{
// 二、界面配置与状态。以下变量仅在本文件使用。
constexpr COLORREF Blue = RGB(0, 63, 136);
constexpr COLORREF Ink = RGB(33, 37, 41);
constexpr COLORREF Muted = RGB(108, 117, 125);
constexpr int RowsPerPage = 8;
constexpr std::array<int, 5> ColumnX = {80, 320, 550, 710, 950};
constexpr std::array<int, 5> ColumnWidths = {220, 210, 140, 220, 170};

// 按钮顺序与操作一一对应，业务处理中不再使用 0、1、2 等数字判断。
enum class BankAction
{
    OpenSaving,
    OpenCredit,
    Deposit,
    Withdraw,
    Transfer,
    Details,
    History,
    Save
};
struct ActionButton
{
    BankAction action;
    const wchar_t *label;
};
constexpr ActionButton ActionButtons[] = {
    {BankAction::OpenSaving, L"储蓄开户"}, {BankAction::OpenCredit, L"信用开户"},
    {BankAction::Deposit, L"存款"},        {BankAction::Withdraw, L"取款"},
    {BankAction::Transfer, L"转账"},       {BankAction::Details, L"账户详情"},
    {BankAction::History, L"账户流水"},    {BankAction::Save, L"保存"}};

Banksystem bank;
bool showingHistory = false;    // false 显示账户，true 显示流水
bool hasUnsavedChanges = false; // 保存失败时阻止继续交易，允许重试
int firstVisibleRow = 0;        // 当前页第一条记录在 visibleIndices 中的位置
std::string selectedAccountId;
std::wstring searchKeyword;
std::wstring statusMessage = L"请选择账户办理业务；金额单位为元。";
COLORREF statusColor = Muted;
// 保存原容器的下标；搜索和分页只改变显示顺序，不移动实际账户或流水。
std::vector<size_t> visibleIndices;

// 三、文字转换、输入对话框与保存。
std::wstring toWide(const std::string &text)
{
    if (text.empty())
    {
        return L"";
    }

    int length =
        MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    std::wstring result(length, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(),
                        length);
    return result;
}

std::string toUtf8(const std::wstring &text)
{
    if (text.empty())
    {
        return "";
    }

    int length = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                                     nullptr, 0, nullptr, nullptr);
    std::string result(length, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(),
                        length, nullptr, nullptr);
    return result;
}

void setStatusMessage(const std::wstring &text, bool error = true)
{
    statusMessage = text;
    statusColor = error ? RGB(190, 45, 45) : RGB(30, 125, 80);
}

bool inputText(const wchar_t *prompt, std::wstring &value)
{
    wchar_t buffer[128] = {};
    if (!InputBox(buffer, 128, prompt, L"银行管理"))
    {
        return false;
    }

    value = buffer;
    return true;
}

bool inputId(const wchar_t *prompt, std::string &value)
{
    std::wstring input;
    if (!inputText(prompt, input))
    {
        return false;
    }

    if (input.empty() || input.size() > 32 ||
        input.find_first_not_of(
            L"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-") !=
            std::wstring::npos)
    {
        setStatusMessage(L"编号须为 1–32 位字母、数字、下划线或短横线。");
        return false;
    }

    value = toUtf8(input);
    return true;
}

bool inputAmount(const wchar_t *prompt, std::int64_t &amount)
{
    std::wstring input;
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

// 复用后端文件格式；失败时保留内存结果并阻止继续交易，允许重试保存。
bool saveBankData()
{
    bool accountsOk = bank.saveAccounts();
    bool transactionsOk = false;
    if (accountsOk)
    {
        transactionsOk = bank.saveTransactions();
    }
    hasUnsavedChanges = !(accountsOk && transactionsOk);

    if (hasUnsavedChanges)
    {
        setStatusMessage(L"保存失败：结果仍在内存中，请点击“保存”重试，暂勿关闭程序。");
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
    visibleIndices.clear();
    if (showingHistory)
    {
        // 最近的流水排在前面；序号取原始位置，不虚构后端没有记录的时间。
        for (size_t i = bank.transactions.size(); i > 0; --i)
        {
            const auto &transaction = bank.transactions[i - 1];
            if (selectedAccountId.empty() || transaction.fromAccountId == selectedAccountId ||
                transaction.toAccountId == selectedAccountId)
            {
                visibleIndices.push_back(i - 1);
            }
        }
    }
    else
    {
        for (size_t i = 0; i < bank.accounts.size(); ++i)
        {
            const auto &account = bank.accounts[i];
            bool matchesKeyword =
                searchKeyword.empty() ||
                toWide(account->accountId).find(searchKeyword) != std::wstring::npos ||
                toWide(account->studentId).find(searchKeyword) != std::wstring::npos;
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
    firstVisibleRow = std::min(firstVisibleRow, lastPageStart);
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
    firstVisibleRow = std::max(0, firstVisibleRow + offset);
    refreshVisibleRows();
}

// 五、绘图：基础控件 → 表格和按钮 → 完整页面。
void drawText(int x, int y, const std::wstring &s, int size = 18, COLORREF color = Ink)
{
    settextstyle(size, 0, L"微软雅黑");
    settextcolor(color);
    outtextxy(x, y, s.c_str());
}

void drawCell(int x, int y, int width, std::wstring value)
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

bool isInside(int x, int y, int left, int top, int width, int height = 40)
{
    return x >= left && x < left + width && y >= top && y < top + height;
}

// 绘图按“表头、数据行、分页、按钮”分块；标题和页脚直接放在 draw 中。

void drawTableHeader()
{
    setfillcolor(RGB(245, 248, 252));
    solidrectangle(60, 222, 1140, 262);

    const wchar_t *accountHeaders[] = {L"账号", L"关联学号", L"账户类型", L"余额（元）",
                                       L"信用额度（元）"};
    const wchar_t *historyHeaders[] = {L"流水序号", L"交易类型", L"金额（元）", L"转出账号",
                                       L"转入账号"};

    for (int column = 0; column < 5; ++column)
    {
        const wchar_t *header = showingHistory ? historyHeaders[column] : accountHeaders[column];
        drawText(ColumnX[column], 233, header, 16, Muted);
    }
}

void drawTableRow(const std::array<std::wstring, 5> &values, int y)
{
    for (size_t column = 0; column < values.size(); ++column)
    {
        drawCell(ColumnX[column], y + 10, ColumnWidths[column], values[column]);
    }
}

std::wstring displayAccountId(const std::string &accountId)
{
    return accountId.empty() ? L"—" : toWide(accountId);
}

void drawHistoryRow(size_t index, int y)
{
    const auto &transaction = bank.transactions[index];
    std::array<std::wstring, 5> values = {
        std::to_wstring(index + 1), toWide(transaction.type), moneyText(transaction.amount),
        displayAccountId(transaction.fromAccountId), displayAccountId(transaction.toAccountId)};
    drawTableRow(values, y);
}

void drawAccountRow(size_t index, int y)
{
    const auto &account = bank.accounts[index];
    // 只有信用账户才有信用额度；转换失败说明这是储蓄账户。
    auto credit = dynamic_cast<CreditAccount *>(account.get());

    std::array<std::wstring, 5> values = {
        toWide(account->accountId), toWide(account->studentId), credit ? L"信用账户" : L"储蓄账户",
        moneyText(account->balance), credit ? moneyText(credit->getCreditLimit()) : L"—"};
    drawTableRow(values, y);
}

void drawRows()
{
    for (int row = 0; row < RowsPerPage; ++row)
    {
        int y = 262 + row * 38;
        int position = firstVisibleRow + row;
        bool exists = position < static_cast<int>(visibleIndices.size());
        bool active = exists && !showingHistory &&
                      bank.accounts[visibleIndices[position]]->accountId == selectedAccountId;

        setfillcolor(active ? RGB(220, 235, 255) : WHITE);
        solidrectangle(60, y, 1140, y + 38);
        if (!exists)
        {
            continue;
        }

        size_t index = visibleIndices[position];
        if (showingHistory)
        {
            drawHistoryRow(index, y);
        }
        else
        {
            drawAccountRow(index, y);
        }
    }
}

void drawPagination()
{
    int pageCount =
        std::max(1, (static_cast<int>(visibleIndices.size()) + RowsPerPage - 1) / RowsPerPage);
    std::wstring pageText = L"共 " + std::to_wstring(visibleIndices.size()) + L" 条    第 " +
                            std::to_wstring(firstVisibleRow / RowsPerPage + 1) + L" / " +
                            std::to_wstring(pageCount) + L" 页";
    drawText(60, 582, pageText, 16, Muted);

    drawButton(900, 575, 110, L"上一页", firstVisibleRow > 0);
    drawButton(1030, 575, 110, L"下一页",
               firstVisibleRow + RowsPerPage < static_cast<int>(visibleIndices.size()));
}

void drawActions()
{
    if (showingHistory)
    {
        drawButton(60, 625, 160, L"返回账户");
        return;
    }
    for (int i = 0; i < 8; ++i)
    {
        BankAction type = ActionButtons[i].action;
        bool opening = type == BankAction::OpenSaving || type == BankAction::OpenCredit;
        bool enabled = type == BankAction::Save ||
                       (!hasUnsavedChanges && (opening || !selectedAccountId.empty()));
        drawButton(60 + i * 137, 625, 121, ActionButtons[i].label, enabled);
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
    drawText(60, 130, showingHistory ? L"交易流水" : L"银行管理", 28);
    std::wstring subtitle = L"储蓄账户 / 信用账户";
    if (showingHistory)
    {
        subtitle = selectedAccountId.empty() ? L"全部账户" : L"账号：" + toWide(selectedAccountId);
    }
    drawText(265, 141, subtitle, 17, Muted);
    drawText(60, 180,
             L"账户总数  " + std::to_wstring(bank.accounts.size()) + L"    交易笔数  " +
                 std::to_wstring(bank.transactions.size()) +
                 (hasUnsavedChanges ? L"    待保存" : L"    数据已同步"),
             18, Blue);
    if (!showingHistory)
    {
        drawButton(680, 125, 140, L"搜索账户");
        drawButton(840, 125, 140, L"显示全部");
        drawButton(1000, 125, 140, L"全部流水");
    }
    drawTableHeader();
    drawRows();
    if (visibleIndices.empty())
    {
        drawText(400, 390, showingHistory ? L"暂无交易流水" : L"暂无匹配账户，可开户或显示全部", 20,
                 Muted);
    }
    drawPagination();
    drawActions();
    drawText(60, 678, statusMessage, 16, statusColor);
    setlinecolor(RGB(205, 212, 224));
    line(0, 706, 1200, 706);
    drawText(405, 724, L"2026 华南理工大学 · 校园综合管理系统", 16, RGB(120, 128, 140));
    FlushBatchDraw();
}

// 六、业务按钮：收集输入后调用 Banksystem，不在界面里计算余额。
bool openAccount(bool creditAccount)
{
    std::string accountId;
    std::string studentId;

    if (!inputId(L"请输入新账号（1–32 位字母、数字、下划线或短横线）", accountId))
    {
        return false;
    }

    if (bank.accountExists(accountId))
    {
        setStatusMessage(L"账号已存在，请使用其他账号。");
        return false;
    }

    if (!inputId(L"请输入关联学号", studentId))
    {
        return false;
    }

    std::int64_t creditLimit = 0;
    if (creditAccount && !inputAmount(L"请输入信用额度（元，最多两位小数）", creditLimit))
    {
        return false;
    }

    bool opened = false;
    if (creditAccount)
    {
        opened = bank.openCreditAccount(accountId, studentId, creditLimit);
    }
    else
    {
        opened = bank.openSavingAccount(accountId, studentId);
    }
    if (!opened)
    {
        setStatusMessage(L"开户失败，请检查账号和信用额度。");
        return false;
    }

    selectedAccountId = accountId;
    searchKeyword.clear();
    firstVisibleRow = (static_cast<int>(bank.accounts.size()) - 1) / RowsPerPage * RowsPerPage;
    return true;
}

void showAccountDetails(BankAccount &account)
{
    std::wstring info = L"账号：" + toWide(account.accountId) + L"\n学号：" +
                        toWide(account.studentId) + L"\n余额：" + moneyText(account.balance) +
                        L" 元";

    auto credit = dynamic_cast<CreditAccount *>(&account);
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
    std::string target;
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
        if (!bank.accountExists(target))
        {
            setStatusMessage(L"收款账户不存在。");
            return false;
        }
    }
    std::int64_t amount = 0;
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
    std::wstring summary =
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
        succeeded = bank.deposit(selectedAccountId, amount);
        break;
    case BankAction::Withdraw:
        succeeded = bank.withdraw(selectedAccountId, amount);
        break;
    case BankAction::Transfer:
        succeeded = bank.Transfer(selectedAccountId, target, amount);
        break;
    default:
        return false;
    }
    if (!succeeded)
    {
        setStatusMessage(L"操作失败：可用余额不足、超出信用额度或收款余额超限。");
    }
    return succeeded;
}

// 一次按钮操作：检查条件 → 执行业务 → 保存 → 更新列表。
void handleAction(BankAction type)
{
    if (type == BankAction::Save)
    {
        saveBankData();
        return;
    }
    if (hasUnsavedChanges)
    {
        setStatusMessage(L"请先重试保存，再办理下一笔业务。");
        return;
    }
    bool opening = type == BankAction::OpenSaving || type == BankAction::OpenCredit;
    BankAccount *account = bank.findAccount(selectedAccountId);
    if (!opening && !account)
    {
        setStatusMessage(L"请先点击表格选择账户。");
        return;
    }

    bool changed = false;
    switch (type)
    {
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
    case BankAction::Save:
        return; // 已在上面处理
    }
    if (changed)
    {
        hasUnsavedChanges = true;
        if (saveBankData())
        {
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
    std::error_code ec;
    bool hasAccounts = std::filesystem::exists("accounts.txt", ec);
    if (ec)
    {
        return false;
    }

    bool hasTransactions = std::filesystem::exists("transactions.txt", ec);
    if (ec || hasAccounts != hasTransactions)
    {
        return false;
    }

    if (!hasAccounts)
    {
        return true;
    }

    // 流水会检查关联账号，因此必须先加载账户，再加载流水。
    if (!bank.loadAccounts())
    {
        return false;
    }
    return bank.loadTransactions();
}

// 七、鼠标操作与主入口。
// 鼠标直接对应页面控件，不再经过多层事件转发。
void handleMouseDown(int x, int y)
{
    if (isInside(x, y, 900, 575, 110))
    {
        scrollPage(-RowsPerPage);
    }
    else if (isInside(x, y, 1030, 575, 110))
    {
        scrollPage(RowsPerPage);
    }
    else if (!showingHistory)
    {
        if (isInside(x, y, 60, 262, 1080, RowsPerPage * 38))
        {
            int position = firstVisibleRow + (y - 262) / 38;
            if (position < static_cast<int>(visibleIndices.size()))
            {
                selectedAccountId = bank.accounts[visibleIndices[position]]->accountId;
            }
        }
        else if (isInside(x, y, 680, 125, 140))
        {
            std::wstring input;
            if (inputText(L"请输入账号或学号关键字（留空显示全部）", input))
            {
                searchKeyword = input;
                selectedAccountId.clear();
                showPage(false);
            }
        }
        else if (isInside(x, y, 840, 125, 140))
        {
            searchKeyword.clear();
            selectedAccountId.clear();
            showPage(false);
        }
        else if (isInside(x, y, 1000, 125, 140))
        {
            selectedAccountId.clear();
            showPage(true);
        }
        else
        {
            for (int i = 0; i < 8; ++i)
            {
                if (isInside(x, y, 60 + i * 137, 625, 121))
                {
                    handleAction(ActionButtons[i].action);
                    break;
                }
            }
        }
    }
}

} // namespace

void runBankGui()
{
    // 每次进入重新加载数据；窗口和双缓冲仍由主系统管理。
    bank = Banksystem{};
    showingHistory = false;
    hasUnsavedChanges = false;
    firstVisibleRow = 0;
    selectedAccountId.clear();
    searchKeyword.clear();
    visibleIndices.clear();
    statusMessage = L"请选择账户办理业务；金额单位为元。";
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
        // Esc 与返回按钮执行相同操作：流水页返回账户页，账户页退出银行。
        bool pressedEscape = msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE;
        bool clickedTopBack = false;
        bool clickedHistoryBack = false;
        if (msg.message == WM_LBUTTONDOWN)
        {
            clickedTopBack = isInside(msg.x, msg.y, 60, 90, 200, 30);
            clickedHistoryBack = showingHistory && isInside(msg.x, msg.y, 60, 625, 160);
        }
        if (pressedEscape || clickedTopBack || clickedHistoryBack)
        {
            if (showingHistory)
            {
                showPage(false);
            }
            else if (canLeaveBank())
            {
                return;
            }
        }
        else if (msg.message == WM_MOUSEWHEEL)
        {
            scrollPage(msg.wheel > 0 ? -RowsPerPage : RowsPerPage);
        }
        else if (msg.message == WM_LBUTTONDOWN)
        {
            handleMouseDown(msg.x, msg.y);
        }
        drawBankPage();
    }
}
