#pragma once
#include "user.h"
#include "BankRole.h"
#include <filesystem>

// 调用方先完成登录，再传入角色和学生档案中的学号；管理员的学号传空字符串。
// 银行不读取 user.csv、不验证密码、不允许在界面里切换角色或修改当前学号。
// 使用调用方已创建的 1200×760 EasyX 窗口与批量绘图，返回时不关闭窗口。
void runBankGui(const User& currentUser, BankRole role, const std::string& studentId,
                const std::filesystem::path& dataDirectory);
