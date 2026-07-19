# 從「Hello, CMake」到看得懂紅線：無人機模擬器第 0 章踩雷實錄

最近在做一個純軟體的無人機飛控模擬器練習專案，把整個學習路徑拆成一個挑戰之路，第 0 章第一關叫「Hello, CMake」——目標簡單到不能再簡單：建一個 CMake 專案，印出 `hello drone`，確認 `cmake -B build && cmake --build build && ./build/drone_sim` 能跑。

我以為這關五分鐘搞定。結果它變成一趟完整的 Windows C++ 開發環境踩雷之旅，幾乎把「編譯器」「建構系統」「編輯器」這三層各自獨立又互相牽連的地雷都踩過一輪。寫下來，一方面給自己留個紀錄，一方面也許能幫到同樣在 Windows 上從零架 C++ 環境的人。

## 專案骨架本身沒有懸念

`CMakeLists.txt` 就標準的五行：

```cmake
cmake_minimum_required(VERSION 3.16)
project(drone_sim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(drone_sim src/main.cpp)
target_compile_options(drone_sim PRIVATE -Wall -Wextra)
```

`main.cpp` 就一句 `std::cout << "hello drone\n";`。這部分完全沒出過問題——出問題的從來不是程式碼，是「怎麼讓程式碼被編譯出來」這件事本身。

## 地雷一：`cmake` 根本不是一個指令

第一步就卡關：

```
cmake : 無法辨識 'cmake' 詞彙是否為 Cmdlet、函數、指令檔或可執行程式的名稱。
```

原因很單純：系統上沒裝 CMake，或裝了但沒進 PATH。我剛好 PowerShell 開著是 `(base)`，代表 Anaconda 的 base 環境是啟用的，順手用 `conda install -c conda-forge cmake` 裝，裝完當下那個視窗馬上就能用，不用重開。這是這趟旅程裡最不痛的一關。

## 地雷二：CMake 選錯了 Generator，找不到編譯器

`cmake` 指令有了，但一跑 `cmake -B build`：

```
-- Building for: NMake Makefiles
CMake Error at CMakeLists.txt:2 (project):
  Running 'nmake' '-?' failed with: no such file or directory
CMake Error: CMAKE_CXX_COMPILER not set, after EnableLanguage
```

CMake 在 Windows 上如果沒有明確指定，會自己猜一個 generator。它猜了 `NMake Makefiles`，但 `nmake.exe` 屬於 Visual Studio 的工具鏈，需要在「Developer Command Prompt」那種已經設好一堆環境變數的殼裡才找得到，我當時只是一個普通的 PowerShell 視窗，自然什麼都抓不到。

這裡學到的事：**CMake 找不找得到編譯器，跟「電腦上有沒有裝編譯器」是兩個問題，先想清楚要走哪條路（MSVC 還是 MinGW），再決定要不要明確指定 `-G`。**

## 地雷三：「我明明裝了 CLion，為什麼還是找不到」

這是最容易讓人卡住想不通的一關。我心想我用 CLion 寫過 C++、跑得好好的，環境變數裡也「有它」，怎麼會找不到編譯器？

答案是：CLion 這種 IDE 的工作方式，是自己內部記著一份 toolchain 設定（CMake 路徑、編譯器路徑，在 `Settings → Build, Execution, Deployment → Toolchains` 裡看得到），IDE 在幫你 build 的時候直接照著這份設定去呼叫對應的執行檔絕對路徑，完全不需要系統 PATH 配合。而我加進 PATH 的多半只是 CLion 主程式所在的資料夾（能讓你打指令開啟 IDE），不是那些藏在很深子目錄裡的 `cmake.exe`、`g++.exe`。這兩層是分開的，「IDE 能跑」不代表「終端機能跑」。

解法就是進 CLion 的 Toolchains 設定,把它實際在用的 CMake 和編譯器完整路徑挖出來，把「那些執行檔所在的資料夾」（不是外層大資料夾）分別加進系統 PATH，開新終端機視窗驗證 `cmake --version`、`g++ --version` 都吐得出版本號。

## 地雷四：改了 Generator，CMake 說「跟之前不一樣」

編譯器抓到了，重新指定 generator：

```powershell
cmake -B build -G "MinGW Makefiles"
```

結果噴：

```
CMake Error: generator : MinGW Makefiles
Does not match the generator used previously: NMake Makefiles
```

這個很好懂：上一次失敗的 `cmake -B build` 已經在 `build/` 資料夾裡留下 `CMakeCache.txt`，把 generator 鎖死成 NMake 了。`build/` 從頭到尾都是 CMake 自己產生出來的東西，從來不屬於原始碼，砍掉重建完全安全：

```powershell
Remove-Item -Recurse -Force build
cmake -B build -G "MinGW Makefiles"
```

順手把 `build/` 加進 `.gitignore`，這種「殘留快取鎖住舊設定」的狀況以後只會更常遇到，直接養成「卡住先想是不是快取問題」的反射動作。

## 地雷五：Build 成功了，但路徑變成亂碼

