#include <algorithm>
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

// 从十进制文本直接解析“分”，避免浮点数舍入；不接受符号、指数或空白。
inline bool parseMoney(const std::wstring& text, std::int64_t& cents)
{
    if (text.empty())
    {
        return false;
    }

    std::int64_t value = 0;
    bool dot = false;
    int decimals = 0;
    int integers = 0;
    const auto max = (std::numeric_limits<std::int64_t>::max)();

    for (wchar_t c : text)
    {
        if (c == L'.' && !dot && integers > 0)
        {
            dot = true;
            continue;
        }

        if (c < L'0' || c > L'9')
        {
            return false;
        }

        if (dot && ++decimals > 2)
        {
            return false;
        }

        if (!dot)
        {
            ++integers;
        }

        int digit = c - L'0';
        if (value > (max - digit) / 10)
        {
            return false;
        }

        value = value * 10 + digit;
    }

    if (dot && decimals == 0)
    {
        return false;
    }

    for (int i = decimals; i < 2; ++i)
    {
        if (value > max / 10)
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
    // 同样支持 INT64_MIN，不能直接对有符号数取负。
    auto magnitude = cents < 0
        ? std::uint64_t(-(cents + 1)) + 1
        : std::uint64_t(cents);
    auto fraction = magnitude % 100;
    return (cents < 0 ? L"-" : L"")
        + std::to_wstring(magnitude / 100)
        + L"."
        + (fraction < 10 ? L"0" : L"")
        + std::to_wstring(fraction);
}

namespace {
constexpr COLORREF Blue = RGB(0, 63, 136);
constexpr COLORREF Ink = RGB(33, 37, 41);
constexpr COLORREF Muted = RGB(108, 117, 125);
constexpr int Rows = 8;

Banksystem bank;
bool history = false;
bool dirty = false;
int first = 0;
std::string selected;
std::wstring keyword;
std::wstring tip = L"请选择账户办理业务；金额单位为元。";
COLORREF tipColor = Muted;
std::vector<size_t> shown;

std::wstring wide(const std::string& text)
{
    if (text.empty())
    {
        return L"";
    }

    int length = MultiByteToWideChar(
        CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    std::wstring result(length, L'\0');
    MultiByteToWideChar(
        CP_UTF8,
        0,
        text.data(),
        static_cast<int>(text.size()),
        result.data(),
        length);
    return result;
}

std::string utf8(const std::wstring& text)
{
    if (text.empty())
    {
        return "";
    }

    int length = WideCharToMultiByte(
        CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
        nullptr, 0, nullptr, nullptr);
    std::string result(length, '\0');
    WideCharToMultiByte(
        CP_UTF8,
        0,
        text.data(),
        static_cast<int>(text.size()),
        result.data(),
        length,
        nullptr,
        nullptr);
    return result;
}

void message(const std::wstring& text, bool error = true)
{
    tip = text;
    tipColor = error ? RGB(190, 45, 45) : RGB(30, 125, 80);
}

bool ask(const wchar_t* prompt, std::wstring& value)
{
    wchar_t buffer[128] = {};
    if (!InputBox(buffer, 128, prompt, L"银行管理"))
    {
        return false;
    }

    value = buffer;
    return true;
}

bool askId(const wchar_t* prompt, std::string& value)
{
    std::wstring input;
    if (!ask(prompt, input))
    {
        return false;
    }

    if (input.empty()
        || input.size() > 32
        || input.find_first_not_of(
            L"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-")
            != std::wstring::npos)
    {
        message(L"编号须为 1–32 位字母、数字、下划线或短横线。");
        return false;
    }

    value = utf8(input);
    return true;
}

bool askAmount(const wchar_t* prompt, std::int64_t& amount)
{
    std::wstring input;
    if (!ask(prompt, input))
    {
        return false;
    }

    if (!parseMoney(input, amount))
    {
        message(L"请输入大于 0 的金额，最多两位小数，且不得超出范围。");
        return false;
    }

    return true;
}

// 复用后端文件格式；失败时保留内存结果并阻止继续交易，允许重试保存。
bool save()
{
    bool accountsOk = bank.saveAccounts();
    bool transactionsOk = accountsOk && bank.saveTransactions();
    dirty = !(accountsOk && transactionsOk);

    if (dirty)
    {
        message(L"保存失败：结果仍在内存中，请点击“保存”重试，暂勿关闭程序。");
    }
    else
    {
        message(L"账户和交易流水已保存。", false);
    }

    return !dirty;
}

void refresh()
{
    shown.clear();
    if (history)
    {
        // 最近的流水排在前面；序号取原始位置，不虚构后端没有记录的时间。
        for (size_t i = bank.transactions.size(); i > 0; --i)
        {
            const auto& t = bank.transactions[i - 1];
            if (selected.empty()
                || t.fromAccountId == selected
                || t.toAccountId == selected)
            {
                shown.push_back(i - 1);
            }
        }
    }
    else
    {
        for (size_t i = 0; i < bank.accounts.size(); ++i)
        {
            const auto& a = bank.accounts[i];
            bool matchesKeyword = keyword.empty()
                || wide(a->accountId).find(keyword) != std::wstring::npos
                || wide(a->studentId).find(keyword) != std::wstring::npos;
            if (matchesKeyword)
            {
                shown.push_back(i);
            }
        }
    }

    int lastPageStart = shown.empty()
        ? 0
        : static_cast<int>((shown.size() - 1) / Rows) * Rows;
    first = std::min(first, lastPageStart);
}
void text(int x, int y, const std::wstring& s, int size = 18, COLORREF color = Ink)
{
    settextstyle(size, 0, L"微软雅黑");
    settextcolor(color);
    outtextxy(x, y, s.c_str());
}

void cell(int x, int y, int width, std::wstring value)
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

    text(x, y, value, 16);
}

void button(int x, int y, int width, const wchar_t* label, bool enabled = true)
{
    setfillcolor(enabled ? Blue : RGB(164, 177, 194));
    setlinecolor(enabled ? Blue : RGB(164, 177, 194));
    fillroundrect(x, y, x + width, y + 40, 8, 8);
    settextstyle(18, 0, L"微软雅黑");
    text(x + (width - textwidth(label)) / 2, y + 9, label, 18, WHITE);
}

bool hit(int x, int y, int left, int top, int width, int height = 40)
{
    return x >= left
        && x < left + width
        && y >= top
        && y < top + height;
}

void drawHeader()
{
    setfillcolor(Blue);
    solidrectangle(0, 0, 1200, 72);
    text(40, 22, L"华南理工大学  校园综合管理系统", 26, WHITE);
}

void drawTitle()
{
    text(60, 92, history ? L"< 返回账户" : L"< 退出银行管理", 18, Blue);
    text(60, 130, history ? L"交易流水" : L"银行管理", 28);

    std::wstring subtitle = history
        ? (selected.empty() ? L"全部账户" : L"账号：" + wide(selected))
        : L"储蓄账户 / 信用账户";
    text(265, 141, subtitle, 17, Muted);
}

void drawSummary()
{
    std::wstring status = dirty ? L"    待保存" : L"    数据已同步";
    std::wstring summary = L"账户总数  "
        + std::to_wstring(bank.accounts.size())
        + L"    交易笔数  "
        + std::to_wstring(bank.transactions.size())
        + status;
    text(60, 180, summary, 18, Blue);
}

void drawToolbar()
{
    if (history)
    {
        return;
    }

    button(680, 125, 140, L"搜索账户");
    button(840, 125, 140, L"显示全部");
    button(1000, 125, 140, L"全部流水");
}

void drawTableHeader()
{
    setfillcolor(RGB(245, 248, 252));
    solidrectangle(60, 222, 1140, 262);

    const int columns[] = {80, 320, 550, 710, 950};
    const wchar_t* accountHeaders[] = {
        L"账号", L"关联学号", L"账户类型", L"余额（元）", L"信用额度（元）"
    };
    const wchar_t* historyHeaders[] = {
        L"流水序号", L"交易类型", L"金额（元）", L"转出账号", L"转入账号"
    };

    for (int column = 0; column < 5; ++column)
    {
        const wchar_t* header = history
            ? historyHeaders[column]
            : accountHeaders[column];
        text(columns[column], 233, header, 16, Muted);
    }
}

void drawHistoryRow(size_t index, int y)
{
    const auto& transaction = bank.transactions[index];
    cell(80, y + 10, 220, std::to_wstring(index + 1));
    cell(320, y + 10, 210, wide(transaction.type));
    cell(550, y + 10, 140, moneyText(transaction.amount));
    cell(
        710,
        y + 10,
        220,
        transaction.fromAccountId.empty()
            ? L"—"
            : wide(transaction.fromAccountId));
    cell(
        950,
        y + 10,
        170,
        transaction.toAccountId.empty()
            ? L"—"
            : wide(transaction.toAccountId));
}

void drawAccountRow(size_t index, int y)
{
    const auto& account = bank.accounts[index];
    auto credit = dynamic_cast<CreditAccount*>(account.get());

    cell(80, y + 10, 220, wide(account->accountId));
    cell(320, y + 10, 210, wide(account->studentId));
    cell(550, y + 10, 140, credit ? L"信用账户" : L"储蓄账户");
    cell(710, y + 10, 220, moneyText(account->balance));
    cell(
        950,
        y + 10,
        170,
        credit ? moneyText(credit->getCreditLimit()) : L"—");
}

void drawRows()
{
    for (int row = 0; row < Rows; ++row)
    {
        int y = 262 + row * 38;
        int position = first + row;
        bool exists = position < static_cast<int>(shown.size());
        bool active = exists
            && !history
            && bank.accounts[shown[position]]->accountId == selected;

        setfillcolor(active ? RGB(220, 235, 255) : WHITE);
        solidrectangle(60, y, 1140, y + 38);
        if (!exists)
        {
            continue;
        }

        size_t index = shown[position];
        if (history)
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
    int pageCount = std::max(
        1, (static_cast<int>(shown.size()) + Rows - 1) / Rows);
    std::wstring pageText = L"共 "
        + std::to_wstring(shown.size())
        + L" 条    第 "
        + std::to_wstring(first / Rows + 1)
        + L" / "
        + std::to_wstring(pageCount)
        + L" 页";
    text(60, 582, pageText, 16, Muted);

    button(900, 575, 110, L"上一页", first > 0);
    button(1030, 575, 110, L"下一页", first + Rows < static_cast<int>(shown.size()));
}

void drawActions()
{
    if (history)
    {
        button(60, 625, 160, L"返回账户");
        return;
    }

    const wchar_t* labels[] = {
        L"储蓄开户", L"信用开户", L"存款", L"取款",
        L"转账", L"账户详情", L"账户流水", L"保存"
    };

    for (int i = 0; i < 8; ++i)
    {
        bool enabled = i == 7 || (!dirty && (i < 2 || !selected.empty()));
        button(60 + i * 137, 625, 121, labels[i], enabled);
    }
}

void drawFooter()
{
    text(60, 678, tip, 16, tipColor);
    setlinecolor(RGB(205, 212, 224));
    line(0, 706, 1200, 706);
    text(405, 724, L"2026 华南理工大学 · 校园综合管理系统", 16, RGB(120, 128, 140));
}

void draw()
{
    cleardevice();
    setbkmode(TRANSPARENT);

    drawHeader();
    drawTitle();
    drawSummary();
    drawToolbar();
    drawTableHeader();
    drawRows();

    if (shown.empty())
    {
        std::wstring emptyMessage = history
            ? L"暂无交易流水"
            : L"暂无匹配账户，可开户或显示全部";
        text(400, 390, emptyMessage, 20, Muted);
    }

    drawPagination();
    drawActions();
    drawFooter();
    FlushBatchDraw();
}

bool openAccount(bool creditAccount)
{
    std::string accountId;
    std::string studentId;

    if (!askId(L"请输入新账号（1–32 位字母、数字、下划线或短横线）", accountId))
    {
        return false;
    }

    if (bank.accountExists(accountId))
    {
        message(L"账号已存在，请使用其他账号。");
        return false;
    }

    if (!askId(L"请输入关联学号", studentId))
    {
        return false;
    }

    std::int64_t creditLimit = 0;
    if (creditAccount
        && !askAmount(L"请输入信用额度（元，最多两位小数）", creditLimit))
    {
        return false;
    }

    bool opened = creditAccount
        ? bank.openCreditAccount(accountId, studentId, creditLimit)
        : bank.openSavingAccount(accountId, studentId);
    if (!opened)
    {
        message(L"开户失败，请检查账号和信用额度。");
        return false;
    }

    selected = accountId;
    keyword.clear();
    first = (static_cast<int>(bank.accounts.size()) - 1) / Rows * Rows;
    return true;
}

void showAccountDetails(BankAccount& account)
{
    std::wstring info = L"账号："
        + wide(account.accountId)
        + L"\n学号："
        + wide(account.studentId)
        + L"\n余额："
        + moneyText(account.balance)
        + L" 元";

    auto credit = dynamic_cast<CreditAccount*>(&account);
    if (credit)
    {
        info += L"\n类型：信用账户\n信用额度："
            + moneyText(credit->getCreditLimit())
            + L" 元";
    }
    else
    {
        info += L"\n类型：储蓄账户";
    }

    MessageBoxW(
        GetHWnd(),
        info.c_str(),
        L"账户详情",
        MB_OK | MB_ICONINFORMATION);
}

bool confirmTransaction(
    int actionType,
    const std::string& target,
    std::int64_t amount)
{
    std::wstring summary = L"账号："
        + wide(selected)
        + L"\n金额："
        + moneyText(amount)
        + L" 元";

    if (actionType == 4)
    {
        summary += L"\n收款账号：" + wide(target);
    }

    const wchar_t* title = actionType == 2
        ? L"确认存款"
        : actionType == 3 ? L"确认取款" : L"确认转账";
    int result = MessageBoxW(
        GetHWnd(),
        summary.c_str(),
        title,
        MB_OKCANCEL | MB_ICONQUESTION);
    return result == IDOK;
}

bool performTransaction(int actionType)
{
    std::string target;
    if (actionType == 4)
    {
        if (!askId(L"请输入收款账号", target))
        {
            return false;
        }

        if (target == selected)
        {
            message(L"不能向同一账户转账。");
            return false;
        }

        if (!bank.accountExists(target))
        {
            message(L"收款账户不存在。");
            return false;
        }
    }

    std::int64_t amount = 0;
    if (!askAmount(L"请输入金额（元，大于 0，最多两位小数）", amount))
    {
        return false;
    }

    if (!confirmTransaction(actionType, target, amount))
    {
        return false;
    }

    bool succeeded = actionType == 2
        ? bank.deposit(selected, amount)
        : actionType == 3
            ? bank.withdraw(selected, amount)
            : bank.Transfer(selected, target, amount);
    if (!succeeded)
    {
        message(L"操作失败：可用余额不足、超出信用额度或收款余额超限。");
        return false;
    }

    return true;
}

bool handleSelectedAccount(int actionType)
{
    BankAccount* account = bank.findAccount(selected);
    if (!account)
    {
        message(L"请先点击表格选择账户。");
        return false;
    }

    if (actionType == 6)
    {
        history = true;
        first = 0;
        refresh();
        return false;
    }

    if (actionType == 5)
    {
        showAccountDetails(*account);
        return false;
    }

    return performTransaction(actionType);
}

void action(int actionType)
{
    if (actionType == 7)
    {
        save();
        return;
    }

    if (dirty)
    {
        message(L"请先重试保存，再办理下一笔业务。");
        return;
    }

    bool changed = actionType < 2
        ? openAccount(actionType == 1)
        : handleSelectedAccount(actionType);
    if (!changed)
    {
        return;
    }

    dirty = true;
    if (save())
    {
        message(L"业务办理成功，账户与流水已保存。", false);
    }
    refresh();
}

bool canExit()
{
    if (!dirty || save())
    {
        return true;
    }

    MessageBoxW(
        GetHWnd(),
        L"数据尚未保存，请检查目录写入权限后重试。",
        L"保存失败",
        MB_OK | MB_ICONERROR);
    return false;
}

bool load()
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

    return bank.loadAccounts() && bank.loadTransactions();
}

void resetState()
{
    bank = Banksystem{};
    history = false;
    dirty = false;
    first = 0;
    selected.clear();
    keyword.clear();
    tip = L"请选择账户办理业务；金额单位为元。";
    tipColor = Muted;
    shown.clear();
}

void handleKeyDown(int key, bool& running)
{
    if (key != VK_ESCAPE)
    {
        return;
    }

    if (history)
    {
        history = false;
        first = 0;
        refresh();
    }
    else
    {
        running = !canExit();
    }
}

void handleMouseDown(int x, int y, bool& running)
{
    bool clickedBack = hit(x, y, 60, 90, 200, 30)
        || (history && hit(x, y, 60, 625, 160));
    if (clickedBack)
    {
        if (history)
        {
            history = false;
            first = 0;
            refresh();
        }
        else
        {
            running = !canExit();
        }
        return;
    }

    if (hit(x, y, 900, 575, 110))
    {
        first = std::max(0, first - Rows);
        return;
    }

    if (hit(x, y, 1030, 575, 110))
    {
        if (first + Rows < static_cast<int>(shown.size()))
        {
            first += Rows;
        }
        return;
    }

    if (history)
    {
        return;
    }

    if (hit(x, y, 60, 262, 1080, Rows * 38))
    {
        int position = first + (y - 262) / 38;
        if (position < static_cast<int>(shown.size()))
        {
            selected = bank.accounts[shown[position]]->accountId;
        }
        return;
    }

    if (hit(x, y, 680, 125, 140))
    {
        std::wstring input;
        if (ask(L"请输入账号或学号关键字（留空显示全部）", input))
        {
            keyword = input;
            selected.clear();
            first = 0;
            refresh();
        }
        return;
    }

    if (hit(x, y, 840, 125, 140))
    {
        keyword.clear();
        selected.clear();
        first = 0;
        refresh();
        return;
    }

    if (hit(x, y, 1000, 125, 140))
    {
        selected.clear();
        history = true;
        first = 0;
        refresh();
        return;
    }

    for (int actionIndex = 0; actionIndex < 8; ++actionIndex)
    {
        int left = 60 + actionIndex * 137;
        if (hit(x, y, left, 625, 121))
        {
            action(actionIndex);
            return;
        }
    }
}

void handleMessage(const ExMessage& msg, bool& running)
{
    if (msg.message == WM_KEYDOWN)
    {
        handleKeyDown(msg.vkcode, running);
    }
    else if (msg.message == WM_MOUSEWHEEL)
    {
        first = std::max(0, first + (msg.wheel > 0 ? -Rows : Rows));
        refresh();
    }
    else if (msg.message == WM_LBUTTONDOWN)
    {
        handleMouseDown(msg.x, msg.y, running);
    }
}

void runEventLoop()
{
    ExMessage msg;
    bool running = true;
    HWND window = GetHWnd();
    while (running && IsWindow(window))
    {
        if (!peekmessage(&msg, EM_MOUSE | EM_KEY))
        {
            draw();
            Sleep(10);
            continue;
        }

        handleMessage(msg, running);
        if (running)
        {
            draw();
        }
    }
}

} // namespace

void runBankGui()
{
    resetState();
    if (!load())
    {
        MessageBoxW(
            GetHWnd(),
            L"银行数据目录不可用，或账户/流水文件缺失、格式错误。\n请确保两份文件配套，原文件未修改。",
            L"加载失败",
            MB_OK | MB_ICONERROR);
        return;
    }

    refresh();
    draw();
    runEventLoop();
}
