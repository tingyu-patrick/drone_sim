#pragma once

#include "../math/vec2.hpp"

// 自由落體用的最小質點：只有位置、速度。
// 沒有把 g 或 dt 藏在這個 struct 裡——它們是每次呼叫時傳進來的參數，
// 這樣同一個 Particle/step 邏輯才能被 1.3、1.4 反覆用不同 dt、不同 g 呼叫，
// 不用擔心殘留的全域狀態把上一次模擬的結果帶進這一次。
struct Particle {
    vec2 position;
    vec2 velocity;
};

// 單一步：semi-implicit Euler（先更新速度，再用新速度更新位置）。
inline void step_particle(Particle& p, double g, double dt) {
    p.velocity.set_y(p.velocity.y() + g * dt);

    p.position.set_x(p.position.x() + p.velocity.x() * dt);
    p.position.set_y(p.position.y() + p.velocity.y() * dt);
}

// 跑一整趟模擬：吃初始條件、g、dt、要跑幾步，回傳最終狀態。
// 純函式風格——參數進、結果出，內部不碰任何全域變數，
// 這是 1.3 能對同一組物理反覆跑不同 dt 的關鍵。
inline Particle simulate_free_fall(vec2 initial_position,
                                    vec2 initial_velocity,
                                    double g,
                                    double dt,
                                    int steps) {
    Particle p{initial_position, initial_velocity};
    for (int i = 0; i < steps; ++i) {
        step_particle(p, g, dt);
    }
    return p;
}

// 自由落體的解析解（y 方向）：y(t) = y0 + vy0*t + 1/2*g*t^2
// 這是「真正答案」，數值模擬的誤差就是拿這個當比較基準。
inline double analytical_y(double y0, double vy0, double g, double t) {
    return y0 + vy0 * t + 0.5 * g * t * t;
}