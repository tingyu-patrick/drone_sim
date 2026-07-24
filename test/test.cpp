#include <catch2/catch_test_macros.hpp>
#include <cmath>

#include "../src/math/vec2.hpp"
#include "../src/physics/state.hpp"
#include "../src/math/RK4.hpp"
#include "../src/math/PIDControler.hpp"

const double PI = std::acos(-1.0);

TEST_CASE("addition works", "[basic]") {
    REQUIRE(1 + 1 == 2);
}

// ---- vec2：建構與存取 ----

TEST_CASE("default constructor gives zero vector", "[vec2]") {
    vec2 v;
    REQUIRE(v == vec2(0.0, 0.0));
}

TEST_CASE("constructor and getters", "[vec2]") {
    vec2 v(3.0, 4.0);
    REQUIRE(v.x() == 3.0);
    REQUIRE(v.y() == 4.0);
}

TEST_CASE("setters modify components", "[vec2]") {
    vec2 v;
    v.set_x(1.5);
    v.set_y(-2.5);
    REQUIRE(v == vec2(1.5, -2.5));
}

// ---- 算術運算 ----

TEST_CASE("operator+= accumulates in place", "[vec2]") {
    vec2 v(1.0, 2.0);
    v += vec2(3.0, 4.0);
    REQUIRE(v == vec2(4.0, 6.0));
}

TEST_CASE("operator+ does not modify operands", "[vec2]") {
    const vec2 a(1.0, 2.0);
    const vec2 b(3.0, 4.0);
    vec2 c = a + b;
    REQUIRE(c == vec2(4.0, 6.0));
    REQUIRE(a == vec2(1.0, 2.0));
    REQUIRE(b == vec2(3.0, 4.0));
}

TEST_CASE("operator-= and operator-", "[vec2]") {
    vec2 v(5.0, 7.0);
    v -= vec2(3.0, 4.0);
    REQUIRE(v == vec2(2.0, 3.0));

    vec2 a(5.0, 7.0);
    vec2 b(3.0, 4.0);
    REQUIRE((a - b) == vec2(2.0, 3.0));
}

TEST_CASE("scalar multiplication works both directions", "[vec2]") {
    vec2 v(2.0, 3.0);
    REQUIRE((v * 2.0) == vec2(4.0, 6.0));
    REQUIRE((2.0 * v) == vec2(4.0, 6.0));
}

TEST_CASE("operator/= and operator/ divide by scalar", "[vec2]") {
    vec2 v(4.0, 6.0);
    v /= 2.0;
    REQUIRE(v == vec2(2.0, 3.0));

    vec2 a(4.0, 6.0);
    REQUIRE((a / 2.0) == vec2(2.0, 3.0));
}

// ---- 向量運算 ----

TEST_CASE("dot product", "[vec2]") {
    vec2 v1(1.0, 2.0);
    vec2 v2(3.0, 4.0);
    REQUIRE(v1.dot(v2) == 11.0);
}

TEST_CASE("length of a 3-4-5 vector", "[vec2]") {
    vec2 v(3.0, 4.0);
    REQUIRE(v.length() == 5.0);
}

TEST_CASE("normalized returns unit vector, original unchanged", "[vec2]") {
    const vec2 v(3.0, 4.0);
    vec2 n = v.normalized();

    REQUIRE(n == vec2(0.6, 0.8));
    REQUIRE(v == vec2(3.0, 4.0));  // v 本身沒被動到
    REQUIRE(std::abs(n.length() - 1.0) < 1e-6);
}

TEST_CASE("normalize mutates the vector in place", "[vec2]") {
    vec2 v(3.0, 4.0);
    v.normalize();
    REQUIRE(v == vec2(0.6, 0.8));
}

TEST_CASE("normalizing the zero vector does not crash or produce NaN", "[vec2]") {
    vec2 v(0.0, 0.0);
    vec2 n = v.normalized();
    REQUIRE(n == vec2(0.0, 0.0));

    v.normalize();
    REQUIRE(v == vec2(0.0, 0.0));
}

// ---- 比較 ----

TEST_CASE("operator==", "[vec2]") {
    REQUIRE(vec2(1.0, 2.0) == vec2(1.0, 2.0));
}

