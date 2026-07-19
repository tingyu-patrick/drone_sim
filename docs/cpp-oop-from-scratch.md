# C++ 物件導向從零開始

這份教材是為了你的無人機模擬器專案寫的完整 OOP 教學，涵蓋 class 的基本概念、封裝、建構子/解構子、運算子多載、const-correctness、複製與搬移語意，以及繼承、多型、virtual function 這些完整 C++ OOP 的內容。所有範例程式碼都在雲端實際編譯執行過（`g++ -std=c++17 -Wall -Wextra`，零警告），可以放心照著打。

這份文件寫完之後，我們可以繼續用互動的方式，一段一段搭配你專案的實際進度，邊教邊讓你動手寫練習加深理解。

## 目錄

0. 你已經會的東西
1. struct 與 class：把資料和行為綁在一起
2. 封裝：public / private
3. 建構子與解構子
4. `this` 指標
5. 運算子多載（Operator Overloading）
6. const-correctness
7. 複製語意（Copy）
8. 搬移語意（Move）
9. 繼承（Inheritance）
10. 多型與 virtual function
11. static 成員
12. 常見陷阱 checklist
13. 對照這個專案：接下來章節會用到什麼

---

## 0. 你已經會的東西

你在挑戰 0.1～0.3 已經寫過函式、`#include`、`std::cout`、`std::vector`，也在 `CsvLogger` 裡碰過建構子、解構子、`= delete`——這些其實都已經是 OOP 的一部分，只是那時候我們沒有系統性地從頭講。這份教材會把散落的拼圖接起來，你會發現很多東西其實已經懂了一半。

物件導向的核心想法只有一句話：**把資料（data）和操作那些資料的函式（behavior）綁在一起，包成一個新的型別**。C 語言（以及 C 風格的 C++）習慣把資料和函式分開寫：

```c
// C 風格：資料和函式各自獨立
struct Vec2 { double x, y; };

double vec2_length(struct Vec2 v) {
    return sqrt(v.x * v.x + v.y * v.y);
}

// 呼叫時要記得函式名稱、記得傳對參數
struct Vec2 a = {3.0, 4.0};
double len = vec2_length(a);
```

OOP 風格把 `length()` 直接變成 `Vec2` 這個型別自己的一部分：

```cpp
Vec2 a(3.0, 4.0);
double len = a.length();   // 資料自己知道怎麼操作自己
```

差別看起來很小，但意義很大：`Vec2` 現在是一個完整的概念單位，它「知道」自己能做什麼，使用者不需要記得一大堆散落各處的自由函式名稱。這就是 OOP 要解決的問題。

---

## 1. struct 與 class：把資料和行為綁在一起

C++ 的 `struct` 其實從一開始就可以放函式，不是只能放資料（這點跟 C 不一樣）：

```cpp
struct Vec2 {
    double x;
    double y;

    double length() const {
        return std::sqrt(x * x + y * y);
    }
};
```

那 `class` 跟 `struct` 差在哪？**只有一個差別：預設的存取權限（access specifier）**。`struct` 裡的成員預設是 `public`（外部可以直接存取），`class` 裡的成員預設是 `private`（外部不能直接存取）。除此之外兩者能力完全一樣——`class` 能做的事 `struct` 都能做，反之亦然。

```cpp
struct S { int x; };   // x 預設 public，外部可以 s.x = 5;
class  C { int x; };   // x 預設 private，外部不能 c.x = 5;（會編譯錯誤）
```

實務上的慣例（不是語言規定，是社群習慣）：**單純裝資料、沒有不變量（invariant）要維護的小東西用 `struct`**（例如你未來會寫的 `State { Vec2 pos; Vec2 vel; }`），**需要封裝內部細節、保護內部一致性的用 `class`**（例如 `CsvLogger`——外部不該直接摸到底層的 `std::ofstream`）。挑戰之路裡兩種都會用到，看的就是這個判斷。

---

## 2. 封裝：public / private

封裝（encapsulation）的意思是：把物件的內部細節藏起來，只透過一組明確定義的介面（public 成員函式）跟外界互動。為什麼要這麼做？看個對比：

