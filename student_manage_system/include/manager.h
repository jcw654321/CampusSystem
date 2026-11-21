#pragma once

#include "user.h"
#include <string>
#include <vector>
using namespace std;

struct DataRecord
{
    string key;
    vector<string> fields;
};

// 管理类公共基类：统一不同管理模块的文件数据操作接口
class Manager : public User
{
public:
    Manager(const string& name,const string& email,const string& userid,const string& password)
        : User(name, email, userid, password) {}

    virtual ~Manager() = default;

    virtual bool loadRecords() = 0;                            // 加载记录
    virtual bool saveRecords() const = 0;                      // 保存记录
    virtual bool addRecord(const DataRecord& record) = 0;      // 添加记录
    virtual bool removeRecord(const string& key) = 0;          // 删除记录
    virtual bool updateRecord(const string& key,const DataRecord& record) = 0;  // 更新记录
    virtual bool findRecord(const string& key,DataRecord& record) const = 0;    // 查找记录
};
