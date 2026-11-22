# 校园综合管理系统

基于 C++17 与 EasyX 开发的校园管理项目，整合学生管理、图书管理和银行账户管理。

项目当前入口是 EasyX 图形界面：登录后根据用户 ID 前缀识别角色，再进入对应模块。

## 功能概览

| 模块 | 管理员 / 角色 | 普通学生 |
|---|---|---|
| 学生管理 | 查看、新增、修改、删除学生；课程管理；成绩排名；按课程筛选 | 查看本人课程和成绩 |
| 图书管理 | 新增、修改、调整库存和上下架图书；查看借阅记录 | 查询、借阅、归还和查看本人借阅记录 |
| 银行管理 | 储蓄/信用账户开户、查看账户和流水 | 存款、取款、转账、查看本人账户和流水 |

## 角色与登录

统一用户数据保存在 `data/user.csv`。

登录账号按用户 ID 前缀区分角色：

| 前缀 | 角色 |
|---|---|
| `U` | 学生 |
| `S` | 学生管理员 |
| `L` | 图书管理员 |
| `B` | 银行管理员 |

当前实现直接比较 CSV 中保存的账号和密码，不使用密码哈希。

## 主界面

主 EasyX 程序位于：

```text
gui_library_system/gui_main.cpp
```

主界面提供：

- 登录页面
- 学生、图书、银行三张功能卡片
- 根据角色控制子系统入口
- 图书查询、借还和管理员维护页面

部分模块提供独立入口或控制台测试入口：

- 学生管理 GUI：`gui_student_system/src/gui_test.cpp`
- 银行控制台：`bank_manage_system/src/main.cpp`
- 图书控制台：`library_manage_system/main.cpp`

## 子系统

### 学生管理

详细说明见 [学生管理 README](student_manage_system/README.md)。

主要功能：

- 学生档案的增删改查
- 学号唯一性检查
- 学生信息与 `data/user.csv` 同步
- 查看、添加、删除课程
- 录入或修改成绩
- 按平均成绩排名
- 按课程编号筛选学生

数据位置：

```text
student_manage_system/data/student_data/student_data.csv
student_manage_system/data/subject_data/<学号>.csv
```

成绩范围为 `0` 到 `100`；未录入成绩使用 `-1`。

### 图书管理

主要目录：

```text
library_manage_system/
├── include/
├── src/
├── main.cpp
└── data/library.txt
```

主要功能：

- 图书查询、新增、修改、调整库存和上下架
- 借书和还书
- 最多借阅未归还数量为 `5`
- 借阅期限为 `30` 天
- 检查库存、重复借阅、重复归还和未归还册数
- 保存图书和借阅记录到 `library_manage_system/data/library.txt`

图书文件以 `|` 分隔的文本格式保存。书目行和借阅行分别以 `BOOK` 或 `LOAN` 开头。

GUI 中普通用户和管理员共用图书页面，管理员会额外看到图书维护按钮。

### 银行管理

主要目录：

```text
bank_manage_system/
├── include/
├── src/
└── build_bank.cmd
```

GUI 银行入口位于 `gui_bank_system/gui_bank.cpp`。

主要功能：

- 储蓄账户和信用账户
- 储蓄、取款、转账
- 查询余额
- 管理员开户并查看账户
- 学生只能操作本人账户
- 金额使用整数“分”，避免浮点舍入

银行数据默认位于：

```text
bank_manage_system/data/accounts.txt
```

账户文件示例（金额单位为分）：

```text
SAVING SU001 U001 1250
CREDIT CU002 U002 -500 100000
```

两个文件分别加载和保存。账户操作只修改内存，保存成功后再更新界面状态。

## 工程结构

```text
CampusSystem/
├── README.md
├── data/
│   └── user.csv
├── include/
│   ├── user.h
│   └── user_manager.h
├── src/
│   ├── user.cpp
│   └── user_manager.cpp
├── gui_library_system/
│   └── gui_main.cpp
├── gui_student_system/
│   ├── include/
│   └── src/
├── gui_bank_system/
│   └── gui_bank.cpp
├── student_manage_system/
│   ├── include/
│   ├── src/
│   └── data/
├── library_manage_system/
│   ├── include/
│   ├── src/
│   ├── main.cpp
│   └── data/
└── bank_manage_system/
    ├── include/
    ├── src/
    ├── data/
    └── build_bank.cmd
```

## 编译环境

当前仓库没有统一 CMakeLists.txt 或可直接使用的解决方案文件。主要代码使用：

- C++17
- Windows API
- EasyX 图形库
- Visual Studio C++ 工具链

图形入口和各模块需要在 Visual Studio 工程中手动加入相应源文件、头文件目录和 EasyX 配置。

银行控制台可使用：

```bat
bank_manage_system\build_bank.cmd
```

构建输出位于：

```text
bank_manage_system\build\main.exe
```

此脚本只用于银行控制台模块，不构建整个校园综合 GUI。

## 运行工作目录

相对路径依赖启动时的工作目录。统一 GUI 通常应在仓库根目录或包含各子系统目录的目录下启动。

程序使用的主要相对路径：

```text
data/user.csv
../data/user.csv
data/library.txt
../library_manage_system/data/library.txt
../student_manage_system/data/student_data/student_data.csv
../bank_manage_system/data/
```

学生和图书 GUI 会尝试切换到可找到数据的目录，并在退出子页面后恢复原工作目录。

运行前应确认：

- `data/user.csv` 存在且格式正确
- 图书数据文件存在
- 学生和课程 CSV 数据存在
- 银行账户文件位于 `bank_manage_system/data/accounts.txt`；文件不存在时会以空账户数据启动

## 数据一致性与错误处理

- 学生档案修改后同步更新用户账号文件
- 删除学生时删除对应课程文件
- 同一学生下课程编号不能重复
- 图书编号不能重复
- 借书检查未归还数量、图书库存和学生借阅上限
- 银行账户加载时会校验格式和重复账号
- 银行金额按整数分保存，拒绝负数、零、非两位以内小数和溢出
- 文件加载或保存失败时应提示失败；不能静默覆盖原数据

当前没有数据库、网络服务或密码哈希机制，适合课程项目和本地演示。

## 手动验证建议

1. 使用学生、学生管理员、图书管理员或银行管理员账号登录。
2. 验证各角色只能进入允许的模块。
3. 在学生模块新增、修改、删除学生，并重启后确认数据保留。
4. 为学生添加课程，录入成绩，再查询课程和成绩排名。
5. 借书、归还和重复归还，确认库存与借阅记录一致。
6. 开户、存款、取款和转账后重启，确认账户和流水恢复。
7. 测试缺少数据文件、重复编号、非法成绩和非法金额等失败场景。
