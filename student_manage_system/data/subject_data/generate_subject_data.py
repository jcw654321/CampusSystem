import csv
import random
from pathlib import Path


FIELDS = ["课程名称", "课程ID", "教师姓名", "课程成绩"]

COURSES = [
    ("高等数学", "C001", "王老师"),
    ("大学英语", "C002", "李老师"),
    ("数据结构", "C003", "张老师"),
    ("计算机网络", "C004", "陈老师"),
    ("操作系统", "C005", "刘老师"),
    ("软件工程", "C006", "赵老师"),
    ("人工智能", "C007", "孙老师"),
    ("数据库原理", "C008", "周老师")
]

script_dir = Path(__file__).resolve().parent
student_file_path = script_dir.parent / "student_data" / "student_data.csv"
data_dir = script_dir

def main():
    with open(student_file_path, "r", encoding="utf-8-sig", newline="") as file:
        students = list(csv.DictReader(file))

    rng = random.Random(2026)

    for student in students:
        user_id = (student.get("学号") or "").strip()
        if not user_id:
            raise ValueError("学生数据中存在空的学号")

        if user_id in {".", ".."} or any(char in user_id for char in '<>:"/\\|?*'):
            raise ValueError(f"学号不能用作文件名：{user_id}")

        output_file = data_dir / f"{user_id}.csv"
        with output_file.open("w", encoding="utf-8-sig", newline="") as file:
            writer = csv.DictWriter(file, fieldnames=FIELDS)
            writer.writeheader()
            selected_courses = rng.sample(COURSES, k=5)

            for name, course_id, teacher in selected_courses:
                writer.writerow(
                    {
                        "课程名称": name,
                        "课程ID": course_id,
                        "教师姓名": teacher,
                        "课程成绩": rng.randint(60, 100),
                    }
                )

    print(f"已为 {len(students)} 位用户生成课程成绩 CSV 文件：{data_dir}")


if __name__ == "__main__":
    main()