TEST_CASE("operator== tolerates tiny floating point error", "[vec2]") {
    vec2 a(0.1 + 0.2, 1.0);  // 經典的浮點數誤差案例，0.1+0.2 != 0.3（精確比較）
    vec2 b(0.3, 1.0);
    REQUIRE(a == b);
}

// ==================================================================
// 挑戰 1.5：會轉的箱子
//
// State 從「位置+速度」擴充為「位置+速度+角度(angle)+角速度(omega)」。
// 這裡分三層驗證：
//   1. State 本身的建構與 operator+/operator* 對新欄位是否正確運作。
//   2. 回歸測試——擴充後，舊的 1.2/1.4 自由落體行為不能被破壞。
//   3. 驗收條件本體——固定力矩下 θ(t) = ½(τ/I)t²，以及無力矩時 ω 恆定。
// ==================================================================

// ---- State：建構與算子（新欄位 angle / omega）----

TEST_CASE("State aggregate initialization sets all four fields", "[state]") {
    State s{vec2(1.0, 2.0), vec2(3.0, 4.0), 0.5, 1.5};
    REQUIRE(s.position == vec2(1.0, 2.0));
    REQUIRE(s.velocity == vec2(3.0, 4.0));
    REQUIRE(s.angle == 0.5);
    REQUIRE(s.omega == 1.5);
}

TEST_CASE("State operator+ adds every field component-wise", "[state]") {
    State a{vec2(1.0, 1.0), vec2(0.0, 1.0), 0.1, 0.2};
    State b{vec2(2.0, 3.0), vec2(1.0, 0.0), 0.3, 0.4};

    State c = a + b;
    REQUIRE(c.position == vec2(3.0, 4.0));
    REQUIRE(c.velocity == vec2(1.0, 1.0));
    REQUIRE(std::abs(c.angle - 0.4) < 1e-9);
    REQUIRE(std::abs(c.omega - 0.6) < 1e-9);

    // 確認是不可變運算——原本的 a、b 不該被動到。
    REQUIRE(a.angle == 0.1);
    REQUIRE(b.angle == 0.3);
}

TEST_CASE("State operator* scales every field", "[state]") {
    State s{vec2(1.0, 2.0), vec2(3.0, 4.0), 2.0, 4.0};
    State scaled = s * 2.0;

    REQUIRE(scaled.position == vec2(2.0, 4.0));
    REQUIRE(scaled.velocity == vec2(6.0, 8.0));
    REQUIRE(scaled.angle == 4.0);
    REQUIRE(scaled.omega == 8.0);
}

TEST_CASE("scalar * State matches State * scalar (both directions defined)", "[state]") {
    State s{vec2(1.0, 0.0), vec2(0.0, 1.0), 1.0, -1.0};

    State a = s * 3.0;
    State b = 3.0 * s;

    REQUIRE(a.position == b.position);
    REQUIRE(a.velocity == b.velocity);
    REQUIRE(a.angle == b.angle);
    REQUIRE(a.omega == b.omega);
}

// ---- 回歸測試：擴充 State 後，1.2/1.4 的自由落體不能壞掉 ----

TEST_CASE("regression: free fall via generic integrate() still matches the analytical solution after extending State", "[state][regression]") {
    const double g = -9.8;
    const double dt = 0.01;
    const int steps = 300;  // 模擬 3 秒
    const vec2 initial_position(0.0, 0.0);
    const vec2 initial_velocity(1.0, 0.0);

    // 力矩、角速度全程不參與——這是在確認新欄位「不用就是 0」，不會悄悄污染舊行為。
    const auto free_fall_derivative = [g](double /*t*/, State s) -> State {
        return State{s.velocity, vec2(0.0, g), s.omega, 0.0};
    };

    State state{initial_position, initial_velocity, 0.0, 0.0};
    for (int i = 0; i < steps; ++i) {
        state = integrate(free_fall_derivative, state, i * dt, dt);
    }

    const double actual_time = steps * dt;
    const double y_analytical = initial_position.y() + initial_velocity.y() * actual_time
        + 0.5 * g * actual_time * actual_time;

    REQUIRE(std::abs(state.position.y() - y_analytical) < 1e-6);
    REQUIRE(state.angle == 0.0);  // 沒有 omega、沒有力矩，角度應該完全不動
    REQUIRE(state.omega == 0.0);
}

