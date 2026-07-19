# 一綠一紅到底在哪裡：無人機模擬器 Catch2 踩雷實錄

第 0 章第一關「Hello, CMake」搞定之後，第二關是接入 Catch2 測試框架，驗收條件很簡單：跑 `ctest` 或直接跑測試執行檔，能看到一個測試過、一個測試沒過，把沒過的修好，全部變綠。聽起來只是把 Catch2 接進 `CMakeLists.txt`、寫兩個 `TEST_CASE` 的小事，結果又踩出兩顆雷，而且這次都不是上一篇提過的那些雷，是全新的坑。寫下來繼續累積這系列的踩雷筆記。

## 背景：這次要接的東西

用 CMake 的 `FetchContent` 機制把 Catch2（v3.15.1）的原始碼直接抓下來、跟自己的專案一起編，不用手動下載或另外裝套件管理員：

```cmake
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.15.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(drone_tests test/test.cpp)
target_link_libraries(drone_tests PRIVATE Catch2::Catch2WithMain)
```

測試檔就兩個 `TEST_CASE`，一個一定過、一個故意寫錯，用來驗證整條測試管線真的有在動：

```cpp
#include <catch2/catch_test_macros.hpp>

TEST_CASE("addition works", "[basic]") {
    REQUIRE(1 + 1 == 2);
}

TEST_CASE("this one is intentionally wrong", "[basic]") {
    REQUIRE(1 + 1 == 3);
}
```

看起來平淡無奇，結果從「編不過」到「編過了但測試框架找不到測試」，一路踩了兩顆完全不同性質的雷。

## 地雷一：同一個資料夾裡，`c++.exe` 和 `g++.exe` 根本不是同一個版本

第一次 `cmake -B build` + `cmake --build build`，Catch2 編到一半直接爆炸：

```
error: 'gmtime_s' was not declared in this scope
    gmtime_s(&timeInfo, &rawtime);
```

`gmtime_s` 是 Catch2 內部產生 JUnit 格式報告時用到的函式，比較新的 MinGW-w64 工具鏈才支援。查 configure 的 log 才發現，CMake 這次抓到的編譯器版本是 `GNU 6.3.0`——一套 2016 年左右的老古董，跟我之前手動測試 `g++ --version` 看到的 `13.2.0` 完全不是同一個版本。

最詭異的地方在於：兩個版本都指向同一個資料夾 `C:\MinGW\bin`。用 `where.exe g++.exe` 和 `where.exe c++.exe` 分別查，兩者都只找到 `C:\MinGW\bin` 底下的執行檔，代表電腦上其實只裝了「一套」MinGW——但這個資料夾裡的 `c++.exe` 和 `g++.exe` 卻是不同版本。最合理的解釋是這個資料夾曾經被不同時期的安裝程式覆蓋過（可能很久以前裝過一次舊版，後來又用別的方式升級過，但沒有把所有執行檔都換掉），導致同一個資料夾裡混著新舊版本的執行檔，而 CMake 幫 C++ 找編譯器時預設候選名稱是 `c++`，剛好抓到了那個沒被更新的舊版。

解法是不要讓 CMake 自己在 PATH 裡瞎猜，直接明講要用哪一個檔案：

```powershell
cmake -B build -G "MinGW Makefiles" `
  -DCMAKE_C_COMPILER="C:/MinGW/bin/gcc.exe" `
  -DCMAKE_CXX_COMPILER="C:/MinGW/bin/g++.exe"
```

指名用 `g++.exe`（而不是讓它自動找 `c++.exe`）之後，configure log 顯示 `The CXX compiler identification is GNU 13.2.0`，Catch2 那幾百個原始檔終於順利編完。

這裡學到的事：**同一個資料夾底下的執行檔不保證是同一個版本**——尤其是像 `C:\MinGW` 這種存在很久、可能被多次安裝/升級動過的路徑。遇到「編譯器版本莫名其妙不對」的狀況，與其相信資料夾名稱，不如直接對可疑的執行檔跑一次 `--version`，眼見為憑。

