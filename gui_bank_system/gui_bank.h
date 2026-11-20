#pragma once
#include "user.h"
#include <filesystem>

// 调用前由主系统创建 1200×760 EasyX 窗口并开启批量绘图。
// 传入已经登录的用户及银行数据目录；返回时不关闭窗口、不改变工作目录。
// 当前不区分银行管理员和学生权限。
void runBankGui(const User& currentUser, const std::filesystem::path& dataDirectory);
