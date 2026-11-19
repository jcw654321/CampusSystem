#pragma once
#include "library_system.h"

class LibraryAdminSystem : public LibrarySystem
{
public:
    using LibrarySystem::LibrarySystem;   // 继承构造函数，不用重写


    bool canManageBooks() const override
    {
        return true;
    }
};