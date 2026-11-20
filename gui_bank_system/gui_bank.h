#pragma once
#include "user.h"
#include "BankRole.h"
#include <filesystem>
#include <string>

// 调用方先完成登录，只传入已登录用户、角色和银行数据目录。
// 如果角色是 Student，银行会根据 currentUser 的 userid 到学生档案中查学号；
// 管理员不需要学号。银行不读取 user.csv、不验证密码、不提供角色切换。
// 使用调用方已创建的 1200×760 EasyX 窗口与批量绘图，返回时不关闭窗口。
void runBankGui(const User& currentUser, BankRole role,
                const std::filesystem::path& dataDirectory);

// 兼容旧调用：调用方已经查好学生学号时可直接传入。
void runBankGui(const User& currentUser, BankRole role, const std::string& studentId,
                const std::filesystem::path& dataDirectory);