清乾淨重跑之後，總算 configure 成功，但 `cmake --build build` 又報：

```
mingw32-make: *** No rule to make target 'D:/?辣/drone_sim/src/main.cpp'
```

`D:\文件\drone_sim` 裡的「文件」兩個中文字被搞成亂碼 `?辣`。這是 MinGW 工具鏈一個很經典的老毛病：路徑裡只要有非 ASCII 字元，某些版本的 `make`/`gcc` 在編碼處理上就有機率出包，去找一個實際不存在的亂碼路徑。跟程式碼、CMake 設定完全無關，純粹是工具鏈對非英數路徑不夠健壯。

解法很暴力但有效：把整個專案搬到全英數路徑，例如 `D:\dev\drone_sim`，搬完記得砍掉 `build/` 重新 configure（裡面快取的還是舊路徑）。順便學到一課：**這種專案往後三十幾關都要在同一個資料夾下開發，路徑裡有中文字這顆雷遲早會在裝其他第三方套件時再被踩到一次，不如現在就把開發習慣改成純英數路徑。**

## 地雷六、七：終端機能跑了，VSCode 卻整片紅線

到這裡命令列已經完全正常，`hello drone` 也印出來了。打開 VSCode 卻發現 `#include <iostream>`、`std::cout`、`std::endl` 全部畫著紅色波浪線，跳出 `'iostream' file not found`。

第一直覺是「VSCode 視窗還開著搬家前的舊資料夾」——因為我剛把專案從 `D:\文件\drone_sim` 搬到 `D:\dev\drone_sim`，如果視窗沒有重新指向新路徑，整套設定當然全部失效。重新 `File → Open Folder` 選對新路徑，紅線沒消。

往下查才發現真正的原因分兩層：

**第一層**，clangd（VSCode 裡負責 C++ 智慧提示的語言伺服器）需要一份 `compile_commands.json` 才知道每個檔案實際的編譯參數和 include path 在哪，要在 `CMakeLists.txt` 裡明確加：

```cmake
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
```

重新 configure 之後才會在 `build/` 裡生出這份檔案，並在 `.vscode/settings.json` 告訴 clangd 去哪裡找：

```json
{
  "clangd.arguments": ["--compile-commands-dir=build"]
}
```

**第二層**，也是最隱蔽的一層：把這份檔案接上之後，clangd 的 log 顯示它確實讀到了 `compile_commands.json`、也確實抓到編譯器路徑 `C:\MinGW\bin\c++.exe`，但依然找不到 `iostream`。

原因是 clangd 出於安全考量，只會對「白名單」裡的編譯器路徑主動執行「查詢這個編譯器內建標準函式庫路徑在哪」這個動作——因為這個查詢本質上是去執行一支外部程式，clangd 不會對任意路徑的執行檔自動這麼做，怕被惡意專案的設定檔誘騙執行任意程式。`C:\MinGW\bin\c++.exe` 這種自訂安裝路徑不在它預設信任的名單裡，於是它拿到了編譯器路徑，卻不去問這個編譯器的標準函式庫在哪，`iostream` 自然找不到。

解法是明確授權：

```json
{
  "clangd.arguments": [
    "--compile-commands-dir=build",
    "--query-driver=C:/MinGW/bin/*.exe"
  ]
}
```

加上 `--query-driver` 之後重新載入視窗，紅線才真正消失。

## 這一關真正學到的事

寫完這篇才意識到，「Hello, CMake」這關的重點從來不是那五行 `CMakeLists.txt`，而是把 Windows 上一套 C++ 開發環境拆成好幾層各自獨立運作的系統：**系統 PATH**（終端機找不找得到執行檔）、**IDE 自己的 toolchain 設定**（跟系統 PATH 完全脫鉤）、**CMake 的快取**（`CMakeCache.txt` 會鎖住第一次的設定，改設定要記得砍）、**檔案系統編碼**（非 ASCII 路徑是所有工具鏈的共同弱點）、**編輯器語言伺服器的安全機制**（build 能跑不代表 clangd 認得同一個編譯器）。這五層任何一層出包，症狀看起來都很像「就是不能動」，但病灶完全不同，得一層一層拆開排查。

也因為這樣，我現在對這句話有了更具體的體感：**「能編譯」跟「編輯器看得懂」走的是兩條完全獨立的路徑**——`cmake --build` 呼叫的是你系統上實際裝的編譯器，clangd 呼叫的是它自己查詢/快取出來的一份設定，兩邊各自出錯互不影響對方,但呈現出來的症狀常常會讓人以為是同一個問題。之後遇到類似「build 過但編輯器還在噴錯」的情況，會直接往 `compile_commands.json` 有沒有生成、clangd 有沒有讀到、`--query-driver` 有沒有授權這三個方向查，不用再從頭亂猜。

最終能動的組合：**CMake + MinGW-w64（g++ / mingw32-make）+ VSCode clangd（帶 `--compile-commands-dir` 和 `--query-driver`）**，專案放在純英數路徑下。接下來要進第 0 章第二關，接 Catch2 測試框架了。
