#include <cmath>
#include <iostream>
#include <vector>

#include "csv_logger.hpp"
#include "physics/particle.hpp"
#include "physics/state.hpp"
#include "math/RK4.hpp"

void run_free_fall_comparison();
void run_rotational_dynamics_comparison();
void run_two_motor();

int main() {

    // run_free_fall_comparison();
    // run_rotational_dynamics_comparison();
    run_two_motor();
    return 0;
}

void run_free_fall_comparison() {
    const double g = -9.8;
    const double total_time = 3.0;
    const vec2 initial_position(0.0, 0.0);
    const vec2 initial_velocity(1.0, 0.0);
 
    const auto free_fall_derivative = [g](double /*t*/, State s) -> State {
        // (位置的變化率, 速度的變化率) = (速度, 重力加速度)
        return State{s.velocity, vec2(0.0, g)};
    };
 
    const std::vector<double> dts = {0.1, 0.05, 0.01, 0.005, 0.001, 0.0005, 0.0001};
 
    CSVLogger logger("output/free_fall_euler_vs_rk4.csv",
                      {"dt", "steps", "actual_time", "euler_error", "rk4_error"});
 
    for (double dt : dts) {
        const int steps = static_cast<int>(std::round(total_time / dt));
        const double actual_time = steps * dt;
        const double y_analytical =
            analytical_y(initial_position.y(), initial_velocity.y(), g, actual_time);
 
        // Euler：沿用 1.2/1.3 現成的實作。
        const Particle euler_final =
            simulate_free_fall(initial_position, initial_velocity, g, dt, steps);
        const double euler_error = std::abs(euler_final.position.y() - y_analytical);
 
        // RK4：用通用模板 + State，一次把位置、速度一起積分。
        State state{initial_position, initial_velocity};
        for (int i = 0; i < steps; ++i) {
            state = integrate(free_fall_derivative, state, i * dt, dt);
        }
        const double rk4_error = std::abs(state.position.y() - y_analytical);
 
        logger.log({dt, static_cast<double>(steps), actual_time, euler_error, rk4_error});
 
        std::cout << "[free fall] dt=" << dt << "\teuler_error=" << euler_error
                  << "\trk4_error=" << rk4_error << "\n";
    }
 
    std::cout << "wrote output/free_fall_euler_vs_rk4.csv\n";
}

void run_rotational_dynamics_comparison() {
    const double g = -9.8;
    const double alpha = 0.1; // 假設的角加速度
    const double total_time = 3.0;
    const vec2 initial_position(0.0, 0.0);
    const vec2 initial_velocity(1.0, 0.0);
    const double initial_angle = 0.0;
    const double initial_omega = 1.0; // 初始角速度
 
    const auto rotational_derivative = [g, alpha](double /*t*/, State s) -> State {
        // (位置的變化率, 速度的變化率, 角度的變化率, 角速度的變化率)
        return State{s.velocity, vec2(0.0, g), s.omega, alpha};
    };
 
    const std::vector<double> dts = {0.1, 0.05, 0.01, 0.005, 0.001};
 
    CSVLogger logger("output/rotational_dynamics_rk4.csv",
                      {"dt", "steps", "actual_time", "final_x", "final_y", "final_angle"});
 
    for (double dt : dts) {
        const int steps = static_cast<int>(std::round(total_time / dt));
        const double actual_time = steps * dt;
 
        // RK4：用通用模板 + State，一次把位置、速度、角度、角速度一起積分。
        State state{initial_position, initial_velocity, initial_angle, initial_omega};
        for (int i = 0; i < steps; ++i) {
            state = integrate(rotational_derivative, state, i * dt, dt);
        }
 
        logger.log({dt, static_cast<double>(steps), actual_time,
                    state.position.x(), state.position.y(), state.angle});
 
        std::cout << "[rotational dynamics] dt=" << dt
                  << "\tfinal_position=(" << state.position.x() << ", " << state.position.y() << ")"
                  << "\tfinal_angle=" << state.angle << "\n";
    }
 
    std::cout << "wrote output/rotational_dynamics_rk4.csv\n";
}

void run_two_motor() {
    // 這裡可以加入兩個馬達的模擬，使用 RK4 積分器來計算位置、速度、角度和角速度的變化。
    // 例如，假設有兩個馬達施加不同的力矩，可以定義一個新的 derivative 函式來描述系統的動態。

    const double L = 2.0; // 馬達與中心點的距離
    const vec2 f_left(0.0, 10.2); // 左馬達的力
    const vec2 f_right(0.0, 10.0); // 右馬達的力
    const double I = 2.0; // 假設的轉動慣量
    const double mass = 2.0; // 假設的質量
    const double g = -9.8; // 重力加速度
    const vec2 initial_position(0.0, 0.0);
    const vec2 initial_velocity(0.0, 0.0);
    const double initial_angle = 0.0;
    const double initial_omega = 0.0; // 初始角速度
    const double total_time = 10.0;

    const double torque = (f_right.y() - f_left.y()) * L; // 力矩 τ = (f_right - f_left) * L

    const auto rotational_derivative = [g, I, mass, torque, f_left, f_right](double t, State s) -> State {
        // (位置的變化率, 速度的變化率, 角度的變化率, 角速度的變化率)
        vec2 acceleration = vec2(0.0, g) + (f_right + f_left).rotate(s.angle) / mass; // 垂直方向的加速度
        return State{s.velocity, acceleration, s.omega, torque / I};
    };

    CSVLogger logger("output/two_motor_different_long_time.csv",
                      {"time", "x", "y", "angle"});

    State state{initial_position, initial_velocity, initial_angle, initial_omega};
    for (double t = 0; t < total_time; t += 0.01) {
        state = integrate(rotational_derivative, state, t, 0.01);
        logger.log({t, state.position.x(), state.position.y(), state.angle});
    }

}