#pragma once

#include <string>

// Requires an active 1200x760 EasyX window; caller owns its lifecycle.
// userId is the account id from user.csv (for example U0001).
void runStudentGui(const std::string& userId);
