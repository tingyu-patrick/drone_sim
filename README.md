# drone_sim — 無人機飛控模擬器

純軟體的 C++ 無人機飛控模擬器練習專案。目標是練 C++ 與物理/控制實作能力，不追求成品完整度，不買實體無人機。整個學習路徑拆成 [挑戰之路](docs/drone-challenge-ladder.md)，一次解一關，通過驗收才前進，每關結束 commit 一次。

## 環境需求

- CMake 3.16+
- 支援 C++17 的編譯器（MinGW-w64 / MSVC / GCC / Clang 皆可）
- Python 3 + `matplotlib`（第 0.3 關開始要畫圖：`pip install matplotlib`）

## 建置與執行

```bash
cmake -B build
cmake --build build
./build/drone_sim        # Linux / macOS
.\build\drone_sim.exe    # Windows
```

跑測試：

```bash
ctest --test-dir build --output-on-failure
```

### Windows 常見問題

如果 `cmake -B build` 選錯 generator 或抓錯編譯器（PATH 上如果同時存在多套 MinGW，很容易抓到不對的版本），可以明確指定：

```powershell
cmake -B build -G "MinGW Makefiles" -DCMAKE_C_COMPILER="<gcc.exe 完整路徑>" -DCMAKE_CXX_COMPILER="<g++.exe 完整路徑>"
```

改了 generator 之後如果出現「跟之前不一樣」的錯誤，把 `build/` 整個刪掉重新 configure（它是純產物，砍掉重建永遠安全）：

```powershell
Remove-Item -Recurse -Force build
```

路徑務必放在**純英數、不含中文字**的資料夾下，MinGW 工具鏈對非 ASCII 路徑不夠健壯，之後裝其他第三方套件時容易在這裡踩雷。詳細踩雷過程見 `docs/blog-*.md`。

## 專案結構

```
drone_sim/
├── CMakeLists.txt
├── .gitignore
├── src/
│   ├── main.cpp          # 目前：CsvLogger 示範（sin 波假資料）
│   └── csv_logger.hpp    # RAII 教材：開檔/寫入/關檔
├── tests/
│   └── test_basic.cpp    # Catch2 單元測試
├── tools/
│   └── plot_csv.py       # 讀 CSV 畫曲線
└── docs/
    └── cpp-oop-from-scratch.md   # C++ OOP 完整教材
```

> 資料夾名稱如果跟你本機實際使用的不同（例如 `tests/` vs `test/`），記得同步調整 `CMakeLists.txt` 裡 `add_executable(drone_tests ...)` 指向的路徑。

## 使用的技術

- **CMake**：build 系統，`FetchContent` 拉取第三方套件
- **Catch2 v3.15.1**：C++ 單元測試框架
- **C++17**：`CMAKE_CXX_EXTENSIONS OFF`，嚴格標準模式，不依賴編譯器擴充語法
- **matplotlib**：CSV 資料視覺化

---

## 進度追蹤

### 第 0 章：軍火庫（環境與基礎建設） — 3/3 ✅

- [x] 0.1 Hello, CMake
- [x] 0.2 第一個測試
- [x] 0.3 黑盒子（CSV Logger + 畫圖）

### 第 1 章：讓東西掉下來（2D 剛體動力學） — 1/7

- [x] 1.1 打造 Vec2
- [ ] 1.2 自由落體
- [ ] 1.3 誤差顯微鏡
- [ ] 1.4 RK4 積分器
- [ ] 1.5 會轉的箱子
- [ ] 1.6 裝上馬達
- [ ] 1.7 見證失控

### 第 2 章：把它穩住（2D 控制） — 0/6

- [ ] 2.1 PID 積木
- [ ] 2.2 定高
- [ ] 2.3 定姿態
- [ ] 2.4 串級——飛到那裡
- [ ] 2.5 撞牆——飽和與 anti-windup
- [ ] 2.6 小考——航點巡航

### 第 3 章：看得見（視覺化） — 0/4

- [ ] 3.1 開窗
- [ ] 3.2 心跳分離——Fix Your Timestep
- [ ] 3.3 讓它出現
- [ ] 3.4 接上方向盤

### 第 4 章：進入三維（大魔王章） — 0/7

- [ ] 4.0 立法——慣例筆記
- [ ] 4.1 Vec3 與 Mat3
- [ ] 4.2 Quaternion
- [ ] 4.3 3D 剛體
- [ ] 4.4 四顆馬達
- [ ] 4.5 3D 穩定
- [ ] 4.6 3D 航點巡航

### 第 5 章：真實世界的噪音（選做進階） — 0/4

- [ ] 5.1 弄髒感測器
- [ ] 5.2 Complementary Filter
- [ ] 5.3 閉上眼睛飛
- [ ] 5.4 Kalman Filter

### 最終 Boss — 0/1

- [ ] 3D 無人機、Crazyflie 真實參數、感測器噪音、只靠估測姿態完成完整航點任務

---

**總進度：4 / 31（不含最終 Boss）**

> 這個表格是手動維護的 checklist——每完成一關，把對應的 `[ ]` 改成 `[x]`，GitHub 上會自動渲染成打勾的核取方塊。完整的挑戰內容、驗收條件、提示，請看 [`docs/drone-challenge-ladder.md`](docs/drone-challenge-ladder.md)。
