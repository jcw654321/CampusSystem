#include <windows.h>
#include "user_manager.h"
#include <algorithm>
#include <fileoperate.h>
#include<fstream>
#include<filesystem>
#include<sstream>
#include"user.h"


static bool readUserRows(const string& path, vector<vector<string>>& rows)
{
    ifstream input(path, ios::binary);
    string header;
    if (!input || !getline(input, header))
        return false;

    string line;
    while (getline(input, line))
    {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (line.empty())
            continue;

        vector<string> fields;
        if (!parseCsvLine(line, fields) || fields.size() != 5)
            return false;
        rows.push_back(fields);
    }
    return true;
}

static bool writeUserRows(const string& path, const vector<vector<string>>& rows)
{
    filesystem::path targetPath(path);
    filesystem::path temporaryPath = targetPath;
    temporaryPath += ".tmp";

    ofstream output(temporaryPath, ios::binary | ios::trunc);
    if (!output)
        return false;

    output << "\xEF\xBB\xBF" << "用户ID,用户名,性别,邮箱,密码\n";
    for (const vector<string>& row : rows)
    {
        for (size_t i = 0; i < row.size(); ++i)
        {
            if (i != 0)
                output << ',';
            output << escapeCsvField(row[i]);
        }
        output << '\n';
    }
    output.close();
    if (!output)
    {
        filesystem::remove(temporaryPath);
        return false;
    }

    if (!MoveFileExW(temporaryPath.c_str(), targetPath.c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        filesystem::remove(temporaryPath);
        return false;
    }
    return true;
}

UserManager::UserManager(std::string dataf):dataFile(dataf){}

void UserManager::loadData() 
{
    vector<vector<string>> rows;
    if (!readUserRows(dataFile, rows))
    {
        cerr << "Failed to load user data from " << dataFile << ".\n";
        users.clear();
        return;
    }

    vector<User> loadedUsers;
    loadedUsers.reserve(rows.size());
    for (const vector<string>& row : rows)
        loadedUsers.emplace_back(row[1], row[3], row[0], row[4], row[2]);
    users.swap(loadedUsers);
}

void UserManager::addUser(const User& user) {
    users.push_back(user);
}

void UserManager::removeUser(const User& user) {
    auto it = std::remove_if(users.begin(), users.end(),
        [&user](const User& u) { return u.getUserid() == user.getUserid(); });
    users.erase(it, users.end());
}

bool UserManager::SaveUser(const User& user, const std::string& previousUserid)
{
    if (user.getUserid().empty())
        return false;

    vector<vector<string>> rows;
    if (!readUserRows(dataFile, rows))
        return false;

    const string userid = user.getUserid();
    const vector<string> updatedRow = {
        userid, user.getName(), user.getSex(), user.getEmail(), user.getPassword()
    };
    const auto matchingCount = count_if(rows.begin(), rows.end(),
        [&userid](const vector<string>& row) { return row[0] == userid; });
    if (matchingCount > 1 || (matchingCount == 1 && previousUserid != userid))
        return false;

    if (matchingCount == 1)
    {
        const auto existing = find_if(rows.begin(), rows.end(),
            [&userid](const vector<string>& row) { return row[0] == userid; });
        *existing = updatedRow;
    }
    else
    {
        if (!previousUserid.empty())
        {
            rows.erase(remove_if(rows.begin(), rows.end(),
                [&previousUserid](const vector<string>& row) {
                    return row[0] == previousUserid;
                }), rows.end());
        }
        rows.push_back(updatedRow);
    }

    if (!writeUserRows(dataFile, rows))
        return false;

    users.erase(std::remove_if(users.begin(), users.end(),
        [&userid, &previousUserid](const User& current) {
            return current.getUserid() == userid ||
                   (!previousUserid.empty() && current.getUserid() == previousUserid);
        }), users.end());
    users.emplace_back(user.getName(), user.getEmail(), userid,
                       user.getPassword(), user.getSex());
    return true;
}

bool UserManager::DeleteUser(const std::string& userid)
{
    if (userid.empty())
        return false;

    vector<vector<string>> rows;
    if (!readUserRows(dataFile, rows))
        return false;

    rows.erase(std::remove_if(rows.begin(), rows.end(),
        [&userid](const vector<string>& row) { return row[0] == userid; }), rows.end());
    if (!writeUserRows(dataFile, rows))
        return false;

    users.erase(std::remove_if(users.begin(), users.end(),
        [&userid](const User& user) { return user.getUserid() == userid; }), users.end());
    return true;
}

User* UserManager::findUser(const std::string& username) {
    for (auto& user : users) {
        if (user.getUserid() == username) {
            return &user;
        }
    }
    return nullptr;
}

bool UserManager::authenticateUser(const std::string& username, const std::string& password) {
    User* user = findUser(username);
    if (user) {
        return user->getPassword() == password;
    }
    return false;
}

