# 無人機飛控模擬器：挑戰之路

> 把整個專案拆成 36 個小挑戰。每一關都有明確的**目標**、可自我檢驗的**驗收條件**、這關要刻意練的 **C++ 重點**，以及分層**提示**（卡住再看，提示一 → 提示二逐層揭開）。
>
> 規則：一次只解一關，通過驗收才前進。每一關結束 commit 一次。卡超過兩三天就看提示，不丟人——真實工程也是這樣做的。

---

## 第 0 章：軍火庫（環境與基礎建設）

> 這章沒有物理，但它決定之後每一章的 debug 體驗。不要跳過。

### 挑戰 0.1：Hello, CMake
- **目標**：在你的電腦上建立專案骨架：`git init`、CMake 專案、一個會印出 "hello drone" 的 `main.cpp`，能用指令列一鍵 build + run。
- **驗收**：`cmake -B build && cmake --build build && ./build/drone_sim` 印出文字。改一行程式重新 build 能看到變化。
- **C++ 重點**：CMake 基本結構（`cmake_minimum_required`、`project`、`add_executable`）、編譯器旗標開 `-Wall -Wextra`。
- **環境備忘**：Windows → 建議 Visual Studio (MSVC) 或 MSYS2 + MinGW；macOS → `xcode-select --install` + Homebrew 裝 cmake；Linux → `sudo apt install build-essential cmake`。
- <details><summary>提示一</summary>先讓 CMake 專案只有一個 executable target，不要急著分 library。目錄結構 `src/` + `CMakeLists.txt` 就夠。</details>
- <details><summary>提示二</summary>最小 CMakeLists 大約 5 行。搜尋關鍵字："CMake minimal executable example"。</details>

### 挑戰 0.2：第一個測試
- **目標**：接入 Catch2（或 GoogleTest），寫一個會過的測試和一個會掛的測試，確認測試框架真的在工作。
- **驗收**：`ctest` 或直接跑測試執行檔，能看到一綠一紅；把紅的修好後全綠。
- **C++ 重點**：CMake 的 `FetchContent` 拉第三方套件、分離 test target 與 main target。
- <details><summary>提示一</summary>Catch2 v3 用 FetchContent 只要幾行，官方 README 有現成範例。</details>
- <details><summary>提示二</summary>先寫 `REQUIRE(1 + 1 == 2)` 這種笨測試就好，這關驗的是管線不是數學。</details>

### 挑戰 0.3：黑盒子（CSV Logger + 畫圖）
- **目標**：寫一個 `CsvLogger` class：開檔、寫 header、每步 append 一列數值、解構時關檔。再寫一支 Python 腳本讀 CSV 畫曲線。
- **驗收**：C++ 程式輸出假資料（如 sin 波）到 CSV，Python 畫出正確的 sin 曲線圖檔。
- **C++ 重點**：**RAII**（檔案在解構子關閉）、`std::ofstream`、避免 copy（刪除 copy constructor 或用 move）。
- <details><summary>提示一</summary>介面想像成：`CsvLogger log("out.csv", {"t","z","vz"}); log.row({0.0, 100.0, 0.0});`</details>
- <details><summary>提示二</summary>RAII 的意思是：建構子拿資源、解構子還資源，中途 return 或丟例外都不會漏關檔。這個 class 就是 RAII 的最小教材。</details>

---

## 第 1 章：讓東西掉下來（2D 剛體動力學）

> 整章都在 2D：狀態是 x、z、pitch 角 θ。先不碰 quaternion。

### 挑戰 1.1：打造 Vec2
- **目標**：寫自己的 `Vec2` class：加減、純量乘除、內積、長度、正規化，全部配測試。
- **驗收**：至少 10 個測試全綠，包含邊界案例（零向量正規化怎麼辦？）。
- **C++ 重點**：**operator overloading**（成員 vs 非成員函式的選擇）、**const-correctness**（哪些方法該標 `const`）、值語意（傳值 vs 傳 const 參考）。
- <details><summary>提示一</summary>`a + b` 建議寫成非成員函式，`a += b` 寫成成員函式，然後用 `+=` 實作 `+`。</details>
- <details><summary>提示二</summary>零向量正規化沒有標準答案——回傳零向量、assert、或回傳 optional 都行，重點是**做出決定並寫成測試**。</details>

