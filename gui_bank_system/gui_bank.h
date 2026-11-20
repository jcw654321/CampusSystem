#pragma once
#include "user.h"
#include "BankRole.h"
#include <filesystem>

void runBankGui(const User& currentUser, BankRole role,
                const std::filesystem::path& dataDirectory);
