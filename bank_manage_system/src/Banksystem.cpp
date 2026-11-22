#include "Banksystem.h"
#include "SavingAccount.h"
#include "CreditAccount.h"
#include <fstream>
#include <limits>
#include <stdexcept>

using namespace std;

// 账户业务按“查询 → 开户 → 存取款 → 转账”阅读。
// 对具体账户调用 withdraw 时，由虚函数自动选择储蓄或信用取款规则。
Banksystem::Banksystem(const User& currentUser, bool isAdministrator,
                       const filesystem::path& dataDirectory)
    : User(currentUser), administrator(isAdministrator),
      dataDirectory(filesystem::absolute(dataDirectory))
{
    // 不验证密码，只检查调用方是否给出了完整的已登录身份。
    if (getUserid().empty())
        throw invalid_argument("银行需要有效的登录用户身份。");
}

BankAccount* Banksystem::findAccountInternal(const string& accountId) const
{
    // 此处只负责按账号查找；权限和销户状态由公开业务函数按场景检查。
    for (const auto& account : accounts)
        if (account->getAccountId() == accountId)
            return account.get();
    return nullptr;
}

bool Banksystem::accountExists(const string& accountId) const
{
    const auto* account = findAccountInternal(accountId);
    return account && !account->isClosed();
}

bool Banksystem::canViewAccount(const BankAccount& account) const
{
    // 管理员可查看所有有效账户，普通用户只能查看本人账户。
    return isAdministrator() || account.getOwnerUserId() == getUserid();
}

const BankAccount* Banksystem::findAccount(const string& accountId) const
{
    const BankAccount* account = findAccountInternal(accountId);
    if (account && !account->isClosed() && canViewAccount(*account))
        return account;
    return nullptr;
}

bool Banksystem::canOperateAccount(const string& accountId) const
{
    // 当前版本管理员不代办存取款、转账。
    // 此处统一检查登录角色、账户归属和销户状态；界面按钮状态不能替代后端校验。
    const BankAccount* account = findAccountInternal(accountId);
    return !administrator && account && !account->isClosed() &&
           account->getOwnerUserId() == getUserid();
}

vector<const BankAccount*> Banksystem::getAccounts() const
{
    vector<const BankAccount*> visible;
    for (const auto& account : accounts)
        if (!account->isClosed() && canViewAccount(*account))
            visible.push_back(account.get());
    return visible;
}

bool Banksystem::userExists(const string& userId) const
{
    if (userId.empty() || userId.find_first_of(", \t\r\n") != string::npos)
        return false;

    // 兼容不同启动目录：从银行数据目录及其上级目录查找用户数据。
    // 找到第一份可读的 user.csv 后即以它为准，避免不同位置的数据互相覆盖判断。
    vector<filesystem::path> roots = {
        dataDirectory, dataDirectory.parent_path(), dataDirectory.parent_path().parent_path()};
    for (const auto& root : roots)
    {
        ifstream in(root / "data" / "user.csv");
        if (!in) continue;

        string line;
        getline(in, line);
        while (getline(in, line))
        {
            if (!line.empty() && line.back() == '\r') line.pop_back();
            const auto separator = line.find(',');
            if (separator != string::npos && line.substr(0, separator) == userId)
                return true;
        }
        return false;
    }
    return false;
}

string Banksystem::openSavingAccount(const string& ownerUserId)
{
    return openAccount(ownerUserId, false);
}

string Banksystem::openCreditAccount(const string& ownerUserId)
{
    return openAccount(ownerUserId, true);
}

string Banksystem::openAccount(const string& ownerUserId, bool credit)
{
    if (!isAdministrator() || !userExists(ownerUserId)) return "";
    // 按用户号和账户类型检查重复，不能只比较新生成的账号。
    for (const auto& account : accounts)
    {
        bool sameType = (dynamic_cast<const CreditAccount*>(account.get()) != nullptr) == credit;
        if (!account->isClosed() && account->getOwnerUserId() == ownerUserId && sameType) return "";
    }
    // 账号由账户类型前缀和用户号组成；销户后重开仍复用原账号。
    string accountId = (credit ? "C" : "S") + ownerUserId;
    const auto* previous = findAccountInternal(accountId);
    if (previous && (!previous->isClosed() || previous->getOwnerUserId() != ownerUserId ||
        (dynamic_cast<const CreditAccount*>(previous) != nullptr) != credit)) return "";

    unique_ptr<BankAccount> created;
    if (credit) created = make_unique<CreditAccount>(accountId, ownerUserId);
    else created = make_unique<SavingAccount>(accountId, ownerUserId);
    // 重新开户沿用相同账号，不产生第二个有效同类账户。
    for (auto& account : accounts)
        if (account->getAccountId() == accountId)
        {
            account = move(created);
            return accountId;
        }
    accounts.push_back(move(created));
    return accountId;
}

bool Banksystem::closeAccount(const string& accountId)
{
    auto* account = findAccountInternal(accountId);
    if (!isAdministrator() || !account || account->isClosed() || account->getBalance() != 0)
        return false;
    account->closed = true; // 保留归属，但禁止后续存取款、收付款。
    return true;
}

bool Banksystem::withdraw(const string &accountId, int64_t amount)
{
    // 权限检查先于余额修改；账户类型决定实际取款规则。
    if (!canOperateAccount(accountId))
        return false;
    BankAccount *account = findAccountInternal(accountId);
    return account->withdraw(amount);
}

bool Banksystem::deposit(const string &accountId, int64_t amount)
{
    // 存款与取款一样，只允许普通用户操作本人未销户账户。
    if (!canOperateAccount(accountId))
        return false;
    BankAccount *account = findAccountInternal(accountId);
    return account->deposit(amount);
}

bool Banksystem::Transfer(const string &fromAccountId, const string &toAccountId, int64_t amount)
{
    if (!canOperateAccount(fromAccountId))
        return false;
    BankAccount *fromAccount = findAccountInternal(fromAccountId);
    BankAccount *toAccount = findAccountInternal(toAccountId);
    if (toAccount == nullptr || toAccount->isClosed())
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
    // 收款账户可以属于其他用户，但必须存在、未销户且能安全接收该金额。
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
        // 入账失败时退回刚才扣除的金额。
        fromAccount->deposit(amount);
        return false;
    }
    return true;
}