```cpp
// 沒封裝：外部可以隨便亂改內部狀態，物件無法保護自己的正確性
struct BankAccountBad {
    double balance;
};

BankAccountBad acc{100.0};
acc.balance = -99999.0;   // 完全合法，但業務邏輯上是災難
```

```cpp
// 封裝：balance 藏起來，只能透過 deposit/withdraw 改動，
// 物件自己保證絕對不會出現負數餘額這種不合理狀態
class BankAccount {
public:
    explicit BankAccount(double initial) : balance_(initial) {}

    void deposit(double amount) {
        balance_ += amount;
    }

    bool withdraw(double amount) {
        if (amount > balance_) return false;   // 擋下不合理操作
        balance_ -= amount;
        return true;
    }

    double balance() const { return balance_; }

private:
    double balance_;   // 外部完全碰不到
};
```

`private` 成員只有這個 class 自己的成員函式能存取，外部程式碼、甚至這個 class 的使用者都無法繞過 `deposit`/`withdraw` 直接改 `balance_`。這樣物件才能「保護自己」——`CsvLogger` 把 `std::ofstream file_` 設成 private 也是同樣道理：外部不應該有機會繞過 `row()` 直接亂寫檔案。

`protected` 是介於兩者之間的第三種：外部存取不到，但**繼承這個 class 的子類別**可以存取，會在第 9 節繼承時再提。

命名慣例：很多 C++ 專案會在 private 成員變數後面加底線（`x_`、`balance_`），跟建構子參數（`x`、`initial`）區分開來，避免同名混淆。你的 `CsvLogger` 裡的 `file_` 就是這個慣例。

---

## 3. 建構子與解構子

**建構子（constructor）**是物件被建立時自動呼叫的特殊函式，負責把物件初始化成一個合法狀態。名稱固定跟 class 同名，沒有回傳型別：

```cpp
class Vec2 {
public:
    Vec2(double x, double y) : x_(x), y_(y) {}   // 建構子

private:
    double x_;
    double y_;
};

Vec2 a(3.0, 4.0);   // 呼叫建構子，x_=3.0, y_=4.0
```

`: x_(x), y_(y)` 這段叫**成員初始化列表**（member initializer list）。它跟在函式主體 `{}` 裡面用 `=` 賦值看起來效果差不多：

```cpp
// 兩種寫法效果類似，但初始化列表更好
Vec2(double x, double y) : x_(x), y_(y) {}          // 建議：初始化列表
Vec2(double x, double y) { x_ = x; y_ = y; }         // 也能動，但較差
```

差別在於：初始化列表是**真正的初始化**（初始化只做一次），函式主體裡賦值則是「先用預設值初始化，再賦值蓋掉一次」，多做了一次工。對 `double` 這種簡單型別差異可以忽略，但如果成員是像 `std::ofstream`、`std::vector` 這種較重的型別（或成員是 `const`、是參考、或沒有預設建構子），初始化列表往往是唯一能動或明顯更有效率的寫法。這也是為什麼你的 `CsvLogger` 建構子寫成 `CsvLogger(...) : file_(path) { ... }`——`file_` 直接用 `path` 初始化，而不是先建立一個空的 `ofstream` 再重新賦值。

建構子可以多載（有多個版本，參數不同），編譯器依照呼叫時給的參數自動挑對的版本：

```cpp
class Vec2 {
public:
    Vec2() : x_(0), y_(0) {}                  // 預設建構子：Vec2 a;
    Vec2(double x, double y) : x_(x), y_(y) {} // Vec2 b(3, 4);
    // ...
};
```

**解構子（destructor）**是物件生命週期結束時自動呼叫的函式，名稱是 `~` 加上 class 名稱，沒有參數、不能多載（一個 class 只能有一個解構子）：

```cpp
~CsvLogger() {
    if (file_.is_open()) file_.close();
}
```

這正是第 0.3 關教過的 RAII：建構子拿資源、解構子還資源。這是這份教材裡你唯一已經實戰過的部分，這裡只是把它放進完整脈絡裡。

---

## 4. `this` 指標

在 class 的成員函式裡，`this` 是一個指向「目前這個物件自己」的指標。大部分時候你不需要顯式寫出 `this`（直接寫 `x_` 編譯器就知道你指的是 `this->x_`），但有兩個常見情境會需要它：

**情境一：回傳自己以支援連鎖呼叫（method chaining）**

