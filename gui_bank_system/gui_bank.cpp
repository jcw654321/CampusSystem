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
#include "gui_bank_internal.h"

using namespace std;

// 银行前端负责显示页面、收集输入并调用后端。
// 开户权限、账户归属、余额和信用额度等规则由 bank_manage_system 检查。
// 阅读顺序：先看 runBankGui，再看 gui_bank_admin.cpp 和 gui_bank_student.cpp。

// 把用户输入的“元”转换成后端使用的整数“分”。
// 例如输入 12.5，输出 1250；直接处理数字字符，避免浮点数计算误差。
// 空输入、非数字、小数超过两位、金额为零或数值过大都会被拒绝。
bool parseMoney(const wstring &text, int64_t &cents)
{
    // 空字符串里没有可转换的金额。
    if (text.empty())
    {
        return false;
    }

    // value 暂时保存正在读取的数字。
    // hasDecimalPoint 记录是否已经遇到小数点。
    // integerDigits 和 decimalDigits 分别记录小数点前后的位数。
    int64_t value = 0;
    bool hasDecimalPoint = false;
    int decimalDigits = 0;
    int integerDigits = 0;
    // 金额最后以“分”保存，所以需要确保转换结果不超过 int64_t 上限。
    const auto maxCents = (numeric_limits<int64_t>::max)();

    // 一位一位读取数字并组成整数；小数部分不足两位时，后面补 0。
    for (wchar_t c : text)
    {
        // 小数点只能出现一次，而且前面必须先有整数位。
        if (c == L'.' && !hasDecimalPoint && integerDigits > 0)
        {
            hasDecimalPoint = true;
            continue;
        }

        // 除了数字和上面允许的小数点，其他字符都不是有效金额。
        if (c < L'0' || c > L'9')
        {
            return false;
        }

        // 统计数字位数；小数部分最多允许两位，因为 1 元等于 100 分。
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

        // wchar_t 数字字符减去 '0'，得到对应的数值，例如 '7' 变成 7。
        int digit = c - L'0';
        // 先检查数值是否会超出整数范围，再进行乘法和加法。
        if (value > (maxCents - digit) / 10)
        {
            return false;
        }

        value = value * 10 + digit;
    }

    if (hasDecimalPoint && decimalDigits == 0)
    {
        // 像 "12." 这样小数点后没有数字，视为输入错误。
        return false;
    }

    // 如果只输入一位小数（如 12.5），乘 10 补齐为两位小数（12.50）。
    // 如果没有小数点，则补两个 0（12 变成 12.00）。
    for (int i = decimalDigits; i < 2; ++i)
    {
        if (value > maxCents / 10)
        {
            return false;
        }

        value *= 10;
    }

    // 把最终的“分”写给调用者；存取款金额必须大于 0。
    cents = value;
    return value > 0;
}

wstring moneyText(int64_t cents)
{
    // 把整数“分”转换成显示用的“元”，例如 1250 变成 12.50。
    // 信用账户余额可能为负数。
    // 先得到不带正负号的数值，再拆成“元”和“分”。
    // 这里用无符号数是为了也能正确处理最小的负整数。
    auto magnitude = cents < 0 ? uint64_t(-(cents + 1)) + 1 : uint64_t(cents);
    auto fraction = magnitude % 100;
    // 整数部分是 magnitude / 100，小数部分是余数；小数不足两位时在前面补 0。
    return (cents < 0 ? L"-" : L"") + to_wstring(magnitude / 100) + L"." +
           (fraction < 10 ? L"0" : L"") + to_wstring(fraction);
}

