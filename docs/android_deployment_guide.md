# Android 系统运行方案

本文档指导如何将 `AttendanceSystem` 移植到 Android 平台运行。

---

## 1. 前提概述

本项目基于 Qt 6 + CMake 构建，理论上可以移植到 Android。但由于以下限制，需要做相应的调整：

| 组件 | Windows 现状 | Android 方案 |
|------|-------------|-------------|
| 数据库 | MySQL / SQLite 双引擎 | **仅 SQLite**（MySQL 无法在 Android 上直接运行） |
| 人脸引擎 | SeetaFace6 (Windows DLL) | 需交叉编译 ARM 版本或使用替代方案 |
| 摄像头 | QCamera (Qt Multimedia) | QCamera → Android Camera2 API（Qt 自动适配） |
| UI | 桌面窗口布局 | 需调整为响应式/移动端布局 |
| Charts | Qt Charts | 可用 |

---

## 2. 开发环境准备

### 2.1 安装必需组件

**方式一：Qt Maintenance Tool（推荐）**

打开 Qt 安装目录下的 `MaintenanceTool.exe`，添加以下组件：

- Qt 6.11.1 → **Android** 勾选：
  - `android_arm64_v8a`（64位 ARM，主流设备）
  - `android_armeabi_v7a`（32位 ARM，老设备，可选）
  - `android_x86_64`（模拟器用，可选）

**方式二：使用 Qt Online Installer 全新安装**

```
选择 Custom Installation → Qt → 6.11.1 → 勾选 Android 组件
```

### 2.2 安装 Android SDK / NDK

Qt 6.11 对 Android 的版本要求如下：

| 组件 | 最低版本 | 推荐版本 |
|------|---------|---------|
| JDK | 17 | OpenJDK 17 |
| Android SDK (API Level) | 31 (Android 12) | 34 (Android 14) |
| Android NDK | 26.x | 26.1.10909125 |
| Gradle | 8.x | 与 Qt 自动生成 |

**安装步骤：**

