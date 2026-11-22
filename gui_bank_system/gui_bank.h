#pragma once
#include "user.h"
#include <filesystem>

// 主系统传入已登录用户和数据目录；银行角色按用户号前缀判定。
void runBankGui(const User& currentUser, const std::filesystem::path& dataDirectory);