namespace bank_gui
{
// 界面颜色、表格列的横坐标和每一列的宽度。
constexpr COLORREF Blue = RGB(0, 63, 136);
constexpr COLORREF Ink = RGB(33, 37, 41);
constexpr COLORREF Muted = RGB(108, 117, 125);
constexpr array<int, 3> ColumnX = {100, 440, 780};
constexpr array<int, 3> ColumnWidths = {280, 280, 250};

// 当前银行业务对象。它负责检查权限和执行银行操作。
unique_ptr<Banksystem> bank;
// 当前用户可以看到的账户。账户只读，修改必须通过后端业务函数。
vector<const BankAccount*> visibleAccounts;
bool hasUnsavedChanges = false; // 上次保存失败时为 true，下一次操作前会重试
int firstVisibleRow = 0;        // 当前页面从列表的第几条账户开始显示
string selectedAccountId;
wstring searchKeyword;             // 管理员当前使用的搜索关键字
wstring statusMessage;             // 页面底部显示的成功或失败提示
COLORREF statusColor = Muted;      // 提示文字颜色
// 记录哪些账户符合搜索条件；搜索和翻页不会改变账户本身。
vector<size_t> visibleIndices;

// 判断用户是否已经在表格里选中了一个账户。
bool hasSelectedAccount()
{
    return !selectedAccountId.empty();
}

// 根据选中的账号向后端查询账户；没有权限查看时后端会返回空指针。
const BankAccount *getSelectedAccount()
{
    return bank->findAccount(selectedAccountId);
}

// 请后端确认当前用户是否能操作选中的账户。
bool canOperateSelectedAccount()
{
    return bank->canOperateAccount(selectedAccountId);
}

// EasyX 页面使用宽字符串，后端使用 UTF-8 字符串，这两个函数负责互相转换。
wstring toWide(const string &text)
{
    // 空字符串不需要转换。
    if (text.empty())
    {
        return L"";
    }

    // 第一次调用只查询转换后需要多少个宽字符，不执行实际转换。
    int length =
        MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    // 按需要的长度准备空间，再进行第二次调用来填入转换结果。
    wstring result(length, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(),
                        length);
    return result;
}

string toUtf8(const wstring &text)
{
    // 空字符串不需要转换。
    if (text.empty())
    {
        return "";
    }

    // 第一次调用查询 UTF-8 结果的字节数。
    int length = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
                                     nullptr, 0, nullptr, nullptr);
    // 按字节数准备字符串空间，再执行真正的转换。
    string result(length, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(),
                        length, nullptr, nullptr);
    return result;
}

void setStatusMessage(const wstring &text, bool error)
{
    // 在页面底部显示操作结果；错误用红色，成功用绿色。
    statusMessage = text;
    statusColor = error ? RGB(190, 45, 45) : RGB(30, 125, 80);
}

bool inputText(const wchar_t *prompt, wstring &value)
{
    wchar_t buffer[128] = {};
    // 弹出输入框；用户按取消时返回 false。
    if (!InputBox(buffer, 128, prompt, L"银行管理"))
    {
        return false;
    }

    value = buffer;
    return true;
}

bool inputId(const wchar_t *prompt, string &value)
{
    // 先通过通用输入框取得用户输入。
    wstring input;
    if (!inputText(prompt, input))
    {
        return false;
    }

    // 账号和用户号会写入文本文件，因此限制为简单的编号字符。
    // 这样就不会把空格等分隔符写进账号字段，影响后端读取文件。
    if (input.empty() || input.size() > 32 ||
        input.find_first_not_of(
            L"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-") !=
            wstring::npos)
    {
        setStatusMessage(L"编号须为 1–32 位字母、数字、下划线或短横线。");
        return false;
    }

    // 检查通过后转成后端使用的 UTF-8 字符串。
    value = toUtf8(input);
    return true;
}

bool inputAmount(const wchar_t *prompt, int64_t &amount)
{
    // 先收集文本，再转换为整数分；输入取消或格式错误时返回 false。
    wstring input;
    if (!inputText(prompt, input))
    {
        return false;
    }

    // 解析失败时给出提示，告诉调用方本次操作不能继续。
    if (!parseMoney(input, amount))
    {
        setStatusMessage(L"请输入大于 0 的金额，最多两位小数，且不得超出范围。");
        return false;
    }

    return true;
}

// 把后端当前的全部账户保存到文件。
// 保存失败时账户变更仍留在内存里，后续操作或退出时再尝试保存。
bool saveBankData()
{
    // 让后端把所有账户写入账户文件，并记录是否保存成功。
    bool accountsOk = bank->saveAccounts();
    // 保存失败就记下“还有未保存的变更”，以后需要重试。
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
    // 没有未保存的变更就继续；有的话先保存，保存失败则不继续新操作。
    return !hasUnsavedChanges || saveBankData();
}