1. 下载 [Android Studio](https://developer.android.com/studio) 并安装
2. 打开 Android Studio → SDK Manager → SDK Tools，安装：
   - **NDK (Side by side)** — 版本 26.1.10909125
   - **CMake** — 3.22.1+
   - **Android SDK Build-Tools** — 34.x
   - **Android SDK Platform-Tools**
   - **Android SDK Platform 34**

3. 设置环境变量（Windows PowerShell `$PROFILE`）：

```powershell
$env:ANDROID_HOME = "$env:LOCALAPPDATA\Android\Sdk"
$env:ANDROID_SDK_ROOT = "$env:LOCALAPPDATA\Android\Sdk"
$env:ANDROID_NDK_ROOT = "$env:LOCALAPPDATA\Android\Sdk\ndk\26.1.10909125"
$env:JAVA_HOME = "C:\Program Files\Android\Android Studio\jbr"
```

### 2.3 验证安装

```powershell
# 检查 Qt for Android 是否安装
Get-ChildItem "D:/Qt/6.11.1/android_arm64_v8a"

# 检查 NDK
& "$env:ANDROID_NDK_ROOT\ndk-build.cmd" --version

# 检查 ADB
& "$env:ANDROID_SDK_ROOT\platform-tools\adb.exe" --version
```

---

## 3. 代码改造

### 3.1 数据库：强制只用 SQLite

修改 `DatabaseManager::initialize()` 或调用处，在 Android 上跳过 MySQL 探测：

```cpp
// DatabaseManager.cpp 的 initialize() 方法中，在 isOpen 检查后增加：
#ifdef Q_OS_ANDROID
    // Android only supports SQLite
    requestedDriver = "QSQLITE";
#endif
```

或者直接在调用处检测平台：

```cpp
// 在 StatisticsWidget、FaceAttendanceWidget 中调用 initialize 时：
#ifdef Q_OS_ANDROID
    if (!db.initialize("QSQLITE")) { ... }
#else
    if (!db.initialize()) { ... }
#endif
```

**推荐方案**：在项目根目录的 `CMakeLists.txt` 中添加平台宏定义，统一处理：

```cmake
if(ANDROID)
    target_compile_definitions(${PROJECT_NAME} PRIVATE FORCE_SQLITE)
endif()
```

然后在 `DatabaseManager::initialize()` 中检测该宏：

```cpp
#ifdef FORCE_SQLITE
    effectiveDriver = "QSQLITE";
#endif
```

### 3.2 人脸引擎：两种方案

#### 方案 A：占位模式（快速可用）

不设置 `SEETA_FACE_DIR` 编译 Android 版本，`FaceEngine` 自动使用占位模式。人脸特征为随机值，录入和识别在逻辑上可走通，但不具备真实识别能力。适合功能演示和 UI 测试。

```powershell
cmake -S . -B build-android `
    -DCMAKE_PREFIX_PATH="D:/Qt/6.11.1/android_arm64_v8a" `
    -DANDROID_SDK_ROOT="$env:ANDROID_SDK_ROOT" `
    -DANDROID_NDK_ROOT="$env:ANDROID_NDK_ROOT" `
    -DCMAKE_TOOLCHAIN_FILE="$env:ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" `
    -DANDROID_ABI=arm64-v8a `
    -DANDROID_PLATFORM=android-34
# 不传 -DSEETA_FACE_DIR → 占位模式
```

#### 方案 B：交叉编译 SeetaFace6 for Android ARM64（完整识别）

**Step 1：获取 SeetaFace6 源码**

```bash
git clone https://github.com/seetafaceengine/SeetaFace6.git
cd SeetaFace6
```

**Step 2：交叉编译（使用 NDK 工具链）**

```powershell
$NDK_ROOT = "$env:ANDROID_NDK_ROOT"
$TOOLCHAIN = "$NDK_ROOT/build/cmake/android.toolchain.cmake"

# 为每个模块单独编译（FaceDetector, FaceLandmarker, FaceRecognizer）
# 以 FaceRecognizer 为例：

cmake -S FaceRecognizer -B build-fr-android `
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN" `
    -DANDROID_ABI=arm64-v8a `
    -DANDROID_PLATFORM=android-34 `
    -DCMAKE_BUILD_TYPE=Release `
    -DCMAKE_INSTALL_PREFIX="./install/android_arm64"

cmake --build build-fr-android --parallel
cmake --install build-fr-android
```

**Step 3：组织 SDK 目录**

```
sf6.0_android/
├── face_detector.csta
├── face_landmarker_pts5.csta
├── face_recognizer.csta
├── include/seeta/  (同 Windows)
└── lib/arm64-v8a/
    ├── libSeetaFaceDetector600.so
    ├── libSeetaFaceLandmarker600.so
    └── libSeetaFaceRecognizer610.so
```

然后在 CMake 配置时传入：

```powershell
-DSEETA_FACE_DIR="sf6.0_android"
```

**⚠️ 注意**：SeetaFace6 部分模块使用了 x86 汇编优化，交叉编译 ARM 版本需要确保代码路径不依赖 SSE/AVX 等 x86 指令集。官方仓库有部分 Android 构建支持，具体可查看 [SeetaFace6 仓库](https://github.com/seetafaceengine/SeetaFace6) 的 `android` 分支或 Issue 讨论。

### 3.3 UI 适配：响应式布局

桌面端 `MainWindow` 使用固定尺寸 `resize(1080, 720)` 和 `navList->setMaximumWidth(180)`。Android 端建议：

**方案 1：QML 重写前端（推荐）**

使用 Qt Quick (QML) 重写 UI 层，C++ 后端逻辑（数据库、人脸引擎）保持不变：

```
src/
├── main.cpp              ← 桌面版入口
├── main_android.cpp      ← Android 版入口
├── qml/                  ← QML 界面
│   ├── MainView.qml
│   ├── StudentPage.qml
│   ├── AttendancePage.qml
│   └── ...
├── backend/              ← C++ 后端（现有代码复用）
│   └── ...
```

**方案 2：Widget 适配（改动较小）**

保持现有 Widget 布局，添加平台条件编译：

```cpp
void MainWindow::setupUi()
{
    // ...
#ifdef Q_OS_ANDROID
    // 手机竖屏，缩小导航栏
    navList->setMaximumWidth(120);
    resize(360, 640);  // 适应手机屏幕
    // 表格字体调小
    QFont smallFont = font();
    smallFont.setPointSize(10);
    setFont(smallFont);
#else
    navList->setMaximumWidth(180);
    resize(1080, 720);
#endif
}
```

### 3.4 Android 特有配置

**AndroidManifest.xml 权限**

Qt 生成的 AndroidManifest 模板在 `build-android/android-build/AndroidManifest.xml`。需要在 CMakeLists.txt 中声明所需权限：

```cmake
if(ANDROID)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        QT_ANDROID_PACKAGE_NAME "com.example.attendancesystem"
        QT_ANDROID_VERSION_CODE 1
        QT_ANDROID_VERSION_NAME "1.0.0"
        QT_ANDROID_MIN_SDK_VERSION 31
        QT_ANDROID_TARGET_SDK_VERSION 34
    )

    # 权限声明（自动写入 AndroidManifest）
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::CorePrivate  # 用于请求运行时权限
    )
endif()
```

关键权限（在 `AndroidManifest.xml` 中）：

```xml
<uses-permission android:name="android.permission.CAMERA" />
<uses-permission android:name="android.permission.WRITE_EXTERNAL_STORAGE" />
<uses-permission android:name="android.permission.READ_EXTERNAL_STORAGE" />

<uses-feature android:name="android.hardware.camera" android:required="true" />
<uses-feature android:name="android.hardware.camera.autofocus" />
```

**运行时权限请求（Android 6.0+）**

Android 上摄像头和存储需要运行时权限。Qt 6 提供了 `QtAndroidPrivate` API：

```cpp
#ifdef Q_OS_ANDROID
#include <QtCore/private/qandroidextras_p.h>

void requestAndroidPermissions()
{
    // 检查摄像头权限
    auto cameraResult = QtAndroidPrivate::checkPermission(
        "android.permission.CAMERA").result();
    if (cameraResult == QtAndroidPrivate::Denied) {
        QtAndroidPrivate::requestPermissions(
            {"android.permission.CAMERA"});
    }
}
#endif
```

### 3.5 数据库文件路径

Android 上 SQLite 数据库文件应放在应用的私有目录：

```cpp
#ifdef Q_OS_ANDROID
    // Android 私有目录，无需额外权限
    QString dbPath = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation) + "/attendance_db.sqlite";
#else
    QString dbPath = QCoreApplication::applicationDirPath() + "/attendance_db.sqlite";
#endif
```

---

## 4. 编译和部署

### 4.1 CMake 配置

完整 Android 构建命令（在项目根目录执行）：

```powershell
# 设置环境变量（一次性）
$env:ANDROID_SDK_ROOT = "$env:LOCALAPPDATA\Android\Sdk"
$env:ANDROID_NDK_ROOT = "$env:LOCALAPPDATA\Android\Sdk\ndk\26.1.10909125"
$env:JAVA_HOME = "C:\Program Files\Android\Android Studio\jbr"

# CMake 配置
cmake -S . -B build-android `
    -DCMAKE_PREFIX_PATH="D:/Qt/6.11.1/android_arm64_v8a" `
    -DCMAKE_TOOLCHAIN_FILE="$env:ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" `
    -DANDROID_SDK_ROOT="$env:ANDROID_SDK_ROOT" `
    -DANDROID_NDK_ROOT="$env:ANDROID_NDK_ROOT" `
    -DANDROID_ABI=arm64-v8a `
    -DANDROID_PLATFORM=android-34 `
    -DQT_ANDROID_BUILD_ALL_ABIS=OFF `
    -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build build-android --parallel
```

### 4.2 生成 APK

编译完成后，用 Qt 工具链生成 APK：

```powershell
# 方法一：使用 androiddeployqt（Qt 自带工具）
& "D:\Qt\6.11.1\android_arm64_v8a\bin\androiddeployqt.exe" `
    --input build-android/android-AttendanceSystem-deployment-settings.json `
    --output build-android/android-build `
    --android-platform android-34 `
    --jdk "$env:JAVA_HOME" `
    --gradle

# 方法二：直接在 CMake 中作为构建目标
cmake --build build-android --target apk
```

生成的 APK 位于：

```
build-android/android-build/build/outputs/apk/debug/android-build-debug.apk
```

### 4.3 安装到设备

```powershell
# 通过 ADB 安装
& "$env:ANDROID_SDK_ROOT\platform-tools\adb.exe" install `
    build-android\android-build\build\outputs\apk\debug\android-build-debug.apk

# 启动应用
& "$env:ANDROID_SDK_ROOT\platform-tools\adb.exe" shell am start `
    -n com.example.attendancesystem/.QtActivity
```

### 4.4 查看 Logcat 日志

```powershell
# 过滤应用日志
& "$env:ANDROID_SDK_ROOT\platform-tools\adb.exe" logcat -s "Qt" "AttendanceSystem" "libc"
```

---

## 5. 完整 CMakeLists.txt 修改示例

```cmake
cmake_minimum_required(VERSION 3.16)
project(AttendanceSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Qt 模块
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql Charts Multimedia)

# ======= Android 配置 =======
if(ANDROID)
    # 强制使用 SQLite（Android 无 MySQL）
    target_compile_definitions(${PROJECT_NAME} PRIVATE FORCE_SQLITE)

    # APK 包信息
    set_target_properties(${PROJECT_NAME} PROPERTIES
        QT_ANDROID_PACKAGE_NAME "com.example.attendancesystem"
        QT_ANDROID_VERSION_CODE 1
        QT_ANDROID_VERSION_NAME "1.0.0"
        QT_ANDROID_MIN_SDK_VERSION 31
        QT_ANDROID_TARGET_SDK_VERSION 34
    )

    # 排除 Android 不可用的 Qt 模块
    # Qt6::OpenGLWidgets 在 Android 上可能需要替换为 QML 方案
endif()

# ======= 源文件（与桌面版共用） =======
add_executable(${PROJECT_NAME}
    src/main.cpp
    # ... 同原 CMakeLists.txt ...
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core Qt6::Widgets Qt6::Sql Qt6::Charts Qt6::Multimedia
)

# ======= SeetaFace6（Android 版） =======
if(ANDROID AND SEETA_FACE_DIR)
    # 同 Windows 逻辑，但链接 .so 而非 .dll/.lib
    find_library(SEETA_FD_LIB NAMES SeetaFaceDetector600
        PATHS "${SEETA_FACE_DIR}/lib/${ANDROID_ABI}" NO_DEFAULT_PATH)
    # ... 同理查找其他库 ...
    if(SEETA_FD_LIB AND SEETA_FL_LIB AND SEETA_FR_LIB)
        target_include_directories(${PROJECT_NAME} PRIVATE "${SEETA_FACE_DIR}/include")
        target_link_libraries(${PROJECT_NAME} PRIVATE ${SEETA_FD_LIB} ${SEETA_FL_LIB} ${SEETA_FR_LIB})
        target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_SEETA_FACE6)
    endif()
endif()
```

---

## 6. 快速验证清单

### 第 1 步：编译验证（不传 SEETA_FACE_DIR）

```powershell
cmake -S . -B build-android `
    -DCMAKE_PREFIX_PATH="D:/Qt/6.11.1/android_arm64_v8a" `
    -DCMAKE_TOOLCHAIN_FILE="$env:ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" `
    -DANDROID_ABI=arm64-v8a `
    -DANDROID_PLATFORM=android-34

cmake --build build-android --parallel
```

### 第 2 步：生成 APK 并安装

```powershell
# 生成 APK
cmake --build build-android --target apk
# 安装
adb install build-android/android-build/build/outputs/apk/debug/android-build-debug.apk
```

### 第 3 步：功能验证

- [ ] 应用正常启动，SQLite 数据库自动创建
- [ ] 学生管理：新增/编辑/删除学生
- [ ] 人脸录入：打开摄像头，拍照提取特征（占位模式）
- [ ] 人脸识别：模拟考勤签到
- [ ] 考勤统计：饼图 + 出勤/缺勤明细表

### 第 4 步：人脸识别验证（需交叉编译 SeetaFace6）

- [ ] 将 `sf6.0_android/` 目录放好，传入 `-DSEETA_FACE_DIR`
- [ ] 重新编译
- [ ] 真实人脸录入 + 识别测试

---

## 7. 常见问题

### Q: CMake 报 "Could not find Qt6 for Android"？

将 Qt Android 路径加入 CMAKE_PREFIX_PATH：

```powershell
-DCMAKE_PREFIX_PATH="D:/Qt/6.11.1/android_arm64_v8a"
```

### Q: 编译时报 "OpenGL not found"？

Android 使用 OpenGL ES。确保链接了 `Qt6::Gui` 而非 `Qt6::OpenGL`。本项目使用的 `QtCharts` 和 `QtMultimedia` 在 Android 上会自动使用 OpenGL ES。

### Q: APK 安装后闪退？

用 ADB 查看日志定位问题：

```powershell
adb logcat -s "Qt" "*:E"
```

常见原因：
- 缺少运行时权限（摄像头、存储）
- SQLite 数据库路径不可写
- SeetaFace6 .so 文件未打包进 APK

### Q: 如何将 .so 打包进 APK？

在 CMake 中，被 `target_link_libraries` 链接的 `.so` 库会自动被 `androiddeployqt` 打包。手动添加的 `.so` 需要在 `CMakeLists.txt` 中声明。

### Q: 摄像头预览黑屏？

- 检查是否授予了相机权限
- Android 9+ 不允许明文流量（HTTP），但本地摄像头不受影响
- 确保 `AndroidManifest.xml` 中有 `<uses-permission android:name="android.permission.CAMERA" />`

### Q: 数据库在哪里？

Android 上 SQLite 数据库在应用私有目录：

```
/data/data/com.example.attendancesystem/files/attendance_db.sqlite
```

可通过 ADB 拉取查看：

```powershell
adb pull /data/data/com.example.attendancesystem/files/attendance_db.sqlite .
```

---

## 8. 推荐的移植路线

| 阶段 | 目标 | 工作内容 |
|------|------|---------|
| **Phase 1** | 跑通基础功能 | 源码适配 + 仅 SQLite + 占位模式人脸 → APK 可在手机上运行 |
| **Phase 2** | UI 适配手机 | Widget 响应式调整 或 QML 重写前端 |
| **Phase 3** | 真实人脸识别 | 交叉编译 SeetaFace6 ARM64 → 链接 → 真实考勤识别 |
| **Phase 4** | 发布优化 | 签名 APK、混淆、性能优化、多语言 |

**Phase 1 预计可在 1-2 天内完成**，后续阶段根据实际需求推进。
