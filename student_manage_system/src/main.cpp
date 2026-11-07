#include <vector>
#include "student_manager.h"
using namespace std;


int main() {
    std::vector<Student> students;

    StudentManager manager("管理员", "admin@example.com", "admin", "123456");

    // Student 参数顺序：姓名、邮箱、用户ID、密码、专业、学号、班级、年级
    Student student("张三", "zhangsan@example.com", "user001", "123456",
                    "计算机科学", "2026001", "计算机1班", "2026");

    manager.addStudent(students, student);

    Student result = manager.getStudent(students, "2026001");
    manager.printStudentInfo(result);

    return 0;
}