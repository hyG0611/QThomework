# 人脸识别集成指南（SeetaFace6）

本文档指导如何将 SeetaFace6 人脸识别引擎完整接入本项目 `AttendanceSystem`。

---

## 1. SeetaFace6 简介

SeetaFace6 是开源的人脸识别引擎，由中科视拓（SeetaTech）发布，支持：

- **人脸检测**（FaceDetector）—— 检测图像中人脸位置
- **人脸关键点定位**（FaceLandmarker）—— 5 点 / 81 点 landmarks
- **人脸识别**（FaceRecognizer）—— 提取 1024 维特征向量 + 相似度计算
- **人脸跟踪**（FaceTracker）—— 视频流中跟踪人脸
- **活体检测**（AntiSpoofing）—— 区分真人和照片/视频攻击
- **质量评估**（QualityAssessor）—— 评估人脸图像质量

本项目至少需要前三个模块。

---

## 2. 模型文件获取

### 2.1 所需模型

| 模型 | 文件名（示例） | 用途 |
|------|---------------|------|
| 人脸检测 | `face_detector.csta` | 检测图像中的人脸位置和数量 |
| 5 点 landmarks | `face_landmarker_pts5.csta` | 获取人脸 5 个关键点（双眼、鼻尖、嘴角） |
| 人脸识别 | `face_recognizer.csta` | 提取 1024 维特征向量（人脸比对的基础） |
| 口罩检测（可选） | `face_landmarker_mask_pts5.csta` | 口罩场景专用 landmarks |
| 活体检测（可选） | `fas_first.csta` / `fas_second.csta` | 静默活体检测 |

### 2.2 获取方式

#### 方式一：官方 GitHub（推荐）

- 源码仓库：https://github.com/seetafaceengine/SeetaFace6
- 模型下载：在 release 页面查找或联系中科视拓获取

#### 方式二：百度网盘

SeetaFace6 的官方模型有时通过网盘分发，请关注官方渠道。

#### 方式三：自行编译

```bash
# 克隆源码
git clone https://github.com/seetafaceengine/SeetaFace6.git
cd SeetaFace6

# 创建编译目录
mkdir build && cd build

# 配置 CMake（Windows 下使用 Visual Studio 生成器）
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build . --config Release --parallel
```

编译产物为 `.lib`（导入库）和 `.dll`（动态库）。

---

## 3. 项目目录结构建议

将 SeetaFace6 SDK 和模型放在以下位置即可被 `FaceEngine::initialize()` 自动发现：

```text
QThomework/
├── sf6.0_windows/           ← 推荐：放在项目根目录或 build 目录
│   ├── face_detector.csta
│   ├── face_landmarker_pts5.csta
│   ├── face_recognizer.csta
│   ├── include/
│   │   └── seeta/
│   │       ├── FaceDetector.h
│   │       ├── FaceLandmarker.h
│   │       ├── FaceRecognizer.h
│   │       ├── CStruct.h        ← 公共结构体（SeetaImageData, SeetaRect 等）
│   │       └── ...
│   └── lib/
│       ├── SeetaFaceDetector600.lib
│       ├── SeetaFaceDetector600.dll
│       ├── SeetaFaceLandmarker600.lib
│       ├── SeetaFaceLandmarker600.dll
│       ├── SeetaFaceRecognizer610.lib
│       └── SeetaFaceRecognizer610.dll
```

`FaceEngine::initialize()` 会按以下顺序搜索模型目录：
1. 传入的 `modelDir` 参数
2. `{exe所在目录}/sf6.0_windows/`
3. `{exe所在目录}/../sf6.0_windows/`
4. `{exe所在目录}/../../sf6.0_windows/`

---

## 4. 编译配置（CMake）

在项目根目录执行 CMake 配置时传入 SDK 路径：

```powershell
cmake -S . -B build `
    -DCMAKE_PREFIX_PATH="D:/Qt/6.11.1/mingw_64" `
    -DSEETA_FACE_DIR="sf6.0_windows"
```