// 四、准备账户列表：先由后端筛权限，再由前端按关键字筛选。
void refreshVisibleRows()
{
    // 后端返回当前身份可以查看的账户；前端不会自行决定谁能看哪些账户。
    visibleAccounts = bank->getAccounts();
    // 清空上次的搜索结果，准备重新筛选。
    visibleIndices.clear();
    for (size_t i = 0; i < visibleAccounts.size(); ++i)
    {
        const auto &account = visibleAccounts[i];
        // 没有关键字时显示所有可见账户；否则账号或用户号包含关键字才显示。
        bool matchesKeyword =
            searchKeyword.empty() ||
            toWide(account->getAccountId()).find(searchKeyword) != wstring::npos ||
            toWide(account->getOwnerUserId()).find(searchKeyword) != wstring::npos;
        if (matchesKeyword)
        {
            // 只记下符合条件的账户位置，不复制或移动账户对象。
            visibleIndices.push_back(i);
        }
    }

    // 计算最后一页的起始位置，避免筛选后还停留在不存在的页码。
    int lastPageStart =
        visibleIndices.empty()
            ? 0
            : static_cast<int>((visibleIndices.size() - 1) / RowsPerPage) * RowsPerPage;
    firstVisibleRow = min(firstVisibleRow, lastPageStart);
}

void scrollPage(int offset)
{
    // offset 通常是正负一页的行数；计算列表最后一页的起点。
    int lastPageStart = visibleIndices.empty()
                            ? 0
                            : static_cast<int>((visibleIndices.size() - 1) / RowsPerPage) *
                                  RowsPerPage;
    // 限制起始行不能小于 0，也不能超过最后一页的起点。
    firstVisibleRow = max(0, min(firstVisibleRow + offset, lastPageStart));
}

// 五、绘图小工具：文字、表格单元格和按钮。
void drawText(int x, int y, const wstring &s, int size = 18, COLORREF color = Ink)
{
    // 设置文字样式和颜色，再把文字画在窗口坐标 (x, y)。
    settextstyle(size, 0, L"微软雅黑");
    settextcolor(color);
    outtextxy(x, y, s.c_str());
}

void drawCell(int x, int y, int width, wstring value)
{
    // 表格文字统一使用 16 号字体。
    settextstyle(16, 0, L"微软雅黑");
    // 内容太长时截短并加省略号，避免文字画到相邻列上。
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
    // enabled 为 true 时用蓝色；为 false 时用灰色表示暂不可操作。
    setfillcolor(enabled ? Blue : RGB(164, 177, 194));
    setlinecolor(enabled ? Blue : RGB(164, 177, 194));
    fillroundrect(x, y, x + width, y + 40, 8, 8);
    // 按钮文字放在按钮中间。
    settextstyle(18, 0, L"微软雅黑");
    drawText(x + (width - textwidth(label)) / 2, y + 9, label, 18, WHITE);
}

// 画账户表格的列标题。
void drawTableHeader()
{
    // 先画表头背景，再逐列画出标题。
    setfillcolor(RGB(245, 248, 252));
    solidrectangle(60, 222, 1140, 262);

    const wchar_t *accountHeaders[] = {L"账号", L"关联用户号", L"账户类型"};

    for (int column = 0; column < 3; ++column)
    {
        drawText(ColumnX[column], 233, accountHeaders[column], 16, Muted);
    }
}