// ---- 挑戰 1.5 驗收條件本體 ----
//
// 把剛體簡化成純轉動：position/velocity 全程維持 0，只讓 angle/omega
// 參與運算，這樣測試才能單獨聚焦在轉動方程 α = τ/I 上，不被平移運動干擾。

TEST_CASE("constant torque produces the rotational parabola theta(t) = 1/2 * (tau/I) * t^2", "[state][rigid_body]") {
    const double torque = 1.5;             // τ
    const double moment_of_inertia = 2.0;  // I，刻意選一個不是 1 的值，避免巧合掩蓋 bug
    const double dt = 0.01;
    const int steps = 300;  // 模擬 3 秒

    const auto constant_torque_derivative = [=](double /*t*/, State s) -> State {
        const double angular_acceleration = torque / moment_of_inertia;
        return State{vec2(0.0, 0.0), vec2(0.0, 0.0), s.omega, angular_acceleration};
    };

    State state{vec2(0.0, 0.0), vec2(0.0, 0.0), 0.0, 0.0};
    for (int i = 0; i < steps; ++i) {
        state = integrate(constant_torque_derivative, state, i * dt, dt);
    }

    const double t = steps * dt;
    const double angular_acceleration = torque / moment_of_inertia;
    const double theta_analytical = 0.5 * angular_acceleration * t * t;
    const double omega_analytical = angular_acceleration * t;

    REQUIRE(std::abs(state.angle - theta_analytical) < 1e-6);
    REQUIRE(std::abs(state.omega - omega_analytical) < 1e-6);
}

TEST_CASE("with zero torque, angular velocity stays constant and angle grows linearly", "[state][rigid_body]") {
    const double initial_omega = 2.0;
    const double dt = 0.01;
    const int steps = 500;

    const auto no_torque_derivative = [](double /*t*/, State s) -> State {
        return State{vec2(0.0, 0.0), vec2(0.0, 0.0), s.omega, 0.0};
    };

    State state{vec2(0.0, 0.0), vec2(0.0, 0.0), 0.0, initial_omega};
    for (int i = 0; i < steps; ++i) {
        state = integrate(no_torque_derivative, state, i * dt, dt);
        // 每一步都檢查，不是只看最後一刻——確保 ω 全程恆定，不是恰好頭尾湊巧相等。
        REQUIRE(std::abs(state.omega - initial_omega) < 1e-9);
    }

    const double t = steps * dt;
    const double theta_analytical = initial_omega * t;
    REQUIRE(std::abs(state.angle - theta_analytical) < 1e-6);
}

TEST_CASE("let vector rotate", "[vec2][rotation]"){
    vec2 v(1.0, 0.0);
    v.rotate(PI / 2); // 旋轉 90 度
    REQUIRE(std::abs(v.x()) < 1e-6);
    REQUIRE(std::abs(v.y() - 1.0) < 1e-6);

    v.rotate(-PI / 2); // 再旋轉 -90 度回到原位
    REQUIRE(std::abs(v.x() - 1.0) < 1e-6);
    REQUIRE(std::abs(v.y()) < 1e-6);

    v.rotate(PI); // 旋轉 180 度
    REQUIRE(std::abs(v.x() + 1.0) < 1e-6);
    REQUIRE(std::abs(v.y()) < 1e-6);
}
// ==================================================================
// 挑戰 2.1：PID 積木
//
// PIDController 只吃 (目標值, 當前值, dt)，吐控制輸出；內部封裝
// 積分累積值（integral_）與前一次誤差（previous_error_）。對應驗收
// 條件的三組核心測試：
//   1. 純 P：輸出 = Kp × 誤差。
//   2. 純 I：積分項隨恆定誤差線性累積。
//   3. 純 D：對階躍誤差的反應（含第一次呼叫的 derivative kick）。
// 另外補上 reset() 的黑盒驗證，以及三項合一的整合測試。
// ==================================================================

TEST_CASE("pure P control: output equals Kp times the current error", "[pid]") {
    PIDController pid(2.0, 0.0, 0.0);  // 只開 P

    const double output = pid.update(/*expect=*/10.0, /*actual=*/4.0, /*dt=*/0.1);
    REQUIRE(std::abs(output - 2.0 * 6.0) < 1e-9);
}

