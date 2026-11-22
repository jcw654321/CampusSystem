#pragma once
#include "user.h"
#include <filesystem>

void runBankGui(const User& currentUser, bool isAdministrator,
                const std::filesystem::path& dataDirectory);