### 挑戰 1.2：自由落體
- **目標**：建一個質點（只有位置、速度、質量），施加重力，用 explicit Euler 積分模擬 3 秒，狀態逐步 log 到 CSV。
- **驗收**：**解析解對照**——寫成測試：模擬 t 秒後的位置與 z₀ − ½gt² 的誤差小於某容忍值；畫出的 z(t) 曲線是漂亮的拋物線。
- **C++ 重點**：struct 設計（`State { Vec2 pos; Vec2 vel; }`）、把 dt 當明確參數傳遞（不要藏在全域）。
- <details><summary>提示一</summary>Euler 積分就兩行：`vel += acc * dt; pos += vel * dt;`。順序不同會得到 semi-implicit Euler——查查兩者差在哪，選一個並註解記錄。</details>
- <details><summary>提示二</summary>容忍值和 dt 有關：Euler 的全域誤差是 O(dt)。dt=0.001、模擬 3 秒，誤差量級大約在公分級。</details>

### 挑戰 1.3：誤差顯微鏡
- **目標**：量化 Euler 的誤差。用不同 dt（0.1, 0.01, 0.001…）跑同一場自由落體，畫「dt vs 最終誤差」的 log-log 圖。
- **驗收**：log-log 圖上的點近似一條斜率 1 的直線（一階方法的證據）。你能用一句話解釋為什麼。
- **C++ 重點**：把模擬包成可重複呼叫的函式（參數進、結果出，無全域狀態）。
- <details><summary>提示一</summary>log-log 圖上，誤差 ∝ dtⁿ 會呈現斜率 n 的直線。這是驗證積分器階數的標準手法。</details>
- <details><summary>提示二</summary>如果斜率不是 1，先檢查：誤差是不是用「解析解 − 數值解」算的？模擬總時長每組是否完全一致？</details>

### 挑戰 1.4：RK4 積分器
- **目標**：實作 Runge-Kutta 4 積分器，重跑 1.3 的實驗，和 Euler 的誤差曲線疊在同一張圖上。
- **驗收**:RK4 的 log-log 誤差線斜率約為 4，且同樣 dt 下誤差比 Euler 小好幾個數量級。
- **C++ 重點**：把「積分器」抽象成可替換的元件——函式模板或 `std::function`，讓 Euler 和 RK4 共用同一個介面。
- <details><summary>提示一</summary>介面可以長這樣：積分器吃「導數函式 f(state, t) → derivative」和 dt，回傳新 state。這樣動力學和積分法就解耦了。</details>
- <details><summary>提示二</summary>RK4 需要 state 支援加法和純量乘法。如果你的 State 是 struct，幫它也加上 operator overloading，或轉成向量表示。</details>

### 挑戰 1.5：會轉的箱子
- **目標**：從質點升級成 2D 剛體：加入 pitch 角 θ 和角速度 ω，能施加力矩讓它旋轉。
- **驗收**：測試一：施加固定力矩，θ(t) 應符合 ½(τ/I)t²（轉動版拋物線）。測試二：無力矩時 ω 恆定不變。
- **C++ 重點**：擴充 State 而不破壞既有測試（回歸測試的價值第一次體現）。
- <details><summary>提示一</summary>2D 的「慣量」只是一個純量 I，轉動方程就是 α = τ/I，和 a = F/m 完全平行。</details>
- <details><summary>提示二</summary>如果 1.4 做得好，這關只是把 state 從 4 維擴到 6 維、導數函式多兩行。若覺得改動很痛苦，代表抽象需要重構——這本身就是有價值的訊號。</details>

