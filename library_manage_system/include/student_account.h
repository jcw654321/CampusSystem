#pragma once

#include <string>
#include <vector>

struct StudentAccount
{
    std::string stuId;
    std::string stuName;
    std::string stuPassword;
};

class StudentAccountStore
{
public:
    bool load(const std::string &csvPath, std::string &reason);

    const StudentAccount *findByStudentId(const std::string &id) const;

    bool verify(const std::string &studentId,
                const std::string &password,
                StudentAccount &out) const;

    int size() const;

private:
    std::vector<StudentAccount> students;
};
