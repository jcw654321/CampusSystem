import csv
from pathlib import Path


FIELDS = ["用户ID", "用户名", "性别", "邮箱", "密码", "专业", "学号", "班级"]
SURNAMES = ["张", "王", "李", "赵", "陈", "刘", "杨", "黄", "周", "吴"]
GIVEN_NAMES = ["伟", "芳", "娜", "敏", "静", "强", "磊", "军", "洋", "勇"]
MAJORS = ["计算机科学与技术", "软件工程", "信息管理", "网络工程", "数据科学"]


def generate_users(count=100):
    users = []
    for number in range(1, count + 1):
        index = number - 1
        users.append(
            {
                "用户ID": f"U{number:04d}",
                "用户名": f"{SURNAMES[index // 10]}{GIVEN_NAMES[index % 10]}",
                "性别": "男" if number % 2 else "女",
                "邮箱": f"user{number:04d}@example.com",
                "密码": f"DemoOnly{number:04d}!",
                "专业": MAJORS[index % len(MAJORS)],
                "学号": f"2026{number:04d}",
                "班级": f"{MAJORS[index % len(MAJORS)]}{index % 3 + 1}班",
            }
        )
    return users


def main():
    output_path = Path(__file__).with_name("student_data.csv")
    with output_path.open("w", encoding="utf-8-sig", newline="") as csv_file:
        writer = csv.DictWriter(csv_file, fieldnames=FIELDS)
        writer.writeheader()
        writer.writerows(generate_users())
    print(f"已生成 100 条用户数据：{output_path}")


if __name__ == "__main__":
    main()