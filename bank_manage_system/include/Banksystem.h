#pragma once

#include "BankAccount.h"
#include "Transaction.h"
#include "user.h"
#include <filesystem>
#include <memory>
#include <vector>

// 银行业务入口：管理全部账户和流水，不负责登录、绘图或读取用户输入。
// 业务函数修改内存，调用方另行调用保存函数写入文件。
// 返回 bool 的操作用 true 表示成功、false 表示失败；所有金额单位均为“分”。
class Banksystem : public User
{
  public:
    // 继承主系统已验证的用户身份。学号仍由账户保存，不与用户 ID 混用。
    explicit Banksystem(const User& currentUser,
                        const std::filesystem::path& dataDirectory = ".")
        : User(currentUser), dataDirectory(std::filesystem::absolute(dataDirectory)) {}

    const std::filesystem::path& getDataDirectory() const { return dataDirectory; }

    // unique_ptr 让容器统一管理账户生命周期，并保留储蓄/信用账户的实际类型。
    std::vector<std::unique_ptr<BankAccount>> accounts;
    std::vector<Transaction> transactions;

    // 1. 查询与开户。账号唯一；当前开户接口不查询学生档案是否存在。
    bool accountExists(const std::string &accountId) const;
    BankAccount *findAccount(const std::string &accountId); // 找不到时返回 nullptr
    bool openSavingAccount(const std::string &accountId, const std::string &studentId);
    bool openCreditAccount(const std::string &accountId, const std::string &studentId,
                           std::int64_t creditLimit);

    // 2. 交易。成功后追加一条流水；失败时不追加成功流水。
    bool deposit(const std::string &accountId, std::int64_t amount);
    bool withdraw(const std::string &accountId, std::int64_t amount);
    // 保留原有 Transfer 名称，兼容 GUI 和独立控制台入口。
    bool Transfer(const std::string &fromAccountId, const std::string &toAccountId,
                  std::int64_t amount);
    void showTransactions(const std::string &accountId); // 向控制台输出

    // 3. 文件读写，实现在 BanksystemFile.cpp；使用构造时确定的数据目录，不切换其他子系统的工作目录。
    // 加载账户要求 accounts 为空；加载流水前必须先加载账户。
    bool loadAccounts();
    bool loadTransactions();
    bool saveAccounts() const;
    bool saveTransactions() const;
    // 解析文件中的一行账户数据；格式或数值错误时返回 nullptr。
    std::unique_ptr<BankAccount> parseAccountLine(const std::string &line) const;
  private:
    std::filesystem::path dataDirectory;
};
