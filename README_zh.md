# PCBA-MC001A1V01
PCBA-MC001A1V0x Development Board Demo Project

[中文](README_zh.md) | [English](README.md)

### VS Code + ESP-IDF

1. **安装 ESP-IDF 擴展**
   - 在 VS Code 的 Extensions 中搜尋並安裝 "ESP-IDF" 擴展
   - Prerequisites: https://docs.espressif.com/projects/idf-im-ui/en/latest/prerequisites.html
   - ESP-IDF Extension for VS Code:https://github.com/espressif/vscode-esp-idf-extension/blob/master/README.md

2. **配置 ESP-IDF 環境**
   - 按 `Ctrl+Shift+P` 開啟命令面板
   - 輸入 "ESP-IDF: Open ESP-IDF Installation Manager"
   - 選擇 New Installation
   - 選擇 Custom Installation
   - 目標設備: All(須包含 esp32s3)
   - 選擇 ESP-IDF 版本：**5.5.3**
   - 設定 ESP-IDF 安裝路徑
   - ESP-IDF: https://docs.espressif.com/projects/vscode-esp-idf-extension/en/latest/installation.html

3. **開啟專案**
   - 使用 "File > Open Folder" 開啟此專案資料夾
   - VS Code 會自動識別為 ESP-IDF 專案

### 使用版本

- **ESP-IDF**: 5.5.3
- **LVGL**: 9.1.0

### UI 修改說明

如果您想要修改使用者介面，只需要編輯 `components/display/display.c` 檔案中的 `ui_init()` 函數即可。

#### 修改步驟：

1. 打開 `components/display/display.c` 檔案
2. 找到 `display_init()` 函數中的以下程式碼：
   ```c
   // ui_demo();
   ui_init();
   ```
3. 您可以選擇：
   - **使用預設 demo UI**：取消註解 `ui_demo()` 並註解掉 `ui_init()`
     ```c
     ui_demo();
     // ui_init();
     ```
   - **使用自訂 UI**：保持 `ui_init()` 並在 `main/ui/ui.c` 中定義您的介面
   - **建立新的 UI 函數**：參考 `ui_demo()` 的實作方式建立您自己的 UI 函數

#### UI 函數說明：

- `ui_demo()`：提供簡單的示範介面，包含標籤和按鈕
- `ui_init()`：載入由 SquareLine Studio 生成的完整 UI 介面

#### 注意事項：

- 修改 UI 時請確保在 `lvgl_port_lock(0)` 和 `lvgl_port_unlock()` 之間進行
- 如需更複雜的 UI 設計，建議使用 SquareLine Studio 工具來設計介面

----------------------------------------------------------

### 使用 SquareLine Studio 

SquareLine Studio 是一個專業的 UI 設計工具，可以讓您透過拖拉方式設計使用者介面。

#### 安裝 SquareLine Studio

1. 前往 [SquareLine Studio 官網](https://squareline.io/) 下載並安裝
2. 註冊帳號並啟動軟體

#### 建立新專案

1. **開啟 SquareLine Studio**
2. **建立新專案**：
   - 選擇 "Create New Project"
   - 選擇 LVGL version 9.1
   - 選擇 "Espressif" 作為目標平台
   - 選擇 "ESP-BOX"
   - 設定專案名稱和路徑
3. **配置顯示設定**：
   - 解析度：800 x 480 (根據您的顯示器規格)
   - 色彩深度：16 bit
   - LVGL 版本：9.1.0

#### 設計 UI 介面

1. **新增元件**：
   - 從左側工具列拖拉元件到畫布上
2. **設定屬性**：
   - 在右側屬性面板調整元件的位置、大小、顏色等
3. **新增事件**：
   - 選擇元件後在 Events 標籤中新增事件處理

#### 匯出程式碼

1. **匯出設定**：
   - 點選 "Export" → "Export UI Files"
   - 選擇匯出路徑為專案的 `main/ui/` 資料夾
2. **匯出檔案**：
   - `ui.c` 和 `ui.h`：主要的 UI 程式碼
   - 其他相關的螢幕和元件檔案

#### 整合到專案中

1. **複製檔案**：
   - 將匯出的 `ui.c` 和 `ui.h` 檔案複製到 `main/ui/` 資料夾
2. **呼叫 UI**：
   - 在 `components/display/display.c` 的 display_init() 中呼叫 `ui_init()` 函數

```
project/
├── 📁 components
│   ├── 📁 aw9523
│   ├── 📁 display
│   ├── 📁 espressif__esp_lvgl_port
│   ├── 📁 lvgl__lvgl
│   ├──    .
│   ├──    .
│   └──    .
├── 📁 main
│   ├── 📄 CMakeLists.txt
│   ├── 📄 idf_component.yml
│   ├── 📄 main.c
│   └── 📁 ui  <------------- 替換
│       ├── 📁 components
│       ├── 📁 fonts
│       ├── 📁 screens
│       ├── 📄 CMakeLists.txt
│       ├── 📄 filelist.txt
│       ├── 📄 project.info
│       ├── 📄 ui.c
│       ├── 📄 ui.h
│       ├── 📄 ui_events.h
│       ├── 📄 ui_helpers.c
│       └── 📄 ui_helpers.h
├── 📄 README.md
└── 📄 README_zh.md
```