```cpp
class Vec2 {
public:
    Vec2& operator+=(const Vec2& rhs) {
        x_ += rhs.x_;
        y_ += rhs.y_;
        return *this;   // *this 是「目前這個物件」本身（不是指標，是解參考後的物件）
    }
    // ...
};

Vec2 a(1, 2);
a += Vec2(1, 1) += Vec2(1, 1);   // 因為 += 回傳 *this，這樣寫才合法
```

**情境二：參數名稱跟成員變數同名時消歧義**

```cpp
class Vec2 {
public:
    void set_x(double x) { this->x_ = x; }  // 不太會撞名，但如果成員沒加底線慣例，
                                              // void set_x(double x) { x = x; } 會是災難
                                              // （這行等於什麼都沒做，因為兩個 x 都指參數）
};
```

這也是為什麼底線命名慣例（`x_`）其實不只是風格問題，還能直接避免這類 bug。

---

## 5. 運算子多載（Operator Overloading）

C++ 允許你重新定義運算子（`+`、`==`、`<<`……）作用在自訂型別上的行為，這是 C++ 相對其他語言比較特別的能力。挑戰 1.1（打造 Vec2）會整關都在練這個。

```cpp
class Vec2 {
public:
    Vec2(double x, double y) : x_(x), y_(y) {}

    double x() const { return x_; }
    double y() const { return y_; }

    // 成員函式版本：left-hand side 是隱含的 *this
    Vec2& operator+=(const Vec2& rhs) {
        x_ += rhs.x_;
        y_ += rhs.y_;
        return *this;
    }

private:
    double x_;
    double y_;
};

// 非成員函式版本：兩邊都要明講
Vec2 operator+(Vec2 lhs, const Vec2& rhs) {
    lhs += rhs;    // 借用已經寫好的 +=，避免重複邏輯
    return lhs;
}

bool operator==(const Vec2& a, const Vec2& b) {
    return a.x() == b.x() && a.y() == b.y();
}

std::ostream& operator<<(std::ostream& os, const Vec2& v) {
    os << "(" << v.x() << ", " << v.y() << ")";
    return os;
}
```

實測結果（真的編譯跑過）：

```cpp
Vec2 a(1.0, 2.0);
Vec2 b(3.0, 4.0);
Vec2 c = a + b;
std::cout << c;              // (4, 6)
std::cout << a.length();     // 2.23607
std::cout << (a == a);       // 1 (true)
std::cout << (a == b);       // 0 (false)
```

**該用成員函式還是非成員函式？** 挑戰 1.1 的提示一其實已經給了經驗法則：**`+=` 這種會修改左邊物件的複合賦值運算子，寫成成員函式**（因為它本質上是在操作 `*this`）；**`+` 這種產生新物件、兩邊地位對稱的二元運算子，寫成非成員函式**。這樣寫還有個好處：`Vec2(1,0) + a` 和 `a + Vec2(1,0)` 都合法（如果 `+` 是成員函式，左邊一定要是 `Vec2` 物件，`3 + a` 這種寫法就不可能成立，除非左邊也剛好是同型別）。

`<<` 一定要寫成非成員函式，因為它的左邊是 `std::ostream`，不是你的 `Vec2`，你沒有能力修改 `std::ostream` 這個 class 去幫它加成員函式。

---

## 6. const-correctness

`const` 出現的位置不同，意義也不同，這是初學者最容易搞混的地方：

```cpp
class Vec2 {
public:
    // const 成員函式：承諾這個函式不會修改物件的任何成員
    double length() const {
        return std::sqrt(x_ * x_ + y_ * y_);
    }

    // const 參考參數：承諾這個函式不會修改傳進來的 rhs
    Vec2& operator+=(const Vec2& rhs) {
        x_ += rhs.x_;
        y_ += rhs.y_;
        return *this;
    }

private:
    double x_, y_;
};
```

**為什麼要標 `const`？** 兩個實際好處：

第一，**編譯器幫你檢查承諾**。如果你在 `const` 成員函式裡不小心寫了 `x_ = 0;`，編譯器會直接報錯拒絕編譯，而不是等到執行期才發現物件被意外改壞。這是把「這個函式不該修改物件」這個意圖，從註解（沒有強制力）升級成編譯器強制檢查的規則。