### 挑戰 1.6：裝上馬達
- **目標**：給剛體裝兩顆馬達（左右各距質心 L）。馬達推力沿機身法線方向（隨 θ 旋轉），兩馬達推力和產生合力、推力差產生力矩。
- **驗收**：測試一：兩馬達各推 mg/2，機體懸停不動（速度、位置、θ 全程近似不變）。測試二：左推力略大於右，機體向右傾斜並向右移動（畫軌跡圖目測確認方向正確）。
- **C++ 重點**：body frame → world frame 的旋轉（2D 只是一個 2×2 旋轉矩陣或 sin/cos 兩行）——這是第 4 章 quaternion 的迷你預習。
- <details><summary>提示一</summary>推力在 body frame 永遠是 (0, F)，轉到 world frame 才參與牛頓方程。先寫一個 `rotate(Vec2 v, double theta)` 函式並測試它（轉 90° 的已知答案）。</details>
- <details><summary>提示二</summary>力矩 = (F_left − F_right) × L（注意正負號慣例：哪邊大會往哪轉？定義好並寫進測試）。</details>

### 挑戰 1.7：見證失控
- **目標**：開環實驗——不做任何控制，給一個「幾乎懸停但略不對稱」的推力組合，模擬 10 秒，畫出軌跡。
- **驗收**：親眼看到機體先緩慢傾斜、然後加速翻滾墜落的完整過程曲線。寫三五行筆記：為什麼四軸無人機天生不穩定？
- **C++ 重點**：無新題——這關是物理直覺課，也是第 2 章存在的理由。
- <details><summary>提示一</summary>傾斜讓推力方向偏掉 → 垂直分量不足開始掉高度 → 沒有任何回復力矩 → 傾斜持續累積。這叫開環不穩定。</details>

---

## 第 2 章：把它穩住（2D 控制）

> 本章預期是全專案最花時間的一章。卡關是正常現象，不是你的問題。

### 挑戰 2.1：PID 積木
- **目標**：寫一個獨立的 `PidController` class：吃 (目標值, 當前值, dt)，吐控制輸出；gain 可設定；先不管飽和。
- **驗收**：單元測試：純 P 控制下輸出 = Kp×誤差；I 項會隨恆定誤差線性累積；D 項對階躍誤差的反應正確。
- **C++ 重點**：class 設計——最小介面、內部狀態封裝（積分累積值、上次誤差）、一個 `reset()` 方法。
- <details><summary>提示一</summary>PID 有內部狀態（積分、前次誤差），所以它是 class 不是純函式。想想哪些成員該是 private。</details>
- <details><summary>提示二</summary>D 項用「誤差變化率」還是「測量值變化率」有差（後者避免 setpoint 突變時的 derivative kick）。先用前者，註解記下這個坑。</details>

### 挑戰 2.2：定高
- **目標**：用一個 PID 控制總推力，讓機體從 z=0 起飛並穩定停在 z=10（θ 先鎖定為 0，假裝姿態完美）。
- **驗收**：畫出 z(t) 的 step response：能到達 10、過衝 < 20%、最後穩在 10±0.1。記錄你最終的 Kp/Ki/Kd 和調參過程筆記。
- **C++ 重點**：把控制器接進模擬迴圈的架構（控制頻率 = 物理頻率，先簡化）。
- <details><summary>提示一</summary>標準調參流程：先只開 Kp 調到會震盪 → 加 Kd 壓震盪 → 最後補一點 Ki 消穩態誤差。每改一次 gain 就畫一次曲線。</details>
- <details><summary>提示二</summary>重力是恆定干擾——沒有 Ki 時會有穩態誤差（懸停在 9.x 而不是 10）。也可以用 feedforward：直接在輸出加上 mg，Ki 的負擔會小很多。兩種都試試。</details>

### 挑戰 2.3：定姿態
- **目標**：解鎖 θ。用第二個 PID 控制左右推力差，讓機體從初始傾斜 30° 回正並保持水平。
- **驗收**：θ(t) 曲線從 30° 收斂到 0±0.5°，不持續震盪。定高功能同時仍正常（兩個 PID 同時工作）。
- **C++ 重點**：兩個控制器的輸出如何合成馬達指令——這就是最簡版 **motor mixing**：`F_left = F_total/2 + δ, F_right = F_total/2 − δ`。
- <details><summary>提示一</summary>姿態 PID 的輸出是「推力差 δ」。注意它和 1.6 定義的正負號慣例要一致，不然會朝反方向修正、越修越歪。</details>
- <details><summary>提示二</summary>姿態環要調得比高度環「快」（gain 相對大、反應時間短）——內環快於外環是串級控制的鐵律，下一關就會正式用到。</details>

