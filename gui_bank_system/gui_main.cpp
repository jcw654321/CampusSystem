#define NOMINMAX
#include <graphics.h>
#include <windows.h>
#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>
#include "Banksystem.h"
#include "CreditAccount.h"
#include <cstdint>
#include <limits>
#include <string>

// 从十进制文本直接解析“分”，避免浮点数舍入；不接受符号、指数或空白。
inline bool parseMoney(const std::wstring& text, std::int64_t& cents) {
    if (text.empty()) return false;
    std::int64_t value = 0;
    bool dot = false;
    int decimals = 0, integers = 0;
    const auto max = (std::numeric_limits<std::int64_t>::max)();
    for (wchar_t c : text) {
        if (c == L'.' && !dot && integers > 0) { dot = true; continue; }
        if (c < L'0' || c > L'9') return false;
        if (dot && ++decimals > 2) return false;
        if (!dot) ++integers;
        int digit = c - L'0';
        if (value > (max - digit) / 10) return false;
        value = value * 10 + digit;
    }
    if (dot && decimals == 0) return false;
    for (int i = decimals; i < 2; ++i) {
        if (value > max / 10) return false;
        value *= 10;
    }
    cents = value;
    return value > 0;
}

inline std::wstring moneyText(std::int64_t cents) {
    // 同样支持 INT64_MIN，不能直接对有符号数取负。
    auto magnitude = cents < 0 ? std::uint64_t(-(cents + 1)) + 1 : std::uint64_t(cents);
    auto fraction = magnitude % 100;
    return (cents < 0 ? L"-" : L"") + std::to_wstring(magnitude / 100)
        + L"." + (fraction < 10 ? L"0" : L"") + std::to_wstring(fraction);
}

namespace {
constexpr COLORREF Blue = RGB(0, 63, 136), Ink = RGB(33, 37, 41);
constexpr COLORREF Muted = RGB(108, 117, 125), Background = RGB(230, 236, 245);
constexpr int Rows = 8;
Banksystem bank;
bool history = false, dirty = false;
int first = 0;
std::string selected;
std::wstring keyword, tip = L"请选择账户办理业务；金额单位为元。";
COLORREF tipColor = Muted;
std::vector<size_t> shown;

std::wstring wide(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring result(n, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), result.data(), n);
    return result;
}
std::string utf8(const std::wstring& s) {
    if (s.empty()) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0, nullptr, nullptr);
    std::string result(n, '\0');
    WideCharToMultiByte(CP_UTF8, 0, s.data(), (int)s.size(), result.data(), n, nullptr, nullptr);
    return result;
}
void message(const std::wstring& s, bool error = true) {
    tip = s;
    tipColor = error ? RGB(190, 45, 45) : RGB(30, 125, 80);
}
bool ask(const wchar_t* prompt, std::wstring& value) {
    wchar_t buffer[128] = {};
    if (!InputBox(buffer, 128, prompt, L"银行管理")) return false;
    value = buffer;
    return true;
}
bool askId(const wchar_t* prompt, std::string& value) {
    std::wstring input;
    if (!ask(prompt, input)) return false;
    if (input.empty() || input.size() > 32 || input.find_first_not_of(
        L"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-") != std::wstring::npos) {
        message(L"编号须为 1–32 位字母、数字、下划线或短横线。");
        return false;
    }
    value = utf8(input);
    return true;
}
bool askAmount(const wchar_t* prompt, std::int64_t& amount) {
    std::wstring input;
    if (!ask(prompt, input)) return false;
    if (!parseMoney(input, amount)) {
        message(L"请输入大于 0 的金额，最多两位小数，且不得超出范围。");
        return false;
    }
    return true;
}