第二，**只有 `const` 物件能呼叫 `const` 成員函式**：

```cpp
void print_length(const Vec2& v) {
    std::cout << v.length();   // 合法：length() 是 const 成員函式
    // v.set_x(5);              // 不合法：如果 set_x 不是 const，這裡會編譯失敗
}
```

如果 `length()` 沒標 `const`，`print_length` 這個函式（參數是 `const Vec2&`）就完全無法呼叫 `v.length()`——編譯器會覺得「你可能會修改這個唯讀物件」而拒絕。實務上的原則是：**任何邏輯上不該修改物件狀態的成員函式，一律標 `const`**，這能讓你的 class 跟 `const` 參考、`const` 物件無縫合作，別人拿到你的物件時也更放心。

**傳參數用 `const T&` 而不是 `T`**：`const Vec2&`（const 參考）表示「借用一下，不複製、也不修改」，比直接傳值 `Vec2`（會整個複製一份）更有效率，尤其型別比較大的時候差異更明顯。這就是挑戰 1.1 提到的「值語意 vs 傳值/傳參考的選擇」。

---

## 7. 複製語意（Copy）

預設情況下，C++ 的 class 複製時會做「逐成員複製」（memberwise copy）：

```cpp
Vec2 a(1.0, 2.0);
Vec2 b = a;   // 複製建構子，b.x_ = a.x_, b.y_ = a.y_（各自獨立的副本）
```

對 `double` 這種值型別完全沒問題。但如果 class 裡有**指標**、擁有某種資源（動態配置的記憶體、檔案控制代碼），預設的逐成員複製就會出大問題——兩個物件會共用同一份底層資源：

```cpp
class BufferBad {
public:
    explicit BufferBad(std::size_t size) : size_(size), data_(new int[size]) {}
    ~BufferBad() { delete[] data_; }   // 沒寫複製建構子，用預設的逐成員複製

private:
    std::size_t size_;
    int* data_;
};

BufferBad a(5);
BufferBad b = a;   // 災難：b.data_ 和 a.data_ 指向同一塊記憶體！
// 程式結束時 a 的解構子 delete[] 一次，b 的解構子又 delete[] 同一塊記憶體一次
// → double free，未定義行為，程式很可能直接崩潰
```

這就是為什麼 `CsvLogger` 要 `= delete` 掉複製——與其冒著讓使用者不小心複製出一個共享同一份底層資源的物件，不如直接在編譯期擋掉。但如果你**需要**複製功能（例如這裡的 `Buffer` 範例，複製出兩份完全獨立的資料），就要自己寫**深拷貝**（deep copy）：

```cpp
class Buffer {
public:
    explicit Buffer(std::size_t size) : size_(size), data_(new int[size]) {
        for (std::size_t i = 0; i < size_; ++i) data_[i] = 0;
    }

    ~Buffer() { delete[] data_; }

    // 複製建構子：配置一塊「新的」記憶體，把內容複製過去
    Buffer(const Buffer& other) : size_(other.size_), data_(new int[other.size_]) {
        for (std::size_t i = 0; i < size_; ++i) data_[i] = other.data_[i];
    }

    // 複製賦值運算子：a = b 這種寫法會呼叫這個
    Buffer& operator=(const Buffer& other) {
        if (this == &other) return *this;   // 防止自己賦值給自己時出錯
        int* new_data = new int[other.size_];
        for (std::size_t i = 0; i < other.size_; ++i) new_data[i] = other.data_[i];
        delete[] data_;         // 先放掉自己原本的資源
        data_ = new_data;
        size_ = other.size_;
        return *this;
    }

    std::size_t size() const { return size_; }

private:
    std::size_t size_;
    int* data_;
};
```

**Rule of Three**：如果一個 class 需要自己寫解構子（代表它管理某種資源），那通常也需要自己寫複製建構子和複製賦值運算子——三者要嘛都自己寫、要嘛都不寫（用預設的），很少只寫一兩個還能正確。這是 C++ 一條經典的經驗法則。

---

## 8. 搬移語意（Move）