### 挑戰 2.4：串級——飛到那裡
- **目標**：實作 2D 串級控制：水平位置 PID 的輸出是「目標傾角 θ_ref」（因為要往右飛必須先往右傾），姿態 PID 追這個 θ_ref。加上定高，機體能飛到指定的 (x, z) 並停住。
- **驗收**：從 (0,0) 飛到 (5,10)，x 和 z 都收斂到目標 ±0.1，θ 最後回到 0。畫出 x(t)、z(t)、θ(t) 三條曲線。
- **C++ 重點**：控制器的組合架構——外環的輸出是內環的 setpoint。想清楚資料流再動手。
- <details><summary>提示一</summary>θ_ref 要限幅（例如 ±20°），否則位置誤差大時外環會命令機體翻 90° 側飛，直接失控。</details>
- <details><summary>提示二</summary>先把外環 gain 調得很保守（慢慢飛），確認架構正確，再逐步加快。架構錯和 gain 錯的症狀很像，先排除前者。</details>

### 挑戰 2.5：撞牆——飽和與 anti-windup
- **目標**：給馬達加上物理極限（0 ≤ F ≤ F_max）。然後設計一個會撞極限的場景（如目標點在很遠處），觀察 integrator windup 的災難，再實作 anti-windup 修好它。
- **驗收**：對照實驗兩張圖——無 anti-windup：到達目標時猛烈過衝；有 anti-windup：正常收斂。兩張圖都留檔，這是你最有故事性的作品證據。
- **C++ 重點**：`std::clamp`、PID class 的介面演化（如何優雅地讓外部告知「輸出被截斷了」）。
- <details><summary>提示一</summary>最簡單的 anti-windup：輸出飽和期間凍結積分項累積（conditional integration）。還有 back-calculation 等進階做法，先做簡單的。</details>
- <details><summary>提示二</summary>windup 的機制：飽和期間誤差持續灌進積分項 → 積分巨大 → 誤差歸零後積分項還在推 → 過衝到反方向才慢慢洩掉。在圖上把積分項的值也畫出來，你會親眼看到這個過程。</details>

### 挑戰 2.6：小考——航點巡航
- **目標**：綜合關卡。給一串 waypoint（如 (0,10)→(5,10)→(5,5)→(0,5)），機體依序抵達每個點（進入 0.2 範圍內視為到達），最後降落回原點。
- **驗收**：完整軌跡圖畫出來近似一個矩形巡邏路線，全程無失控。
- **C++ 重點**：簡單的狀態機（enum class + switch，管理「前往第 n 個航點」的邏輯）。
- <details><summary>提示一</summary>降落不是「目標 z=0 全速衝」——給一個緩慢下降的目標速度或分段目標高度，否則會砸地。</details>

---

## 第 3 章：看得見（視覺化）

### 挑戰 3.1：開窗
- **目標**：裝 SDL2，開出一個視窗，跑一個乾淨的 game loop：處理事件（可關窗）、清空畫面、呈現，60fps。
- **驗收**：視窗打開、能關閉、CPU 不會吃滿（有正確的 frame 節流）。
- **C++ 重點**：**RAII wrapper**——把 `SDL_Window*`、`SDL_Renderer*` 包進自動釋放的 class（或用 `std::unique_ptr` 配自訂 deleter）。這是本專案最精華的 RAII 練習。
- <details><summary>提示一</summary>SDL 是 C API，到處都是要手動 destroy 的裸指標——正是 RAII 的用武之地。目標：你的 main 裡看不到任何 `SDL_Destroy*` 呼叫。</details>
- <details><summary>提示二</summary>`std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>` 是一種寫法；自己寫小 wrapper class 是另一種。都試過再選。</details>

