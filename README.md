# Resume Builder

完全离线的专业简历制作软件，使用 C++17 + ImGui 构建，支持中文字体、实时预览和 PDF 导出。

## 功能

- **实时预览**：左侧编辑、右侧实时显示简历效果
- **PDF 导出**：A4 尺寸，支持中文字体嵌入
- **证件照**：支持添加照片，自动排版到简历右上角
- **多模板**：经典、技术岗、实习、社招 4 种模板
- **颜色自定义**：可调整文字和边框颜色
- **撤销/重做**：Ctrl+Z / Ctrl+Y
- **自动保存**：每 30 秒自动保存
- **完全离线**：无需网络，无数据上传

## 编辑模块

- 个人信息（姓名、电话、邮箱、求职意向、证件照）
- 教育经历（学校、专业、学历、GPA、主修课程）
- 工作/实习经历（公司、职位、工作内容）
- 项目经历（项目描述、技术栈、个人职责）
- 技能特长（分类管理，熟练/掌握/熟悉/了解 四级）
- 证书奖项

## 构建

### 依赖

- CMake 3.20+
- 支持 C++17 的编译器（MSVC 2022 / GCC 10+ / Clang 13+）

### Windows

```bash
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

可执行文件在 `build/Release/resume_builder.exe`

### macOS

```bash
cmake -B build -G Xcode
cmake --build build --config Release
```

生成的 `.app` 在 `build/Release/` 目录下。

### Linux

```bash
cmake -B build
cmake --build build --config Release
```

需要安装：`libgl1-mesa-dev libx11-dev libxext-dev libxi-dev`

## 项目结构

```
resume-builder/
├── CMakeLists.txt
├── cmake/                  # CMake 配置
├── fonts/                  # 中文字体（思源黑体）
├── resources/              # 字体子集工具
├── scripts/                # 各平台构建脚本
├── src/
│   ├── main.cpp            # 入口
│   ├── app.cpp/h           # 应用状态管理
│   ├── ui/                 # ImGui 界面
│   │   ├── ui_editor.cpp   # 编辑面板
│   │   ├── ui_preview.cpp  # 预览面板
│   │   └── modules/        # 6 个编辑模块
│   ├── data/               # 数据模型与序列化
│   ├── template/           # 模板引擎
│   ├── export/             # PDF 导出
│   └── platform/           # 平台适配
└── third_party/            # 第三方库
    ├── imgui/              # Dear ImGui
    ├── libharu/            # PDF 生成库
    ├── zlib/               # 压缩库
    ├── nlohmann/           # JSON 库
    └── stb_image.h         # 图片加载
```

## 技术栈

| 组件 | 用途 |
|------|------|
| Dear ImGui | 即时模式 GUI 框架 |
| libharu | PDF 生成 |
| nlohmann/json | JSON 序列化 |
| stb_image | 图片加载 |
| zlib | 压缩（libharu 依赖） |

## 许可证

本项目代码为自用项目。第三方库各自遵循其开源许可证。