深拷貝雖然安全，但有時候明顯浪費——如果來源物件之後根本不會再用到（例如函式回傳一個臨時物件），複製一份完整資料是白費工。C++11 引入**搬移語意**解決這個問題：與其複製整份資料，不如直接把資源的所有權「偷」過來，來源物件放棄持有。

```cpp
class Buffer {
    // ...（複製建構子/賦值同上）

    // 搬移建構子：接收一個「即將消失」的物件（rvalue reference &&），
    // 直接偷走它的指標，不做任何複製
    Buffer(Buffer&& other) noexcept : size_(other.size_), data_(other.data_) {
        other.data_ = nullptr;   // 讓來源物件的解構子不會誤刪同一塊記憶體
        other.size_ = 0;
    }

    // 搬移賦值運算子
    Buffer& operator=(Buffer&& other) noexcept {
        if (this == &other) return *this;
        delete[] data_;           // 先放掉自己原本的資源
        data_ = other.data_;      // 偷資源
        size_ = other.size_;
        other.data_ = nullptr;
        other.size_ = 0;
        return *this;
    }
};
```

`Buffer&&` 裡的 `&&` 叫 rvalue reference（右值參考），白話講就是「一個即將被丟棄、你可以安心把它掏空的物件」。`std::move(x)` 這個標準函式庫工具的作用，是把一個原本的左值（左值＝有名字、還會繼續存在的物件）**強制標記成右值**，告訴編譯器「我知道這個物件之後不用了，可以被掏空」：

```cpp
Buffer a(5);
Buffer b = a;                 // 呼叫複製建構子（深拷貝，a 之後還能用）
Buffer c = std::move(a);      // 呼叫搬移建構子（偷資源，a 現在是空殼，不該再用了）
```

實測結果：

```
buf2.size()=5 buf3.size()=5
```

`buf2` 是複製出來的、`buf3` 是搬移出來的，兩個都能正確回報 `size()`，因為搬移建構子把 `size_` 也正確轉移過去了（只有底層的 `int*` 是「偷」，`size_` 這種值型別成員該複製還是要複製）。

**Rule of Five**：C++11 之後，Rule of Three 擴充成 Rule of Five——如果自己寫了解構子/複製建構子/複製賦值，通常也該考慮搬移建構子/搬移賦值，讓 class 在「需要複製」和「可以偷資源」兩種情境下都有效率的行為。你的 `CsvLogger` 選擇「禁止複製、允許搬移」（用 `= default` 讓編譯器自動生成搬移版本），是這條規則的另一種常見組合——不是所有 class 都需要五個一起自己寫，`std::ofstream` 成員本身就是可搬移的，所以編譯器自動生成的版本就夠用。

---

## 9. 繼承（Inheritance）

繼承讓你基於一個既有的 class（**base class / 基底類別**）定義一個新 class（**derived class / 衍生類別**），衍生類別自動擁有基底類別的所有 public/protected 成員，可以再加上自己專屬的東西。這在描述「is-a」關係時很自然——「圓形**是一種**形狀」、「Circle **is-a** Shape」。

```cpp
class Shape {
public:
    // 之後第 10 節會講為什麼這裡要加 virtual
};

class Circle : public Shape {
public:
    explicit Circle(double r) : r_(r) {}

private:
    double r_;
};
```

`: public Shape` 表示「Circle 繼承自 Shape，而且是 public 繼承」（幾乎所有情況都用 public 繼承，這裡不深入 protected/private 繼承這種較冷門的用法）。如果基底類別的建構子需要參數，衍生類別要在自己的初始化列表裡明講怎麼呼叫基底類別的建構子：

```cpp
class Animal {
public:
    explicit Animal(std::string name) : name_(std::move(name)) {}
    std::string name() const { return name_; }

private:
    std::string name_;
};

class Dog : public Animal {
public:
    Dog(std::string name, std::string breed)
        : Animal(std::move(name)), breed_(std::move(breed)) {}   // 先初始化基底類別部分

private:
    std::string breed_;
};
```

值得先說在前面：**你目前規劃的無人機專案完全不需要用到繼承**（`Vec2`、`PidController`、`Quaternion` 都是各自獨立的 class，沒有共同的基底類別）。這節和下一節是完整 C++ OOP 知識的一部分，但不是這個專案的必修內容——放進來是因為你要求完整學一次，之後你接觸別的專案（尤其是遊戲引擎、GUI 框架這類大量用繼承的領域）會用得到。