// 复用后端文件格式；失败时保留内存结果并阻止继续交易，允许重试保存。
bool save() {
    bool accountsOk = bank.saveAccounts();
    bool transactionsOk = accountsOk && bank.saveTransactions();
    dirty = !(accountsOk && transactionsOk);
    if (dirty) message(L"保存失败：结果仍在内存中，请点击“保存”重试，暂勿关闭程序。");
    else message(L"账户和交易流水已保存。", false);
    return !dirty;
}
void refresh() {
    shown.clear();
    if (history) {
        // 最近的流水排在前面；序号取原始位置，不虚构后端没有记录的时间。
        for (size_t i = bank.transactions.size(); i > 0; --i) {
            const auto& t = bank.transactions[i - 1];
            if (selected.empty() || t.fromAccountId == selected || t.toAccountId == selected)
                shown.push_back(i - 1);
        }
    } else {
        for (size_t i = 0; i < bank.accounts.size(); ++i) {
            const auto& a = bank.accounts[i];
            if (keyword.empty() || wide(a->accountId).find(keyword) != std::wstring::npos
                || wide(a->studentId).find(keyword) != std::wstring::npos) shown.push_back(i);
        }
    }
    first = std::min(first, shown.empty() ? 0 : int((shown.size() - 1) / Rows) * Rows);
}
void text(int x, int y, const std::wstring& s, int size = 18, COLORREF color = Ink) {
    settextstyle(size, 0, L"微软雅黑");
    settextcolor(color);
    outtextxy(x, y, s.c_str());
}
void cell(int x, int y, int width, std::wstring s) {
    settextstyle(16, 0, L"微软雅黑");
    if (textwidth(s.c_str()) > width) {
        while (!s.empty() && textwidth((s + L"…").c_str()) > width) s.pop_back();
        s += L"…";
    }
    text(x, y, s, 16);
}
void button(int x, int y, int width, const wchar_t* label, bool enabled = true) {
    setfillcolor(enabled ? Blue : RGB(164, 177, 194));
    setlinecolor(enabled ? Blue : RGB(164, 177, 194));
    fillroundrect(x, y, x + width, y + 40, 8, 8);
    settextstyle(18, 0, L"微软雅黑");
    text(x + (width - textwidth(label)) / 2, y + 9, label, 18, WHITE);
}
bool hit(int x, int y, int left, int top, int width, int height = 40) {
    return x >= left && x < left + width && y >= top && y < top + height;
}
void draw() {
    cleardevice();
    setbkmode(TRANSPARENT);
    setfillcolor(Blue); solidrectangle(0, 0, 1200, 72);
    text(40, 22, L"华南理工大学  校园综合管理系统", 26, WHITE);
    text(60, 92, history ? L"< 返回账户" : L"< 退出银行管理", 18, Blue);
    text(60, 130, history ? L"交易流水" : L"银行管理", 28);
    text(265, 141, history ? (selected.empty() ? L"全部账户" : L"账号：" + wide(selected))
        : L"储蓄账户 / 信用账户", 17, Muted);
    text(60, 180, L"账户总数  " + std::to_wstring(bank.accounts.size()) + L"    交易笔数  "
        + std::to_wstring(bank.transactions.size()) + (dirty ? L"    待保存" : L"    数据已同步"), 18, Blue);
    if (!history) {
        button(680, 125, 140, L"搜索账户"); button(840, 125, 140, L"显示全部");
        button(1000, 125, 140, L"全部流水");
    }
    setfillcolor(RGB(245, 248, 252)); solidrectangle(60, 222, 1140, 262);
    const int columns[] = {80, 320, 550, 710, 950};
    const wchar_t* accountHeaders[] = {L"账号", L"关联学号", L"账户类型", L"余额（元）", L"信用额度（元）"};
    const wchar_t* historyHeaders[] = {L"流水序号", L"交易类型", L"金额（元）", L"转出账号", L"转入账号"};
    for (int c = 0; c < 5; ++c) text(columns[c], 233, history ? historyHeaders[c] : accountHeaders[c], 16, Muted);
    for (int row = 0; row < Rows; ++row) {
        int y = 262 + row * 38, pos = first + row;
        bool exists = pos < (int)shown.size();
        bool active = exists && !history && bank.accounts[shown[pos]]->accountId == selected;
        setfillcolor(active ? RGB(220, 235, 255) : WHITE);
        solidrectangle(60, y, 1140, y + 38);
        if (!exists) continue;
        if (history) {
            const auto& t = bank.transactions[shown[pos]];
            cell(80, y + 10, 220, std::to_wstring(shown[pos] + 1));
            cell(320, y + 10, 210, wide(t.type));
            cell(550, y + 10, 140, moneyText(t.amount));
            cell(710, y + 10, 220, t.fromAccountId.empty() ? L"—" : wide(t.fromAccountId));
            cell(950, y + 10, 170, t.toAccountId.empty() ? L"—" : wide(t.toAccountId));
        } else {
            auto& a = bank.accounts[shown[pos]];
            auto credit = dynamic_cast<CreditAccount*>(a.get());
            cell(80, y + 10, 220, wide(a->accountId)); cell(320, y + 10, 210, wide(a->studentId));
            cell(550, y + 10, 140, credit ? L"信用账户" : L"储蓄账户");
            cell(710, y + 10, 220, moneyText(a->balance));
            cell(950, y + 10, 170, credit ? moneyText(credit->getCreditLimit()) : L"—");
        }
    }
    if (shown.empty()) text(400, 390, history ? L"暂无交易流水" : L"暂无匹配账户，可开户或显示全部", 20, Muted);
    text(60, 582, L"共 " + std::to_wstring(shown.size()) + L" 条    第 " + std::to_wstring(first / Rows + 1)
        + L" / " + std::to_wstring(std::max(1, ((int)shown.size() + Rows - 1) / Rows)) + L" 页", 16, Muted);
    button(900, 575, 110, L"上一页", first > 0);
    button(1030, 575, 110, L"下一页", first + Rows < (int)shown.size());
    if (!history) {
        const wchar_t* labels[] = {L"储蓄开户", L"信用开户", L"存款", L"取款", L"转账", L"账户详情", L"账户流水", L"保存"};
        for (int i = 0; i < 8; ++i)
            button(60 + i * 137, 625, 121, labels[i], i == 7 || (!dirty && (i < 2 || !selected.empty())));
    } else button(60, 625, 160, L"返回账户");
    text(60, 678, tip, 16, tipColor);
    setlinecolor(RGB(205, 212, 224)); line(0, 706, 1200, 706);
    text(405, 724, L"2026 华南理工大学 · 校园综合管理系统", 16, RGB(120, 128, 140));
    FlushBatchDraw();
}