CMake 会自动：
1. 查找 `SeetaFaceDetector600`、`SeetaFaceLandmarker600`、`SeetaFaceRecognizer610` 三个库
2. 添加头文件路径 `-I{SEETA_FACE_DIR}/include`
3. 链接库文件
4. 定义宏 `-DHAVE_SEETA_FACE6`，使 `FaceEngine` 中的真实代码生效

**不设置 `SEETA_FACE_DIR` 也能编译**：此时 FaceEngine 使用占位模式（可用但特征无效），不影响其他功能。

---

## 5. FaceEngine API 设计说明

当前 `FaceEngine` 类位于 `src/face/FaceEngine.h` `.cpp`，提供以下接口：

| 方法 | 说明 |
|------|------|
| `initialize(modelDir)` | 初始化引擎，加载模型文件 |
| `isInitialized()` | 检查是否初始化成功 |
| `detect(image, faceRect)` | 检测人脸，返回人脸矩形 |
| `extractFeature(image)` | 提取人脸特征（1024 个 float） |
| `recognize(image, students, threshold)` | 1:N 识别，返回最佳匹配 |
| `compareFeatures(featureA, featureB)` | 两两特征比对（余弦相似度） |

### 5.1 识别流程（完整）

```
摄像头帧 → detect(人脸矩形) → Landmarker.mark(5点) → Recognizer.Extract(1024维特征)
    ↓
遍历数据库中的学生特征 → compareFeatures() 计算余弦相似度
    ↓
最高分 ≥ 阈值(0.65) → 匹配成功，返回学生信息
```

### 5.2 注册人脸流程

1. 学生管理界面 → 点击"录入人脸"
2. 打开摄像头或选择照片
3. 调用 `detect()` 确认检测到人脸
4. 调用 `extractFeature()` 提取特征
5. 将提取到的 `QByteArray` 写入 `students.face_feature` 和 `face_feature_size`

---

## 6. FaceEngine 真实代码示例

在 `FaceEngine.cpp` 中，所有真实 SeetaFace6 调用都在 `#ifdef HAVE_SEETA_FACE6` 块中。
以下是关键的代码替换示例：

### 6.1 初始化

```cpp
#include <seeta/FaceDetector.h>
#include <seeta/FaceLandmarker.h>
#include <seeta/FaceRecognizer.h>

bool FaceEngine::initialize(const QString &modelDir)
{
    // ... 路径查找代码保持不变 ...

    seeta::ModelSetting detectorSetting(detectorPath.toLocal8Bit().constData());
    seeta::ModelSetting landmarkerSetting(landmarkerPath.toLocal8Bit().constData());
    seeta::ModelSetting recognizerSetting(recognizerPath.toLocal8Bit().constData());

    m_faceDetector = new seeta::FaceDetector(detectorSetting);
    m_faceLandmarker = new seeta::FaceLandmarker(landmarkerSetting);
    m_faceRecognizer = new seeta::FaceRecognizer(recognizerSetting);

    m_initialized = true;
    return true;
}
```

### 6.2 人脸检测

```cpp
bool FaceEngine::detect(const QImage &image, QRect &faceRect)
{
    // QImage → SeetaImageData 转换
    QImage rgb = image.convertToFormat(QImage::Format_RGB888);
    SeetaImageData seetaImage;
    seetaImage.width = rgb.width();
    seetaImage.height = rgb.height();
    seetaImage.channels = 3;
    seetaImage.data = const_cast<unsigned char *>(rgb.bits());

    auto fd = static_cast<seeta::FaceDetector *>(m_faceDetector);
    SeetaFaceInfoArray faces = fd->detect(seetaImage);

    if (faces.size == 0) {
        faceRect = QRect();
        return false;
    }

    // 取最大人脸
    const auto &best = faces.data[0];
    faceRect = QRect(best.pos.x, best.pos.y, best.pos.width, best.pos.height);
    return true;
}
```

### 6.3 特征提取