### 挑戰 3.2：心跳分離——Fix Your Timestep
- **目標**：實作 fixed timestep + accumulator：物理永遠以固定 dt（如 1/240s）更新，渲染以螢幕更新率跑，兩者解耦。
- **驗收**：把渲染人為拖慢（模擬掉幀），物理結果（最終位置）和不掉幀時**完全一致**（bit-level 或誤差極小）。這證明物理與 render rate 無關。
- **C++ 重點**：game loop 架構、`std::chrono` 計時。
- <details><summary>提示一</summary>經典文章就叫 "Fix Your Timestep"（Gaffer On Games），照著理解後自己實作，不要照抄。</details>
- <details><summary>提示二</summary>核心：每 frame 把實際流逝時間加進 accumulator，`while (accumulator >= dt) { physics_step(dt); accumulator -= dt; }`。</details>

### 挑戰 3.3：讓它出現
- **目標**：把第 2 章的模擬畫出來：側視圖的無人機（一條線代表機身 + 兩端小方塊代表馬達，隨 θ 旋轉）、地面線、目標點標記。
- **驗收**：即時看到 2.6 的航點巡航全程動畫，機身傾角肉眼可辨且方向正確。
- **C++ 重點**：世界座標 → 螢幕座標的轉換（注意螢幕 y 軸朝下！）、渲染程式碼與物理程式碼的分離。
- <details><summary>提示一</summary>寫一個 `Camera` 或 `worldToScreen()` 函式集中處理座標轉換，不要在畫圖處散落 magic number。</details>
- <details><summary>提示二</summary>螢幕 y 朝下但世界 z 朝上，這是你第一個「座標系慣例」實戰——比第 4 章的溫和多了，把它當熱身。</details>

### 挑戰 3.4：接上方向盤
- **目標**：鍵盤互動：方向鍵或滑鼠點擊即時改變目標點，無人機追過去。加一個鍵切換「手動推力模式」（直接控制兩馬達，體驗徒手駕駛有多難）。
- **驗收**：能用滑鼠點哪飛哪；手動模式下你大概率墜機——這反而證明第 2 章的價值。
- **C++ 重點**：事件處理、輸入層/控制層/物理層的架構分離。
- <details><summary>提示一</summary>滑鼠座標要經過 3.3 的座標轉換的**反函式**才是世界座標。</details>

---

## 第 4 章：進入三維（大魔王章）

> 開工前先寫**慣例筆記**（挑戰 4.0）。本章每一關都嚴重依賴它。

### 挑戰 4.0：立法——慣例筆記
- **目標**：寫一頁 `CONVENTIONS.md`，白紙黑字決定：world frame 用 ENU 還是 NED、body frame 軸向、quaternion 儲存順序 (w,x,y,z)、Hamilton 慣例、rotation 代表 body→world 還是反向、歐拉角順序（若用到）。
- **驗收**：文件存在且每項都有明確答案。之後所有程式碼註解引用它。
- <details><summary>提示一</summary>建議組合：ENU（z 朝上，直覺）、Hamilton、(w,x,y,z)、quaternion 代表 body→world 旋轉。和多數教材一致，查資料時最省事。</details>

### 挑戰 4.1：Vec3 與 Mat3
- **目標**：擴充數學庫：`Vec3`（含 cross product）、`Mat3`（矩陣乘法、轉置、matrix-vector 乘法），完整測試。
- **驗收**：測試全綠，包含 cross product 的右手定則案例（x̂ × ŷ = ẑ）。
- **C++ 重點**：程式碼與 Vec2 大量相似——考慮要不要用 template 統一（`Vec<N>`）？做出取捨並記錄理由（過度泛化也是病）。
- <details><summary>提示一</summary>不 template 也完全合法——三個維度就寫三個 class 是很多成熟引擎的選擇。重點是「有意識地選」。</details>

