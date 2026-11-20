#pragma once

// 由调用方在登录成功后传入。银行界面不提供角色切换，也不重复验证密码。
enum class BankRole
{
    Administrator,
    Student
};
