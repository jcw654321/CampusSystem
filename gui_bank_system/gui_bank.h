#pragma once
#include "user.h"
#include <filesystem>

// 主系统调用入口；已登录用户、角色和银行数据目录均由调用方传入。
void runBankGui(const User& currentUser, bool isAdministrator,
                const std::filesystem::path& dataDirectory);