### 挑戰 4.2：Quaternion——本專案最危險的一關
- **目標**：實作 `Quaternion` class：乘法、共軛、正規化、旋轉向量、從軸角建構。**同時**把 Eigen 接進測試依賴，每個運算用隨機輸入和 Eigen 對照。
- **驗收**：對照測試（隨機 1000 組輸入 vs Eigen）全過；已知答案測試：繞 z 轉 90° 把 x̂ 變成 ŷ（依你 4.0 的慣例）；連續旋轉合成順序正確。
- **C++ 重點**：以參考實作驗證自製程式碼的測試手法；FetchContent 引入 Eigen（header-only，只給 test target 用）。
- <details><summary>提示一</summary>Eigen 的 quaternion 儲存順序和建構子參數順序**不一樣**（內部 x,y,z,w；建構子 w,x,y,z）——對照時最容易在這裡誤判自己寫錯。</details>
- <details><summary>提示二</summary>quaternion 乘法 16 項展開錯一個正負號，旋轉「大致對但慢慢歪」。隨機對照測試就是為了抓這種肉眼不可見的錯。</details>

### 挑戰 4.3：3D 剛體
- **目標**：3D 動力學：state 為 position(3) + velocity(3) + quaternion(4) + angular velocity(3)。實作含慣量張量的轉動方程與 quaternion 微分方程，接上 RK4。
- **驗收**：測試一：自由落體仍過（回歸）。測試二：無力矩自轉時角動量守恆、quaternion 模長維持 1（記得每步正規化）。測試三：繞單軸恆定力矩的解析解對照。
- **C++ 重點**：State 泛化後既有積分器的相容性——第 1 章的抽象在這裡接受期末考。
- <details><summary>提示一</summary>quaternion 微分：q̇ = ½ q ⊗ (0, ω)。ω 在 body frame 還是 world frame 取決於乘的順序——回去看你的 CONVENTIONS.md。</details>
- <details><summary>提示二</summary>轉動方程（body frame）：I ω̇ = τ − ω × (I ω)。第二項是 gyroscopic term，對稱慣量+單軸旋轉時為零，這正好給你一個分段驗證的機會。</details>

### 挑戰 4.4：四顆馬達
- **目標**：X 型四旋翼配置：4 馬達推力 → 總推力 + 三軸力矩（yaw 力矩來自螺旋槳反扭矩，注意對角線馬達旋向相反）。參數**抄 Crazyflie 2.x**：質量 27g、機臂與慣量、推力係數。寫出 mixing matrix 及其反矩陣。
- **驗收**：測試：四馬達等推力 = 純升力零力矩；對角線增減 = 純 yaw；前後差 = 純 pitch。懸停測試（推力和 = mg）位置姿態全程不動。
- **C++ 重點**：用 Mat3/矩陣思維組織 mixing，而不是散落的加減式。
- <details><summary>提示一</summary>搜尋 "Crazyflie 2.1 system identification" 可找到公開參數論文。慣量量級約 1e-5 kg·m²——如果你自己估的差了幾個數量級，就是它救你的時候。</details>

### 挑戰 4.5：3D 穩定
- **目標**：3D 串級姿態控制：attitude controller（quaternion 誤差 → 目標角速度）+ rate controller（角速度誤差 → 三軸力矩）+ 定高，從任意初始傾斜回到水平懸停。
- **驗收**：初始 roll 30°、pitch 20° 的擾動下，3 秒內回到水平懸停；曲線圖顯示三軸角速度都收斂到 0。
- **C++ 重點**：控制架構的擴展——2D 的兩層變 3D 的多層，檢驗你第 2 章的抽象是否撐得住。
- <details><summary>提示一</summary>quaternion 姿態誤差：q_err = q_ref ⊗ q⁻¹，取其向量部分（小角度近似下正比於旋轉軸×角度）當 P 控制的誤差輸入。這是業界飛控（如 PX4）的實際做法之一。</details>
- <details><summary>提示二</summary>rate 環（內環）先單獨調穩——直接命令「角速度歸零」，確認能剎住旋轉，再套外面的 attitude 環。</details>

