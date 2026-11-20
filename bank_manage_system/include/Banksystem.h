#pragma once

#include "BankAccount.h"
#include "BankRole.h"
#include "Transaction.h"
#include "user.h"
#include <filesystem>
#include <memory>
#include <vector>

// 继承已登录用户的身份；管理员开户和查询，学生只操作本人账户。
// currentStudentId 必须由调用方通过学生档案取得，不能让学生自行填入或用 userid 代替。
// 所有金额单位均为分。业务操作先修改内存，调用方另行保存。
class Banksystem : public User
{
public:
    Banksystem(const User& currentUser, BankRole role, const std::string& currentStudentId,
               const std::filesystem::path& dataDirectory = ".");

    bool isAdministrator() const { return role == BankRole::Administrator; }
    const std::string& getCurrentStudentId() const { return currentStudentId; }
    const std::filesystem::path& getDataDirectory() const { return dataDirectory; }

    // 返回当前身份可查看的内容。账户只读，不能绕过交易函数直接改余额。
    // 返回的账户指针在此对象销毁或重新加载账户后失效，界面应重新获取列表。
    std::vector<const BankAccount*> getAccounts() const;
    std::vector<Transaction> getTransactions() const;
    const BankAccount* findAccount(const std::string& accountId) const;
    bool canOperateAccount(const std::string& accountId) const;

    // 仅告知账号是否存在，供转账检查收款方；不会暴露对方余额或学号。
    bool accountExists(const std::string& accountId) const;
    bool openSavingAccount(const std::string& accountId, const std::string& studentId);
    bool openCreditAccount(const std::string& accountId, const std::string& studentId,
                           std::int64_t creditLimit);
    bool deposit(const std::string& accountId, std::int64_t amount);
    bool withdraw(const std::string& accountId, std::int64_t amount);
    bool Transfer(const std::string& fromAccountId, const std::string& toAccountId,
                  std::int64_t amount);
    void showTransactions(const std::string& accountId);

    // 保存的是完整数据，不能把学生可见列表写回文件而丢失其他人的账户。
    bool loadAccounts();
    bool loadTransactions();
    bool saveAccounts() const;
    bool saveTransactions() const;

private:
    BankAccount* findAccountInternal(const std::string& accountId) const;
    bool canViewAccount(const BankAccount& account) const;
    std::unique_ptr<BankAccount> parseAccountLine(const std::string& line) const;
    BankRole role;
    std::string currentStudentId;
    std::filesystem::path dataDirectory;
    std::vector<std::unique_ptr<BankAccount>> accounts;
    std::vector<Transaction> transactions;
};
