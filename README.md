# QThomework · Qt 考勤管理系统

这是一个使用 C++17 和 Qt 6 编写的桌面课程项目，提供学生信息管理、课程与成绩管理、考勤记录和统计。接入 SeetaFace6 后，可通过摄像头录入人脸并进行识别签到。

## 在 Windows 上构建

准备 CMake 3.16 或更高版本、支持 C++17 的编译器，以及与编译器匹配的 Qt 6。项目需要 Qt Core、Widgets、Sql、Charts、Multimedia 模块和 SQLite 驱动。下面以 MinGW 和 Ninja 为例，请按本机安装位置设置 Qt 路径。

```powershell
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/mingw_64"
cmake --build build
.\build\AttendanceSystem.exe
```

可以在 Qt Creator 中打开根目录的 `CMakeLists.txt`，选择对应的 Desktop Kit 后运行。如果从终端启动，请确保 Qt 和编译器的运行时 DLL 可用。

## 数据库配置

默认使用 SQLite。首次使用数据库功能时，程序在可执行文件所在目录创建 `attendance_db.sqlite` 并初始化表结构。

如需连接 MySQL，先创建本地 `attendance_system` 数据库。全新数据库可参考 [初始化脚本](sql/init_database.sql)，其中包含删表语句，执行前请确认目标是空数据库。将 `ATTENDANCE_DB_DRIVER` 设为 `QMYSQL`，并在启动进程的环境中设置 `ATTENDANCE_DB_USER` 和 `ATTENDANCE_DB_PASSWORD`。QMYSQL 连接地址为 `127.0.0.1:3306`。使用 ODBC 时，将驱动设为 `QODBC` 并设置 `ATTENDANCE_DB_ODBC_DSN` 为本机数据源名称。

## 可选的人脸识别

配置 CMake 时传入 `-DSEETA_FACE_DIR="C:/path/to/sf6.0_windows"`。准备与编译器匹配的 SeetaFace6 SDK，把运行时 DLL 放在可执行文件旁或加入 PATH。将 `face_detector.csta`、`face_landmarker_pts5.csta` 和 `face_recognizer.csta` 放在可执行文件旁或项目根目录的 `sf6.0_windows/`。查看 [人脸识别接入指南](docs/face_recognition_guide.md)。

## 目录

- `src/ui/` 存放 Qt 界面。
- `src/database/` 和 `src/models/` 存放数据库访问和数据模型。
- `src/face/` 存放人脸识别适配代码。
- `sql/` 存放 MySQL 初始化脚本。
- `docs/` 存放设计、依赖和部署文档。