```cpp
QByteArray FaceEngine::extractFeature(const QImage &image)
{
    QRect faceRect;
    if (!detect(image, faceRect)) {
        m_lastError = "未检测到人脸";
        return {};
    }

    // 转换图像
    QImage rgb = image.convertToFormat(QImage::Format_RGB888);
    SeetaImageData seetaImage;
    seetaImage.width = rgb.width();
    seetaImage.height = rgb.height();
    seetaImage.channels = 3;
    seetaImage.data = const_cast<unsigned char *>(rgb.bits());

    // Landmarks
    SeetaPointF points[5];
    auto fl = static_cast<seeta::FaceLandmarker *>(m_faceLandmarker);
    fl->mark(seetaImage, faces.data[0].pos, points);

    // Extract feature
    float features[1024];
    auto fr = static_cast<seeta::FaceRecognizer *>(m_faceRecognizer);
    fr->Extract(seataImage, points, features);

    return QByteArray(reinterpret_cast<const char *>(features), FeatureByteSize);
}
```

---

## 7. 特征比对算法

本项目使用 **余弦相似度** 进行特征比对：

```cpp
float compareFeatures(const QByteArray &featureA, const QByteArray &featureB)
{
    const float *a = reinterpret_cast<const float *>(featureA.constData());
    const float *b = reinterpret_cast<const float *>(featureB.constData());

    float dot = 0, normA = 0, normB = 0;
    for (int i = 0; i < 1024; ++i) {
        dot += a[i] * b[i];
        normA += a[i] * a[i];
        normB += b[i] * b[i];
    }

    return dot / (std::sqrt(normA) * std::sqrt(normB) + 1e-6f);
}
```

### 阈值建议

| 阈值 | 含义 | 适用场景 |
|------|------|---------|
| < 0.60 | 宽松，可能误识 | 快速录入/测试 |
| 0.60 - 0.70 | 中等 | 一般考勤场景 |
| 0.65 - 0.75 | **推荐** | 正式使用 |
| > 0.75 | 严格，可能漏识 | 高安全场景 |

---

## 8. 运行前检查清单

### 编译前
- [ ] SeetaFace6 SDK 已放置在正确目录
- [ ] 模型文件（.csta）已下载
- [ ] CMake 配置时传入了 `-DSEETA_FACE_DIR=...`

### 运行前
- [ ] SeetaFace6 DLL 已在 PATH 或与 exe 同目录
- [ ] 模型文件在可执行文件能找到的路径
- [ ] `windeployqt` 已完成（Qt 依赖已部署）

### 验证
1. 启动程序，切换到"运行状态"页，点击"测试摄像头"，确认摄像头可用
2. 切换到"人脸识别"页，确认模型加载成功（日志区应有提示）
3. 在学生管理中录入一名学生的人脸特征
4. 在人脸识别中测试识别效果

---

## 9. 常见问题

### Q: 模型加载失败？
- 检查模型文件路径
- 检查模型文件完整性（是否下载完整）
- 查看程序日志输出（运行状态页）

### Q: 找不到 SeetaFace6 DLL？
- 将 `sf6.0_windows/lib/*.dll` 复制到 exe 所在目录
- 或将 `sf6.0_windows/lib/` 加入系统 PATH

### Q: 编译时报错找不到 seeta/FaceDetector.h？
- 确认 SDK 目录结构：`include/seeta/FaceDetector.h`
- 确认 CMake 传入的 `SEETA_FACE_DIR` 路径正确

### Q: 人脸识别率低？
- 调整阈值（`threshold` 参数）
- 检查光照条件
- 确保注册时人脸清晰、正面
- 考虑引入质量评估模块过滤低质量帧

### Q: 性能问题（识别慢）？
- 限制数据库中学生数量（按班级/院系过滤）
- 启动时将人脸特征缓存在内存中
- 使用人脸跟踪：连续帧只检测一次，后续帧使用 tracker 跟踪
- 将识别逻辑放入 `QThread` 避免阻塞 UI