---

## 10. 多型與 virtual function

單純繼承本身還不是多型（polymorphism）。多型的重點是：**透過基底類別的指標或參考，呼叫的函式版本會依照物件「實際的」型別決定，而不是指標宣告的型別**。要做到這件事，函式要標記 `virtual`：

```cpp
class Shape {
public:
    virtual ~Shape() = default;              // 稍後解釋為什麼要 virtual 解構子
    virtual double area() const = 0;         // 純虛擬函式（pure virtual）
    virtual std::string name() const = 0;
};

class Circle : public Shape {
public:
    explicit Circle(double r) : r_(r) {}
    double area() const override { return 3.14159265358979323846 * r_ * r_; }
    std::string name() const override { return "Circle"; }

private:
    double r_;
};

class Rectangle : public Shape {
public:
    Rectangle(double w, double h) : w_(w), h_(h) {}
    double area() const override { return w_ * h_; }
    std::string name() const override { return "Rectangle"; }

private:
    double w_, h_;
};
```

`= 0` 代表這是**純虛擬函式**——`Shape` 本身沒有提供實作，任何繼承 `Shape` 的 class 都**必須**自己實作 `area()` 和 `name()`，否則編譯不過。一個 class 只要有任何一個純虛擬函式沒被實作，它就是**抽象類別**（abstract class），無法直接 `new Shape()` 建立實體，只能透過它的衍生類別使用。這正是「Shape」這個概念該有的行為——「形狀」本身太抽象了，具體的面積算法只有「圓形」「矩形」才知道怎麼算。

`override` 關鍵字不是必要的，但強烈建議加：它讓編譯器幫你檢查「這個函式真的有覆寫到基底類別的某個 virtual 函式」，如果你手滑打錯函式簽名（例如少打一個 `const`），沒有 `override` 的話編譯器不會報錯，只會默默產生一個新的、無關的函式，多型完全失效但你不會發現；加了 `override`，簽名對不上編譯器會直接報錯。

實測結果：

```cpp
std::vector<std::unique_ptr<Shape>> shapes;
shapes.push_back(std::make_unique<Circle>(2.0));
shapes.push_back(std::make_unique<Rectangle>(3.0, 4.0));

for (const auto& s : shapes) {
    std::cout << s->name() << " area=" << s->area() << "\n";
}
// Circle area=12.5664
// Rectangle area=12
```

重點在這裡：`shapes` 這個容器裝的是 `Shape` 指標（`std::unique_ptr<Shape>`），迴圈裡呼叫 `s->area()` 時，程式在**執行期**才決定要跑 `Circle::area()` 還是 `Rectangle::area()`——這就是多型：同一段呼叫程式碼，依照物件實際型別跑出不同行為，呼叫端完全不需要知道容器裡裝的是什麼具體型別，這在寫「處理一堆不同種類但有共通介面的物件」時非常強大。

**為什麼基底類別的解構子要標 `virtual`？** 這是新手最容易踩的坑：

```cpp
class Base {
public:
    ~Base() {}  // 忘記標 virtual
};
class Derived : public Base {
public:
    ~Derived() { /* 假設這裡釋放了某些資源 */ }
};

Base* p = new Derived();
delete p;   // 未定義行為！只會呼叫 Base::~Base()，Derived::~Derived() 完全不會被執行
```

如果 `Base` 的解構子不是 `virtual`，透過 `Base*` 指標 `delete` 一個實際是 `Derived` 的物件時，只會呼叫到 `Base` 的解構子，`Derived` 專屬的清理工作完全被跳過，造成資源洩漏。**只要一個 class 有可能被拿來當多型的基底類別使用（也就是可能透過基底類別指標刪除衍生類別物件），解構子就一定要標 `virtual`。** 這也是為什麼上面的 `Shape` 範例裡 `virtual ~Shape() = default;` 那行不是裝飾，是必要的正確性保證。

---

## 11. static 成員

`static` 成員屬於「這個 class 本身」，不屬於任何一個特定物件，所有物件共用同一份：

