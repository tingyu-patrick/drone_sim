#pragma once

// 通用 RK4 積分器。
//
// 利用函式模板
//   1. State 可以是任何「支援 operator+ 和 operator*(scalar)」的型別——
//      vec2、下面的 State{position, velocity}、甚至一個裸的 double
//      （自由落體的另一種示範用方程式就會直接吃 double），這個函式完全
//      不用為了型別不同而改一行。
//   2. 模板在編譯期就能把 f 直接 inline
//      展開，沒有額外的間接呼叫成本。
//   3. 1.5 開始 State 會從「位置+速度」擴充到「位置+速度+角度+角速度」，
//      只要新的 State 也定義好 operator+/operator*，這個函式一行都不用改。
template <typename State, typename Derivative>
State integrate(Derivative f, State state, double t0, double dt) {
    const State k1 = f(t0, state);
    const State k2 = f(t0 + dt / 2.0, state + k1 * (dt / 2.0));
    const State k3 = f(t0 + dt / 2.0, state + k2 * (dt / 2.0));
    const State k4 = f(t0 + dt, state + k3 * dt);

    return state + (k1 + k2 * 2.0 + k3 * 2.0 + k4) * (dt / 6.0);
}