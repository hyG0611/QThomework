# 源码与第三方库搜集指南

---

## 1. 必需依赖

### 1.1 Qt 开发框架

| 项目 | 版本建议 | 下载/安装方式 |
|------|----------|---------------|
| Qt 6 | 6.5 LTS 或更高 | https://www.qt.io/download |
| Qt 5 | 5.15 LTS | 如硬件或环境受限可使用 |

**需要安装的模块**：

- Qt Widgets
- Qt SQL（含 MySQL 插件）
- Qt Charts
- Qt Multimedia（摄像头采集）
- Qt Concurrent（可选，用于异步识别）

#### 安装建议

- **Windows**：使用 Qt Online Installer 安装
- **Linux (Ubuntu/Debian)**：

```bash
sudo apt update
sudo apt install qt6-base-dev qt6-charts-dev qt6-multimedia-dev \
                 libqt6sql6-mysql libmysqlclient-dev
```

- **macOS**：`brew install qt`

---

### 1.2 MySQL 数据库

| 项目 | 版本建议 | 下载/安装方式 |
|------|----------|---------------|
| MySQL Server | 8.0+ | https://dev.mysql.com/downloads/mysql/ |
| MySQL Workbench | 最新版 | 图形化管理工具，可选 |

#### 安装建议

- **Windows**：下载 MySQL Installer
- **Linux**：

```bash
sudo apt install mysql-server
sudo mysql_secure_installation
```

- **macOS**：`brew install mysql`

---

### 1.3 OpenCV

用于摄像头图像采集与图像格式转换。

| 项目 | 版本建议 | 下载/安装方式 |
|------|----------|---------------|
| OpenCV | 4.5+ | https://opencv.org/releases/ |

#### 安装建议

- **Windows**：下载官方 release 或自行编译
- **Linux**：

```bash
sudo apt install libopencv-dev
```

- **macOS**：`brew install opencv`

---

### 1.4 SeetaFace6 人脸识别引擎

| 项目 | 用途 | 获取方式 |
|------|------|----------|
| SeetaFace6 源码 | 编译 SDK | https://github.com/seetafaceengine/SeetaFace6 |
| SeetaFace6 模型 | 人脸检测/关键点/识别 | release 页面或官方文档 |

#### 替代方案

| 项目 | 特点 | 获取方式 |
|------|------|----------|
| SeetaFace2 | 更轻量，编译更简单 | https://github.com/seetafaceengine/SeetaFace2 |
| OpenCV DNN + insightface | 需要额外学习成本 | 自行搜索预训练模型 |

---

## 2. 推荐的开源代码/示例

### 2.1 Qt 官方示例

- `qtsql` 示例：学习数据库操作
- `qtmultimedia` 摄像头示例
- `qtcharts` 柱状图示例

### 2.2 SeetaFace 示例

- 官方 example 目录下的 `search`、`register` 示例
- 社区 Qt + SeetaFace 集成博客/项目

### 2.3 CMake 模板

- Qt 官方 CMake 示例项目
- 本项目已提供基础 `CMakeLists.txt`

---

## 3. 开发工具

| 工具 | 用途 | 推荐 |
|------|------|------|
| IDE | 代码编写 | Qt Creator（首选）、CLion、VS Code |
| 数据库客户端 | SQL 调试 | MySQL Workbench、DBeaver、Navicat |
| 版本控制 | 代码管理 | Git |
| 抓包/调试 | 网络问题排查 | Wireshark（可选） |

---

## 4. 依赖清单汇总

| 依赖 | 必须 | 用途 |
|------|------|------|
| Qt Widgets | ✅ | GUI 界面 |
| Qt SQL + MySQL 插件 | ✅ | 数据库访问 |
| Qt Charts | ✅ | 考勤统计柱状图 |
| Qt Multimedia | ✅ | 摄像头预览 |
| OpenCV | ✅ | 摄像头采集与图像处理 |
| SeetaFace6 | ✅ | 人脸识别核心 |
| MySQL Server | ✅ | 数据持久化 |
| Git | 强烈推荐 | 版本控制 |
| Qt Creator | 推荐 | 集成开发环境 |

---

## 5. 各平台快速安装命令参考

### Windows

1. 安装 Qt Online Installer
2. 安装 MySQL Installer（Server + Connector/C）
3. 安装 OpenCV（推荐下载预编译版）
4. 下载 SeetaFace6 源码与模型，使用 Visual Studio 编译

### Ubuntu / Debian

```bash
# 系统依赖
sudo apt update
sudo apt install build-essential cmake git

# Qt
sudo apt install qt6-base-dev qt6-charts-dev qt6-multimedia-dev \
                 libqt6sql6-mysql libmysqlclient-dev

# OpenCV
sudo apt install libopencv-dev

# MySQL
sudo apt install mysql-server mysql-client
```

### macOS

```bash
# 使用 Homebrew
brew install qt mysql opencv cmake git
```

---

## 6. 注意事项

1. **Qt MySQL 插件**：某些 Qt 安装包可能不带 MySQL 插件，需要自行编译 `qsqlmysql` 插件。
2. **OpenCV 与 Qt 冲突**：两者都有 `QImage`/`cv::Mat` 转换需求，注意头文件包含顺序。
3. **SeetaFace 模型版权**：模型文件通常有使用许可，请遵守官方说明。
4. **跨平台路径**：所有资源路径（模型、配置文件）都应使用 Qt 的 `QDir`/`QStandardPaths` 处理。
