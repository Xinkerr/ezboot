# 脚本工具说明

## 1. **module_install.bat**

该脚本用于在 Windows 环境下安装 `ota_generate.py` 所需的 Python 依赖模块。

## 2. **ota_generate.py**

该脚本用于生成 OTA 文件，执行时需提供两个输入参数：

- `ezboot_config.h` 文件的路径
- App 的 `.hex` 文件路径

### 使用方法

在 App 的 Keil 工程中，打开 `Options for Target` -> `User` -> `After Build/Rebuild`，添加如下命令：

```bash
python .\ota_generate.py .\ezboot\config $L@L.hex
```

填写命令后，勾选 **Run** 选项以启用该脚本。