void action(int which) {
    if (which == 7) { save(); return; }
    if (dirty) { message(L"请先重试保存，再办理下一笔业务。"); return; }
    if (which < 2) {
        std::string id, student;
        if (!askId(L"请输入新账号（1–32 位字母、数字、下划线或短横线）", id)) return;
        if (bank.accountExists(id)) { message(L"账号已存在，请使用其他账号。"); return; }
        if (!askId(L"请输入关联学号", student)) return;
        std::int64_t limit = 0;
        if (which == 1 && !askAmount(L"请输入信用额度（元，最多两位小数）", limit)) return;
        bool ok = which == 0 ? bank.openSavingAccount(id, student) : bank.openCreditAccount(id, student, limit);
        if (!ok) { message(L"开户失败，请检查账号和信用额度。"); return; }
        selected = id; keyword.clear(); first = (int(bank.accounts.size()) - 1) / Rows * Rows;
    } else {
        auto a = bank.findAccount(selected);
        if (!a) { message(L"请先点击表格选择账户。"); return; }
        if (which == 6) { history = true; first = 0; refresh(); return; }
        if (which == 5) {
            std::wstring info = L"账号：" + wide(a->accountId) + L"\n学号：" + wide(a->studentId)
                + L"\n余额：" + moneyText(a->balance) + L" 元";
            auto credit = dynamic_cast<CreditAccount*>(a);
            info += credit ? L"\n类型：信用账户\n信用额度：" + moneyText(credit->getCreditLimit()) + L" 元" : L"\n类型：储蓄账户";
            MessageBoxW(GetHWnd(), info.c_str(), L"账户详情", MB_OK | MB_ICONINFORMATION);
            return;
        }
        std::string target;
        if (which == 4) {
            if (!askId(L"请输入收款账号", target)) return;
            if (target == selected) { message(L"不能向同一账户转账。"); return; }
            if (!bank.accountExists(target)) { message(L"收款账户不存在。"); return; }
        }
        std::int64_t amount;
        if (!askAmount(L"请输入金额（元，大于 0，最多两位小数）", amount)) return;
        std::wstring summary = L"账号：" + wide(selected) + L"\n金额：" + moneyText(amount) + L" 元";
        if (which == 4) summary += L"\n收款账号：" + wide(target);
        if (MessageBoxW(GetHWnd(), summary.c_str(), which == 2 ? L"确认存款" : which == 3 ? L"确认取款" : L"确认转账",
            MB_OKCANCEL | MB_ICONQUESTION) != IDOK) return;
        bool ok = which == 2 ? bank.deposit(selected, amount) : which == 3 ? bank.withdraw(selected, amount) : bank.Transfer(selected, target, amount);
        if (!ok) { message(L"操作失败：可用余额不足、超出信用额度或收款余额超限。"); return; }
    }
    dirty = true;
    if (save()) message(L"业务办理成功，账户与流水已保存。", false);
    refresh();
}

