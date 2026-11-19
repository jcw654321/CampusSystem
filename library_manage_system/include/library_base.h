#pragma once

#pragma once

class LibraryBase
{
public:
    virtual ~LibraryBase() = default;

    
    // 默认不是管理员
    virtual bool canManageBooks() const { return false; }
};