TEST_CASE("pure P control tracks error every call with no memory across calls", "[pid]") {
    PIDController pid(3.0, 0.0, 0.0);

    REQUIRE(std::abs(pid.update(10.0, 0.0, 0.1) - 3.0 * 10.0) < 1e-9);
    // 誤差變小，純 P 應該立刻跟著變小——不會被前一次呼叫殘留影響。
    REQUIRE(std::abs(pid.update(10.0, 8.0, 0.1) - 3.0 * 2.0) < 1e-9);
    REQUIRE(std::abs(pid.update(10.0, 10.0, 0.1) - 0.0) < 1e-9);
}

TEST_CASE("pure I control accumulates the integral linearly under constant error", "[pid]") {
    PIDController pid(0.0, 2.0, 0.0);  // 只開 I
    const double error = 5.0;          // expect - actual 全程固定
    const double dt = 0.1;

    for (int step = 1; step <= 5; ++step) {
        const double output = pid.update(error, 0.0, dt);
        const double expected_integral = error * dt * step;  // 恆定誤差 → 積分項對 t 線性成長
        REQUIRE(std::abs(output - 2.0 * expected_integral) < 1e-9);
    }
}

TEST_CASE("pure D control reacts to a step error then settles once the error stops changing", "[pid]") {
    PIDController pid(0.0, 0.0, 4.0);  // 只開 D
    const double dt = 0.1;

    // 第一次呼叫：誤差從 0（初始 previous_error_）階躍到 5，
    // derivative = (5 - 0) / dt——這是刻意留著的 derivative kick（見 2.1 提示二）。
    const double first_output = pid.update(5.0, 0.0, dt);
    REQUIRE(std::abs(first_output - 4.0 * (5.0 / dt)) < 1e-9);

    // 誤差維持不變 → 誤差變化率為 0 → D 輸出應該歸零。
    const double second_output = pid.update(5.0, 0.0, dt);
    REQUIRE(std::abs(second_output - 0.0) < 1e-9);

    // 誤差再次階躍（縮小 2）→ derivative = (3 - 5) / dt = -2 / dt。
    const double third_output = pid.update(3.0, 0.0, dt);
    REQUIRE(std::abs(third_output - 4.0 * (-2.0 / dt)) < 1e-9);
}

TEST_CASE("reset() clears integral and previous error back to a fresh controller's state", "[pid]") {
    PIDController pid(1.0, 1.0, 1.0);

    // 先跑幾步，讓 integral_ / previous_error_ 都累積出不為零的值。
    pid.update(10.0, 0.0, 0.1);
    pid.update(10.0, 2.0, 0.1);
    pid.update(10.0, 5.0, 0.1);

    pid.reset();

    PIDController fresh(1.0, 1.0, 1.0);
    // reset 過的 pid 和全新建構的 fresh，面對同一組輸入應該給出一模一樣的輸出——
    // 這是從外部黑盒驗證 private 狀態確實被清空的唯一方式。
    const double after_reset = pid.update(10.0, 3.0, 0.1);
    const double from_fresh = fresh.update(10.0, 3.0, 0.1);
    REQUIRE(std::abs(after_reset - from_fresh) < 1e-9);
}

TEST_CASE("full PID combines P + I + D and matches a hand-computed sequence", "[pid]") {
    PIDController pid(1.0, 0.5, 0.2);
    const double dt = 0.1;

    // 手算對照：error = expect - actual；integral 逐步累加 error*dt；
    // derivative = (error - previous_error) / dt。跟 PIDController::update 的
    // 實作逐行對應，用來抓「合起來三項有沒有互相干擾」這種整合層級的 bug。
    struct Step { double expect; double actual; };
    const Step steps[] = {{10.0, 0.0}, {10.0, 4.0}, {10.0, 7.0}};

    double integral = 0.0;
    double previous_error = 0.0;

    for (const auto& step : steps) {
        const double error = step.expect - step.actual;
        integral += error * dt;
        const double derivative = (error - previous_error) / dt;
        const double expected_output = 1.0 * error + 0.5 * integral + 0.2 * derivative;
        previous_error = error;

        const double actual_output = pid.update(step.expect, step.actual, dt);
        REQUIRE(std::abs(actual_output - expected_output) < 1e-9);
    }
}