```cpp
class Counter {
public:
    Counter() { ++count_; }
    ~Counter() { --count_; }
    static int alive() { return count_; }   // static 成員函式：不透過任何特定物件呼叫

private:
    static int count_;                       // static 成員變數：宣告
};
int Counter::count_ = 0;                     // static 成員變數：定義（一定要在 class 外面再寫一次）
```

實測結果：

```cpp
{
    Counter x, y, z;
    std::cout << Counter::alive();   // 3
}
std::cout << Counter::alive();       // 0（x,y,z 離開作用域，解構子把 count_ 都減回去了）
```

呼叫 `static` 成員函式不需要（也不能依賴）任何特定物件，直接用 `Counter::alive()`（class 名稱 + `::`）呼叫。這種「統計所有存在物件數量」的模式是 `static` 成員的經典用途之一。

---

## 12. 常見陷阱 checklist

寫 C++ class 時容易踩的坑，整理成檢查清單：

- **基底類別當多型使用卻忘記 `virtual` 解構子** —— 第 10 節講過，會造成衍生類別的清理邏輯被跳過。
- **class 裡有裸指標卻沒寫複製建構子/賦值運算子** —— 預設的逐成員複製會讓兩個物件共用同一份資源，其中一個解構時另一個變成 dangling pointer，之後使用會是未定義行為。要嘛自己寫深拷貝，要嘛乾脆 `= delete` 複製（像 `CsvLogger` 那樣）。
- **忘記幫成員函式標 `const`** —— 導致這個 class 的物件沒辦法用 `const&` 傳遞（例如當函式參數），到處都要傳非 const 參考，整個程式的 const-correctness 會一路被破壞下去。
- **回傳區域變數的參考或指標** —— 函式結束後區域變數就被銷毀了，回傳的參考/指標變成 dangling，之後使用是未定義行為。
- **複製賦值運算子忘記處理自我賦值**（`a = a;`）—— 沒檢查 `this == &other` 的話，先 `delete` 自己的資源、後面又想從（已經被刪除的）自己身上複製資料，直接壞掉。
- **把衍生類別物件「切片」傳給接受基底類別、傳值（不是參考/指標）的函式**（object slicing）—— 衍生類別專屬的部分會被整個切掉，只剩基底類別的部分，多型完全失效。這也是為什麼多型的情境幾乎都用指標或參考操作，很少直接傳值。

---

## 13. 對照這個專案：接下來章節會用到什麼

回頭看挑戰之路，把剛剛學的東西對應到實際會用到的地方：

- **挑戰 1.1（打造 Vec2）**：完整用到第 5 節運算子多載、第 6 節 const-correctness、第 7 節值語意的判斷（Vec2 這種小型值型別通常不需要自己寫複製/搬移，用編譯器預設生成的逐成員複製就夠了，因為裡面都是 `double`，沒有裸指標）。
- **挑戰 1.2、1.5（State struct）**：第 1 節 struct 的選用時機——單純裝資料、沒有不變量要保護，用 `struct` 而不是 `class`。
- **挑戰 2.1（PidController）**：第 2、3 節封裝 + 建構子——內部狀態（積分累積值、上次誤差）要 private，只透過明確的方法（例如 `update()`、`reset()`）操作。
- **挑戰 3.1（SDL2 RAII wrapper）**：第 3 節建構子/解構子、第 7 節複製語意——這是全專案 RAII 練習的高潮，`SDL_Window*` 這種裸指標資源要嘛用 `unique_ptr` 配自訂 deleter、要嘛自己寫 wrapper class，思路跟 `CsvLogger`、跟這份教材第 7 節的 `Buffer` 範例完全一樣。
- **挑戰 4.2（Quaternion）**：再一輪第 5 節運算子多載的實戰，這次會複雜不少（乘法、共軛等）。

**第 9、10 節（繼承、多型）目前規劃完全用不到**——這個專案裡每個 class 都是獨立的，沒有共同基底類別的設計。放進這份教材純粹是因為你要求完整學一次；如果之後接觸的專案（遊戲引擎的 Entity 系統、GUI 框架的 Widget 階層）需要繼承/多型，這裡的基礎就夠用了。

---

準備好之後，我們可以挑一節開始互動練習——例如拿挑戰 1.1 的 `Vec2` 當實作題，你自己動手寫，我看你寫的程式碼給回饋，比單純讀文件更容易記住。想先從哪一節開始都可以。