bool canExit() {
    if (!dirty || save()) return true;
    MessageBoxW(GetHWnd(), L"数据尚未保存，请检查目录写入权限后重试。", L"保存失败", MB_OK | MB_ICONERROR);
    return false;
}

// 禁用系统关闭菜单，所有常规退出入口均经过 canExit，避免漏掉保存失败提示。
bool load() {
    std::error_code ec;
    bool hasAccounts = std::filesystem::exists("accounts.txt", ec);
    if (ec) return false;
    bool hasTransactions = std::filesystem::exists("transactions.txt", ec);
    if (ec || hasAccounts != hasTransactions) return false;
    if (!hasAccounts) return true;
    return bank.loadAccounts() && bank.loadTransactions();
}
} // namespace

int main() {
    // 固定到项目 bank_manage_system，启动方式不会改变数据位置。
    wchar_t executable[32768] = {};
    DWORD length = GetModuleFileNameW(nullptr, executable, 32768);
    if (length == 0 || length >= 32768) return 1;
    auto root = std::filesystem::path(executable).parent_path().parent_path().parent_path();
    std::error_code ec;
    std::filesystem::current_path(root / "bank_manage_system", ec);
    if (ec || !load()) {
        MessageBoxW(nullptr, L"银行数据目录不可用，或账户/流水文件缺失、格式错误。\n请确保两份文件配套，原文件未修改。", L"加载失败", MB_OK | MB_ICONERROR);
        return 1;
    }
    HANDLE lock = CreateMutexW(nullptr, TRUE, L"Local\\CampusBankGui");
    if (!lock || GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxW(nullptr, L"银行前端已经运行，请使用已打开的窗口。", L"提示", MB_OK);
        if (lock) CloseHandle(lock);
        return 1;
    }
    initgraph(1200, 760);
    SetWindowTextW(GetHWnd(), L"校园综合管理系统 - 银行管理");
    EnableMenuItem(GetSystemMenu(GetHWnd(), FALSE), SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
    setbkcolor(Background);
    BeginBatchDraw(); refresh(); draw();
    ExMessage msg;
    bool running = true;
    while (running && IsWindow(GetHWnd())) {
        if (!peekmessage(&msg, EM_MOUSE | EM_KEY)) { Sleep(10); continue; }
        if (msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE) {
            if (history) { history = false; first = 0; refresh(); }
            else running = !canExit();
        } else if (msg.message == WM_MOUSEWHEEL) {
            first = std::max(0, first + (msg.wheel > 0 ? -Rows : Rows)); refresh();
        } else if (msg.message == WM_LBUTTONDOWN) {
            int x = msg.x, y = msg.y;
            if (hit(x, y, 60, 90, 200, 30) || (history && hit(x, y, 60, 625, 160))) {
                if (history) { history = false; first = 0; refresh(); } else running = !canExit();
            } else if (hit(x, y, 900, 575, 110)) { first = std::max(0, first - Rows); }
            else if (hit(x, y, 1030, 575, 110)) { if (first + Rows < (int)shown.size()) first += Rows; }
            else if (!history) {
                if (hit(x, y, 60, 262, 1080, Rows * 38)) {
                    int pos = first + (y - 262) / 38;
                    if (pos < (int)shown.size()) selected = bank.accounts[shown[pos]]->accountId;
                } else if (hit(x, y, 680, 125, 140)) {
                    std::wstring input;
                    if (ask(L"请输入账号或学号关键字（留空显示全部）", input)) { keyword = input; selected.clear(); first = 0; refresh(); }
                } else if (hit(x, y, 840, 125, 140)) { keyword.clear(); selected.clear(); first = 0; refresh(); }
                else if (hit(x, y, 1000, 125, 140)) { selected.clear(); history = true; first = 0; refresh(); }
                else for (int i = 0; i < 8; ++i) if (hit(x, y, 60 + i * 137, 625, 121)) { action(i); break; }
            }
        }
        if (running) draw();
    }
    EndBatchDraw(); closegraph(); ReleaseMutex(lock); CloseHandle(lock);
    return 0;
}
