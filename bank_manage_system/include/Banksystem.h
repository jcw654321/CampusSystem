#pragma once

#include "BankAccount.h"
#include "user.h"
#include <filesystem>
#include <memory>
#include <vector>

// 继承已登录用户的身份；管理员开户和查询，普通用户只操作本人账户。
// 账户归属和权限均使用全局用户号，不依赖学生学号。
// 所有金额单位均为分。业务操作先修改内存，调用方另行保存。
class Banksystem : public User
{
public:
    Banksystem(const User& currentUser, bool isAdministrator,
               const std::filesystem::path& dataDirectory = "data");

    bool isAdministrator() const { return administrator; }
    const std::filesystem::path& getDataDirectory() const { return dataDirectory; }

    // 返回当前身份可查看的内容。账户只读，不能绕过交易函数直接改余额。
    // 返回的账户指针在此对象销毁或重新加载账户后失效，界面应重新获取列表。
    std::vector<const BankAccount*> getAccounts() const;
    const BankAccount* findAccount(const std::string& accountId) const;
    bool canOperateAccount(const std::string& accountId) const;

    // 仅告知账号是否存在，供转账检查收款方；不会暴露对方余额或用户号。
    bool accountExists(const std::string& accountId) const;
    // 开户成功返回后端生成的账号，失败返回空字符串；余额初始为 0。
    std::string openSavingAccount(const std::string& ownerUserId);
    std::string openCreditAccount(const std::string& ownerUserId);
    bool closeAccount(const std::string& accountId);
    bool userExists(const std::string& userId) const;
    bool deposit(const std::string& accountId, std::int64_t amount);
    bool withdraw(const std::string& accountId, std::int64_t amount);
    bool Transfer(const std::string& fromAccountId, const std::string& toAccountId,
                  std::int64_t amount);

    // 保存的是完整数据，不能把按用户过滤的列表写回文件而丢失其他人的账户。
    bool loadAccounts();
    bool saveAccounts() const;

private:
    std::string openAccount(const std::string& ownerUserId, bool credit);
    BankAccount* findAccountInternal(const std::string& accountId) const;
    bool canViewAccount(const BankAccount& account) const;
    std::unique_ptr<BankAccount> parseAccountLine(const std::string& line) const;
    bool administrator;
    std::filesystem::path dataDirectory;
    std::vector<std::unique_ptr<BankAccount>> accounts;
};
