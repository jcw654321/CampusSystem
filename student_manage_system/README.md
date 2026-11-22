# 学生管理子系统

基于 C++17 实现的学生与课程管理模块，可作为独立控制台程序运行，也可由校园综合管理系统的 EasyX GUI 调用。

## 功能

### 学生信息管理

- 查看全部学生
- 按学号查询学生
- 新增学生
- 修改学生信息
- 删除学生
- 用户账号、学生档案和数据文件同步更新

### 课程与成绩管理

- 查看指定学生的课程
- 为学生添加课程
- 删除学生课程
- 录入或修改课程成绩
- 防止重复课程编号
- 成绩范围限定为 `0` 到 `100`
- 未录入成绩时使用 `-1` 表示
- 按平均成绩对学生排序
- 按课程编号筛选选修该课程的学生

### 图形界面

模块提供 EasyX 图形界面，支持：

- 管理员学生管理页面
- 管理员课程管理页面
- 学生个人课程和成绩查看页面
- 鼠标点击操作、输入框和提示框
- 从图书或校园主 GUI 返回时恢复工作目录

## 菜单

### 控制台菜单

```text
1. 查看所有学生
2. 查询学生
3. 新增学生
4. 修改学生
5. 删除学生
6. 按平均成绩排名
7. 按课程筛选学生
8. 课程管理
0. 退出
```

课程管理子菜单：

```text
1. 查看学生课程
2. 添加课程
3. 删除课程
4. 录入/修改成绩
0. 返回主菜单
```

修改学生时，直接回车表示保留原值。

### GUI 操作

管理员页面提供：

```text
获取所有学生信息
添加学生
删除学生
更新学生信息
查找学生
按课程查询学生
成绩排名
管理学生课程
```

学生账号登录后，可以查看所选课程和课程成绩。

## 工程结构

```text
student_manage_system/
├── include/
│   ├── manager.h
│   ├── student.h
│   ├── student_manager.h
│   ├── subject.h
│   └── subject_manager.h
├── src/
│   ├── main.cpp
│   ├── fileoperate.cpp
│   ├── student.cpp
│   ├── student_manager.cpp
│   ├── subject.cpp
│   └── subject_manager.cpp
├── data/
│   ├── student_data/
│   │   └── student_data.csv
│   └── subject_data/
│       └── <学号>.csv
└── README.md

gui_student_system/
├── include/
│   ├── course_manager_gui.h
│   ├── student_gui.h
│   └── student_manager_gui.h
└── src/
    ├── course_manager_gui.cpp
    ├── gui_test.cpp
    ├── student_gui.cpp
    └── student_manager_gui.cpp
```

公共模块位于仓库根目录的 `include/` 和 `src/`，包括 `User`、`UserManager` 等用户账号类。

## 核心类

| 类 | 职责 |
|---|---|
| `User` | 保存姓名、邮箱、用户 ID、密码和性别 |
| `Student` | 在 `User` 基础上保存专业、学号、班级及课程列表 |
| `subject` | 保存课程名称、课程 ID、教师姓名和成绩 |
| `Manager` | 定义数据加载、保存、新增、删除、修改和查找接口 |
| `StudentManager` | 管理学生记录及其用户账号同步 |
| `SubjectManager` | 管理课程文件、课程筛选、成绩排名和学生课程同步 |

类关系：

```text
User
└── Student
      └── subject（一个学生可拥有多门课程）
Manager
├── StudentManager
└── SubjectManager
```

## 数据文件

程序相对运行目录读取和保存以下文件。

### 学生数据

文件：`data/student_data/student_data.csv`

| 字段 | 含义 |
|---|---|
| 用户ID | 登录账号和统一用户编号 |
| 用户名 | 学生姓名 |
| 性别 | 男、女或空值 |
| 邮箱 | 用户邮箱 |
| 密码 | 登录密码 |
| 专业 | 学生专业 |
| 学号 | 学生唯一编号，同时作为课程文件名 |
| 班级 | 班级名称 |

文件示例：

```csv
用户ID,用户名,性别,邮箱,密码,专业,学号,班级
U0001,张伟,男,user0001@example.com,123456,计算机,20260001,1班
```

### 课程数据

文件：`data/subject_data/<学号>.csv`

例如 `20260001.csv` 对应学号为 `20260001` 的学生。

| 字段 | 含义 |
|---|---|
| 课程名称 | 课程名称 |
| 课程ID | 同一学生下的课程唯一编号 |
| 教师姓名 | 任课教师 |
| 课程成绩 | `0` 到 `100`；未录入时为 `-1` |

文件示例：

```csv
课程名称,课程ID,教师姓名,课程成绩
大学英语,C002,李老师,74
数据结构,C003,张老师,98
操作系统,C005,刘老师,99
```

学生和课程数据使用 UTF-8 CSV 格式，并支持包含逗号或双引号的字段转义。

## 编译和运行

### 控制台程序

运行目录应位于仓库根目录或 `student_manage_system/`。

Windows 下可在兼容 C++17 的编译器中编译：

- `student_manage_system/src/main.cpp`
- `student_manage_system/src/student.cpp`
- `student_manage_system/src/student_manager.cpp`
- `student_manage_system/src/subject.cpp`
- `student_manage_system/src/subject_manager.cpp`
- `student_manage_system/src/fileoperate.cpp`
- 根目录 `src/user.cpp`
- 根目录 `src/user_manager.cpp`

包含以下头文件目录：

```text
include/
student_manage_system/include/
```

运行前应确保工作目录中存在：

```text
data/student_data/student_data.csv
data/subject_data/
```

### GUI 程序

GUI 使用 EasyX，编译时需要：

- 在项目中包含并链接 EasyX 图形库
- 包含 `gui_student_system/include/`
- 包含 `student_manage_system/include/`
- 包含仓库根目录 `include/`
- 链接学生、课程、用户、文件解析和 GUI 对应的源文件

管理器和学生个人 GUI 启动时会尝试从仓库根目录、学生子系统目录或图书模块启动时的相邻目录中定位学生数据，并在退出后恢复原工作目录。

## 数据修改约束

- 学号不允许重复。
- 新增或修改时，学生记录和 `data/user.csv` 中的用户账号保持同步。
- 删除学生时会删除对应课程文件。
- 同一学生下课程编号不允许重复。
- 成绩必须能够解析为浮点数且位于 `0` 到 `100`。
- CSV 文件缺失或格式不正确时，对应记录会被跳过或操作失败并给出提示。
- 文件保存失败不会向用户报告成功。
