#pragma once

#include <cstdint>
#include <string>

class Transaction {
public:
    std::string type;
    std::int64_t amount;
    std::string fromAccountId;
    std::string toAccountId;
};
