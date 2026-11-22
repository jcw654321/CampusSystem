#define NOMINMAX
#include <windows.h>
#include "Banksystem.h"
#include <filesystem>
#include <iostream>
#include <system_error>


using namespace std;

// 独立控制台测试入口。接入主系统时不编译本文件，避免出现两个 main。
// 此入口直接输入整数“分”；GUI 允许输入元，并转换成分后调用相同后端。
int main(int argc, char* argv[])
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    // 独立测试入口没有统一登录页，显式提供测试身份；不用于主系统认证。
    // 仅用于独立测试：main.exe 默认管理员；main.exe --user 用户号 测试学生。
    // 正式接入时必须由主系统提供已验证的角色和用户号，不能信任用户自选参数。
    bool administrator = true;
    string userId = "bank-console-test";
    if (argc == 3 && string(argv[1]) == "--user")
    {
        administrator = false;
        userId = argv[2];
        if (userId.empty()) return 1;
    }
    else if (argc != 1)
    {
        cout << "用法：main.exe 或 main.exe --user 用户号（仅测试身份）" << endl;
        return 1;
    }
    User testUser("银行控制台测试", "", userId, "");
    if (testUser.getUserid().empty())
    {
        cout << "银行需要有效的登录用户身份。" << endl;
        return 1;
    }
    Banksystem bankSystem(testUser, administrator);
    cout << (bankSystem.isAdministrator() ? "管理员测试模式" : "学生测试模式") << endl;
    error_code ec;
    filesystem::create_directories(bankSystem.getDataDirectory(), ec);
    if (ec)
    {
        cout << "无法创建银行数据目录：" << ec.message() << endl;
        return 1;
    }
    bool hasFile = filesystem::exists(bankSystem.getDataDirectory() / "accounts.txt", ec);
    if (ec)
    {
        cout << "无法检查账户文件：" << ec.message() << endl;
        return 1;
    }
    if (hasFile)
    {
        if (!bankSystem.loadAccounts())
        {
            cout << "账户文件读取失败，程序退出，原文件未修改。" << endl;
            return 1;
        }
        cout << "已恢复账户。" << endl;
    }
    // 控制台仅用于测试，不提供手动保存；图形界面负责自动持久化。
    // 管理员分支负责开户、销户和余额查询；学生分支只操作本人账户。
    while (true)
    {
        if (bankSystem.isAdministrator())
            cout << "0.退出  1.储蓄开户  2.信用开户  3.查询余额  4.销户" << endl;
        else
            cout << "0.退出  1.存款  2.取款  3.转账  4.本人余额" << endl;
        int choice;
        if (!(cin >> choice)) break;
        if (choice == 0)
        {
            break;
        }
        else if (bankSystem.isAdministrator() && (choice == 1 || choice == 2))
        {
            cout << "请输入用户号：";
            string ownerUserId;
            cin >> ownerUserId;
            string accountId = choice == 1 ? bankSystem.openSavingAccount(ownerUserId)
                                          : bankSystem.openCreditAccount(ownerUserId);
            if (accountId.empty()) cout << "开户失败：检查用户号和同类账户" << endl;
            else cout << "开户成功，账号：" << accountId << endl;
        }
        else if (bankSystem.isAdministrator() && choice == 4)
        {
            cout << "请输入待销户账号：";
            string accountId;
            cin >> accountId;
            cout << (bankSystem.closeAccount(accountId) ? "销户成功" : "销户失败：需要管理员权限且余额为 0") << endl;
        }
        else if (!bankSystem.isAdministrator() && choice == 1)
        {
            cout << "请输入账号：";
            string accountId;
            cin >> accountId;
            cout << "请输入存款金额：";
            int64_t amount;
            cin >> amount;
            if (bankSystem.deposit(accountId, amount))
            {
                cout << "存款成功" << endl;
            }
            else
            {
                cout << "存款失败" << endl;
            }
        }
        else if (!bankSystem.isAdministrator() && choice == 2)
        {
            cout << "请输入账号：";
            string accountId;
            cin >> accountId;
            cout << "请输入取款金额：";
            int64_t amount;
            cin >> amount;
            if (bankSystem.withdraw(accountId, amount))
            {
                cout << "取款成功" << endl;
            }
            else
            {
                cout << "取款失败" << endl;
            }
        }
        else if (!bankSystem.isAdministrator() && choice == 3)
        {
            cout << "请输入转出账号：";
            string fromAccountId;
            cin >> fromAccountId;
            cout << "请输入转入账号：";
            string toAccountId;
            cin >> toAccountId;
            cout << "请输入转账金额：";
            int64_t amount;
            cin >> amount;
            if (bankSystem.Transfer(fromAccountId, toAccountId, amount))
            {
                cout << "转账成功" << endl;
            }
            else
            {
                cout << "转账失败" << endl;
            }
        }
        else if ((bankSystem.isAdministrator() && choice == 3) ||
                 (!bankSystem.isAdministrator() && choice == 4))
        {
            cout << "请输入账号：";
            string accountId;
            cin >> accountId;
            const BankAccount *account = bankSystem.findAccount(accountId);
            if (account != nullptr)
            {
                cout << "余额为：" << account->getBalance() << "分" << endl;
            }
            else
            {
                cout << "账号不存在" << endl;
            }
        }
        else
        {
            cout << "选项无效" << endl;
        }
    }
}
