#include "Banksystem.h"
#include "SavingAccount.h"
#include "CreditAccount.h"
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace std;

// 账户业务按“查询 → 开户 → 存取款 → 转账 → 流水查询”阅读。
// 对具体账户调用 withdraw 时，由虚函数自动选择储蓄或信用取款规则。
namespace
{
constexpr double DefaultSavingInterestRate = 0.05;
}

Banksystem::Banksystem(const User& currentUser, BankRole role,
                       const string& currentStudentId, const filesystem::path& dataDirectory)
    : User(currentUser), role(role), currentStudentId(currentStudentId),
      dataDirectory(filesystem::absolute(dataDirectory))
{
    // 不验证密码，只检查调用方是否给出了完整的已登录身份。
    if (getUserid().empty() ||
        (role != BankRole::Administrator && role != BankRole::Student) ||
        (role == BankRole::Student && currentStudentId.empty()))
        throw invalid_argument("银行需要有效角色；学生身份还必须包含已关联的学号。");
}

BankAccount* Banksystem::findAccountInternal(const string& accountId) const
{
    for (const auto& account : accounts)
        if (account->accountId == accountId)
            return account.get();
    return nullptr;
}

bool Banksystem::accountExists(const string& accountId) const
{
    return findAccountInternal(accountId) != nullptr;
}

bool Banksystem::canViewAccount(const BankAccount& account) const
{
    return isAdministrator() || account.studentId == currentStudentId;
}

const BankAccount* Banksystem::findAccount(const string& accountId) const
{
    const BankAccount* account = findAccountInternal(accountId);
    if (account && canViewAccount(*account))
        return account;
    return nullptr;
}

bool Banksystem::canOperateAccount(const string& accountId) const
{
    // 当前版本管理员不代办存取款、转账。
    const BankAccount* account = findAccountInternal(accountId);
    return role == BankRole::Student && account && account->studentId == currentStudentId;
}

vector<const BankAccount*> Banksystem::getAccounts() const
{
    vector<const BankAccount*> visible;
    for (const auto& account : accounts)
        if (canViewAccount(*account))
            visible.push_back(account.get());
    return visible;
}

vector<Transaction> Banksystem::getTransactions() const
{
    vector<Transaction> visible;
    for (const auto& transaction : transactions)
    {
        // 学生能看到涉及本人任一账户的流水，包括别人转给自己的记录。
        if (isAdministrator() || canOperateAccount(transaction.fromAccountId) ||
            canOperateAccount(transaction.toAccountId))
            visible.push_back(transaction);
    }
    return visible;
}

bool Banksystem::openSavingAccount(const string &accountId, const string &studentId)
{
    if (!isAdministrator() || accountId.empty() || studentId.empty() || accountExists(accountId))
    {
        return false;
    }
    // make_unique 创建账户并交给容器管理，不需要手动 delete。
    accounts.push_back(make_unique<SavingAccount>(accountId, studentId, DefaultSavingInterestRate));
    return true;
}

bool Banksystem::openCreditAccount(const string &accountId, const string &studentId,
                                   int64_t creditLimit)
{
    if (!isAdministrator() || accountId.empty() || studentId.empty() ||
        creditLimit < 0 || accountExists(accountId))
    {
        return false;
    }
    accounts.push_back(make_unique<CreditAccount>(accountId, studentId, creditLimit));
    return true;
}

bool Banksystem::withdraw(const string &accountId, int64_t amount)
{
    if (!canOperateAccount(accountId))
        return false;
    BankAccount *account = findAccountInternal(accountId);
    if (account == nullptr)
    {
        return false;
    }
    if (!account->withdraw(amount))
    {
        return false;
    }
    // 账户操作成功后再记录流水，避免失败操作留下成功记录。
    transactions.push_back({"取款", amount, accountId, ""});
    return true;
}

bool Banksystem::deposit(const string &accountId, int64_t amount)
{
    if (!canOperateAccount(accountId))
        return false;
    BankAccount *account = findAccountInternal(accountId);
    if (account == nullptr)
    {
        return false;
    }
    if (!account->deposit(amount))
    {
        return false;
    }
    transactions.push_back({"存款", amount, "", accountId});
    return true;
}

bool Banksystem::Transfer(const string &fromAccountId, const string &toAccountId, int64_t amount)
{
    if (!canOperateAccount(fromAccountId))
        return false;
    BankAccount *fromAccount = findAccountInternal(fromAccountId);
    BankAccount *toAccount = findAccountInternal(toAccountId);
    if (fromAccount == nullptr || toAccount == nullptr)
    {
        return false;
    }
    if (fromAccountId == toAccountId)
    {
        return false;
    }
    if (amount <= 0)
    {
        return false;
    }
    // 先确认收款方能接收金额，再扣付款方的钱。
    if (toAccount->getBalance() > numeric_limits<int64_t>::max() - amount)
    {
        return false;
    }
    // 信用账户允许在额度内透支，储蓄账户不允许透支。
    if (!fromAccount->withdraw(amount))
    {
        return false;
    }
    if (!toAccount->deposit(amount))
    {
        // 入账失败时退回刚才扣除的金额，不生成流水。
        fromAccount->deposit(amount);
        return false;
    }
    transactions.push_back({"转账", amount, fromAccountId, toAccountId});
    return true;
}

void Banksystem::showTransactions(const string &accountId)
{
    if (!findAccount(accountId))
    {
        cout << "账号不存在或无权查看" << endl;
        return;
    }
    bool found = false;
    for (const auto &transaction : getTransactions())
    {
        if (transaction.fromAccountId == accountId || transaction.toAccountId == accountId)
        {
            found = true;
            cout << "交易类型: " << transaction.type << ", 金额: " << transaction.amount;
            if (!transaction.fromAccountId.empty())
            {
                cout << ", 转出账号: " << transaction.fromAccountId;
            }
            if (!transaction.toAccountId.empty())
            {
                cout << ", 转入账号: " << transaction.toAccountId;
            }
            cout << endl;
        }
    }
    if (!found)
    {
        cout << "该账户暂无交易记录" << endl;
    }
}
