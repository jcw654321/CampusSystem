#pragma once

#include <cstdint>
#include <string>

// 一条成功交易的记录；amount 的单位为“分”。
// 存款：fromAccountId 为空；取款：toAccountId 为空；转账：两者都有值。
// 当前结构不保存交易时间，也不保存登录用户身份。
class Transaction
{
  public:
    std::string type;
    std::int64_t amount;
    std::string fromAccountId;
    std::string toAccountId;
};