### 挑戰 4.6：3D 航點巡航
- **目標**：完整 3D 位置控制（position → velocity → attitude → rate 全串級），飛一個 3D 路徑（如水平正方形 + 高度變化），視覺化升級：至少畫兩個視角（側視 + 俯視），或挑戰簡單 3D 投影。
- **驗收**：3D 航點全部依序到達，全程穩定；動畫肉眼可看出機體傾斜方向與移動方向的正確關係。
- **C++ 重點**：這是全專案架構的總驗收——如果每章的抽象都健康，這關主要是組裝而非新寫。
- <details><summary>提示一</summary>位置環輸出「期望加速度向量」，換算成期望姿態：機體 z 軸應指向（期望加速度 + 重力補償）的方向。這一步叫 acceleration-to-attitude mapping。</details>

---

## 第 5 章：真實世界的噪音（選做進階）

### 挑戰 5.1：弄髒感測器
- **目標**：模擬 IMU：gyro 輸出 = 真實角速度 + 高斯白噪音 + 緩慢漂移的 bias；accelerometer 同理。控制器**只能**讀感測器輸出，不能再偷看真值。
- **驗收**：畫「真值 vs 感測值」對照圖，噪音特性肉眼可見；直接用髒資料餵控制器，觀察並記錄性能退化（抖動、漂移）。
- **C++ 重點**：`<random>`（`std::normal_distribution`、seed 管理讓實驗可重現）、介面設計——「感測器」作為模擬真值與控制器之間的抽象層。
- <details><summary>提示一</summary>bias 用隨機漫步模擬：每步 bias += 小高斯噪音。這是 IMU 的真實行為，也是後面濾波器要解決的主因。</details>

### 挑戰 5.2：Complementary Filter
- **目標**：用互補濾波器融合 gyro（短期準、長期漂）與 accelerometer（長期準、短期吵）估測姿態。
- **驗收**：畫三線圖：真實姿態、純 gyro 積分（會漂走）、濾波估測（貼著真值）。估測誤差 < 2°。
- **C++ 重點**：濾波器作為又一個「有內部狀態的小 class」——和 PID 結構同型，比較兩者的介面設計。
- <details><summary>提示一</summary>核心一行：estimate = α×(estimate + gyro×dt) + (1−α)×accel_angle，α 約 0.98。理解為什麼這樣就同時取了兩者之長。</details>

### 挑戰 5.3：閉上眼睛飛
- **目標**：把 5.2 的估測值（而非真值）餵給第 4 章的控制器，重跑 3D 航點巡航。
- **驗收**：仍能完成巡航（允許比真值版抖一點）；記錄兩版的軌跡誤差對比。
- <details><summary>提示一</summary>如果失控，先降低控制 gain——估測延遲相當於在迴路裡加了延遲，過高的 gain 會放大它。</details>

### 挑戰 5.4：Kalman Filter（硬核加分題）
- **目標**：先做 1D 練習：用 KF 融合「帶噪音的高度計 + 帶噪音的垂直加速度」估測高度與垂直速度。行有餘力再挑戰姿態 EKF。
- **驗收**：KF 估測誤差顯著小於任一單獨感測器；你能解釋 Kalman gain 在「信模型」與「信量測」之間怎麼權衡。
- **C++ 重點**：矩陣運算實戰（你的 Mat3 或 Eigen）；這是機器人業界面試最常考的演算法，做完記得寫進履歷。
- <details><summary>提示一</summary>1D KF 只有 2×2 矩陣，手推得動。先在紙上跑一兩步迭代再寫程式，理解每個矩陣的物理意義。</details>

---

## 最終 Boss

**場景**：3D 無人機、Crazyflie 真實參數、感測器有噪音、只靠估測姿態，從地面起飛 → 依序通過 5 個 3D 航點 → 回到原點緩降著陸。全程即時視覺化 + 完整資料 log。

**通關即代表你擁有**：自製數學庫（含 quaternion）、RK4 積分器、3D 剛體引擎、串級 PID 飛控、motor mixing、感測器模擬與濾波、SDL2 即時視覺化、完整測試套件——這是一份可以直接放上 GitHub 當履歷作品的專案。

---

> 節奏建議：一週 1–3 關是健康速度。第 2 章和第 4 章的單關可能要一週以上，屬正常。每完成一章，回頭看第 8 個盲點的 C++ 清單，確認手感有練到。
>
> 逐關進度請見 [`README.md`](../README.md) 的進度追蹤表。