## 地雷二：Build 成功、執行檔也正常，`ctest` 卻死活找不到測試

編譯器問題解決後，`drone_tests.exe` 順利建出來，直接執行完全正常：

```
this one is intentionally wrong
D:\drone_sim\test\test.cpp:8: FAILED:
  REQUIRE( 1 + 1 == 3 )
with expansion:
  2 == 3

test cases: 2 | 1 passed | 1 failed
```

一綠一紅，看起來大功告成。結果一跑 `ctest --test-dir build --output-on-failure`：

```
No tests were found!!!
```

這個反差很讓人困惑：執行檔自己知道自己有兩個測試、也正確跑出結果，`ctest` 卻完全不知道有這回事。這裡的關鍵認知是——**Catch2 本身知道自己有哪些測試（透過 `TEST_CASE` 巨集在程式啟動時自動註冊），但 `ctest` 是一個獨立的外部工具，它不會自動知道你的執行檔裡藏著測試框架**，必須靠 `CMakeLists.txt` 裡明確的幾行設定，把「這個 target 有測試、測試內容是什麼」的資訊寫出來給 `ctest` 讀。

回頭檢查自己的 `CMakeLists.txt`，發現整份檔案裡完全沒有以下這幾行：

```cmake
enable_testing()

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(drone_tests)
```

缺了它們，`drone_tests` 這個 target 對 CMake 來說就只是一支普通的執行檔，跟 CTest 機制毫無關聯。逐行拆解這幾行分別在做什麼：

- `enable_testing()`：整個 CTest 機制的總開關，沒開什麼都不會動。
- `list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)`：Catch2 原始碼裡有一個 `extras/Catch.cmake`，裡面定義了 `catch_discover_tests` 這個函式，這行告訴 CMake 去哪裡找它。
- `include(CTest)`、`include(Catch)`：分別載入 CMake 內建的 CTest 模組，和上一行找到的 Catch2 專用模組。
- `catch_discover_tests(drone_tests)`：真正做事的一行。它會在 `drone_tests` build 完之後，自動用特殊參數執行這支程式一次，把它內部所有 `TEST_CASE` 的名稱列出來，寫成一份清單交給 CTest，`ctest` 指令才知道要跑哪些測試、怎麼跑。

補上這四行、重新 configure，`ctest` 終於能正確列出兩個測試案例，結果跟直接執行 `drone_tests.exe` 看到的一致。

## 學到的事

這兩顆雷合起來，其實點出同一種思維習慣：**「東西動了」不代表「東西被正確接上了」**。第一顆雷裡，`cmake --build` 這個指令本身沒有語法錯誤、看起來一切正常在跑，但實際呼叫的編譯器是錯的；第二顆雷裡，測試執行檔本身完全正常、Catch2 內部的測試註冊機制也完全正常，但它和 `ctest` 之間那座橋（`catch_discover_tests`）從頭到尾沒被搭起來。兩次的症狀都是「表面上看起來快成功了，但差最後一塊拼圖」，而且錯誤訊息本身都不會直接告訴你少了哪一塊——`No tests were found` 不會說「你忘記寫 `enable_testing()`」，`gmtime_s not declared` 也不會說「你的編譯器版本不對」，得往上一層去想「這個症狀通常代表哪個環節被跳過了」。

具體技巧上也學到兩招以後可以直接複用：一是懷疑編譯器版本時，別只看資料夾路徑，直接對可疑的執行檔跑 `--version`；二是 `FetchContent` 抓下來的第三方套件如果有自己的 CMake 輔助模組（像 Catch2 的 `extras/Catch.cmake`），通常要手動 `list(APPEND CMAKE_MODULE_PATH ...)` 才抓得到，這一步很容易被官方 README 的簡短範例帶過去而漏掉。

接下來要進第 0 章第三關，寫 `CsvLogger` 練 RAII 了。