// 画当前页的账户行，并标出当前选中的账户。
void drawRows()
{
    // 一次固定画 RowsPerPage 行；最后一页不够的行保持为空白。
    for (int row = 0; row < RowsPerPage; ++row)
    {
        // row 是当前页里的行号；position 是整份筛选列表里的位置。
        int y = 262 + row * 38;
        int position = firstVisibleRow + row;
        bool exists = position < static_cast<int>(visibleIndices.size());
        // 如果该行对应的账号正好是选中账号，就高亮这一行。
        bool active = exists &&
                      visibleAccounts[visibleIndices[position]]->getAccountId() == selectedAccountId;

        setfillcolor(active ? RGB(220, 235, 255) : WHITE);
        solidrectangle(60, y, 1140, y + 38);
        if (!exists)
        {
            // 这一行没有账户数据，留白后继续画下一行。
            continue;
        }

        // 通过 visibleIndices 找到账户在 visibleAccounts 中的位置。
        size_t index = visibleIndices[position];
        array<wstring, 3> values;
        const auto& account = visibleAccounts[index];
        // 准备要显示的三列：账号、所属用户号和账户类型。
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

// 画两个角色共用的页面部分；具体操作按钮由管理员或学生页面绘制。
void drawBankPageFrame(const wchar_t *title)
{
    // 每次重画先清屏，然后按顺序画页眉、标题、表格、提示和页脚。
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

// 六、业务操作：收集用户输入后调用 Banksystem，前端不直接改余额。
bool openAccount(bool creditAccount)
{
    string ownerUserId;
    // 先询问为哪个用户开户；取消输入就结束本次操作。
    if (!inputId(L"请输入关联用户号（账号自动生成）", ownerUserId)) return false;
    // 先给出具体提示；后端开户时也会检查用户是否存在及是否允许开户。
    if (!bank->userExists(ownerUserId))
    {
        setStatusMessage(L"未找到该用户，无法开户。");
        return false;
    }

    string accountId;
    // 根据按钮选择调用后端的储蓄开户或信用开户函数。
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
        // 后端用空字符串表示开户失败。
        setStatusMessage(L"开户失败：该用户已有同类账户，或账号不符合要求。");
        return false;
    }

    // 开户成功后自动选中新账户，并清空搜索，确保新账户能显示出来。
    selectedAccountId = accountId;
    searchKeyword.clear();
    return true;
}

bool closeSelectedAccount()
{
    // 先从后端取得选中账户；若已不存在或当前用户不可见，就不能继续。
    const auto* account = bank->findAccount(selectedAccountId);
    if (!account) return false;
    // 先说明为什么不能销户，再向用户确认；最终仍由后端执行销户。
    if (account->getBalance() != 0)
    {
        setStatusMessage(L"余额必须为 0 才能销户，请先取出余额或还清欠款。");
        return false;
    }
    // 销户不可随意撤销，因此先弹出确认窗口。
    wstring prompt = L"确定注销账户 " + toWide(selectedAccountId) +
                     L" 吗？\n销户后不能交易。";
    if (MessageBoxW(GetHWnd(), prompt.c_str(), L"确认销户", MB_YESNO | MB_ICONQUESTION) != IDYES)
        return false;
    // 确认后调用后端真正销户；后端会再检查权限和余额。
    if (!bank->closeAccount(selectedAccountId))
    {
        setStatusMessage(L"销户失败，请检查权限和账户余额。");
        return false;
    }
    // 销户成功后清除选择，避免继续把已销户账户当作当前操作对象。
    selectedAccountId.clear();
    return true;
}

void showAccountBalance(const BankAccount &account)
{
    // 将余额格式化成“元”；信用账户额外显示信用额度。
    wstring info = L"账号：" + toWide(account.getAccountId()) +
                   L"\n余额：" + moneyText(account.getBalance()) + L" 元";
    // 只有信用账户有额度，所以先判断账户实际类型再显示额度。
    if (const auto *credit = dynamic_cast<const CreditAccount *>(&account))
        info += L"\n信用额度：" + moneyText(credit->getCreditLimit()) + L" 元";
    MessageBoxW(GetHWnd(), info.c_str(), L"账户余额", MB_OK | MB_ICONINFORMATION);
}

// 处理存款、取款或转账。只有转账需要额外输入收款账号并确认信息。
bool performTransaction(StudentBankAction type)
{
    string target;
    if (type == StudentBankAction::Transfer)
    {
        // 先检查收款账号并给出具体提示，真正转账时后端还会再检查。
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
    // 存款、取款、转账都需要金额；输入框中的单位是元。
    if (!inputAmount(L"请输入金额（元，大于 0，最多两位小数）", amount))
    {
        return false;
    }

    if (type == StudentBankAction::Transfer)
    {
        // 最后把转出账号、金额和收款账号展示出来，请用户确认。
        wstring summary = L"账号：" + toWide(selectedAccountId) +
                          L"\n金额：" + moneyText(amount) + L" 元\n收款账号：" + toWide(target);
        if (MessageBoxW(GetHWnd(), summary.c_str(), L"确认转账",
                        MB_OKCANCEL | MB_ICONQUESTION) != IDOK)
            return false;
    }

    bool succeeded = false;
    // 根据按钮对应的操作，调用后端执行真正的交易。
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
        // 后端只返回成功或失败；这里用简明提示说明常见失败原因。
        setStatusMessage(L"操作失败：无权操作、金额不足或超出额度范围。");
    }
    return succeeded;
}

void finishBankAction(bool changed, bool openingAccount, bool closingAccount)
{
    // 只有业务成功并改变了账户数据才保存；取消或失败时不写文件。
    if (!changed)
        return;

    // 业务已修改账户；先标记为未保存，再尝试立刻保存。
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

    // 账户集合可能变了，重新生成表格数据。
    refreshVisibleRows();
    if (openingAccount)
    {
        firstVisibleRow =
            (static_cast<int>(visibleAccounts.size()) - 1) / RowsPerPage * RowsPerPage;
    }
}

bool canLeaveBank()
{
    // 如果之前保存失败，退出前再试一次；仍失败就留在页面，避免丢失变更。
    if (!hasUnsavedChanges)
    {
        return true;
    }
    // 再保存一次；如果还是失败，提示用户并阻止退出。
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
    // 创建账户数据目录；如果目录创建或检查失败，银行页面无法正常使用。
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

    // 文件存在时交给后端读取并检查账户内容。
    return bank->loadAccounts();
}

void applyAccountSearch(const wstring &keyword)
{
    // 应用搜索条件并清除旧选择，避免继续操作已经被筛掉的账户。
    searchKeyword = keyword;
    selectedAccountId.clear();
    // 新的搜索结果从第一页开始显示。
    firstVisibleRow = 0;
    // 按新关键字重新生成显示列表。
    refreshVisibleRows();
}

void selectAccountAt(int y, const RECT &tableRect)
{
    // 根据鼠标点击的位置，算出用户选中了当前页的哪一行。
    int position = firstVisibleRow + (y - tableRect.top) / 38;
    // 只有点击到有账户的行时，才更新选中的账号。
    if (position < static_cast<int>(visibleIndices.size()))
        selectedAccountId = visibleAccounts[visibleIndices[position]]->getAccountId();
}

} // namespace bank_gui

using namespace bank_gui;

void runBankGui(const User& currentUser, const filesystem::path& dataDirectory)
{
    // 用户号以 B 开头时进入管理员页面，否则进入普通用户页面。
    const string userId = currentUser.getUserid();
    const bool isAdministrator = !userId.empty() && userId.front() == 'B';
    // 用主系统已经登录的用户创建银行后端对象。
    bank = make_unique<Banksystem>(currentUser, isAdministrator, dataDirectory);
    // 每次进入页面都清空上次留下的搜索、选择和保存状态。
    visibleAccounts.clear();
    hasUnsavedChanges = false;
    firstVisibleRow = 0;
    selectedAccountId.clear();
    searchKeyword.clear();
    visibleIndices.clear();
    statusMessage.clear();
    statusColor = Muted;
    // 先恢复账户文件；加载失败就提示并返回主系统，不显示不完整的数据。
    if (!loadBankData())
    {
        MessageBoxW(GetHWnd(),
                    L"银行数据目录不可用，或账户文件缺失、格式错误。\n原文件未修改。",
                    L"加载失败", MB_OK | MB_ICONERROR);
        return;
    }

    // 加载成功后准备账户列表，再根据身份运行对应的界面循环。
    refreshVisibleRows();
    if (bank->isAdministrator())
        runAdminBankFrontend();
    else
        runStudentBankFrontend();
}


