#include <cmath>
#include <iostream>
#include <vector>

#include "csv_logger.hpp"
#include "physics/particle.hpp"

int main() {

    const double g = -9.8;
    const double total_time = 3.0;
    const vec2 initial_position(0.0, 0.0);
    const vec2 initial_velocity(1.0, 0.0);

    // 從粗到細，故意跨好幾個數量級，log-log 圖才看得出斜率。
    const std::vector<double> dts = {0.1, 0.05, 0.01, 0.005, 0.001, 0.0005, 0.0001};

    CSVLogger logger("output/error_vs_dt.csv",
                      {"dt", "steps", "actual_time", "numerical_y", "analytical_y", "abs_error"});

    for (double dt : dts) {
        // steps 用四捨五入取整數，並且用「實際模擬到的時間」(steps * dt)
        // 去跟解析解比較，而不是想當然爾用 total_time——
        // 這樣才能保證每一組 dt 模擬的總時長完全一致，不會因為除不盡而悄悄偏掉。
        const int steps = static_cast<int>(std::round(total_time / dt));
        const double actual_time = steps * dt;

        const Particle final_state =
            simulate_free_fall(initial_position, initial_velocity, g, dt, steps);

        const double y_numerical = final_state.position.y();
        const double y_analytical =
            analytical_y(initial_position.y(), initial_velocity.y(), g, actual_time);
        const double abs_error = std::abs(y_numerical - y_analytical);

        logger.log({dt, static_cast<double>(steps), actual_time, y_numerical, y_analytical, abs_error});

        std::cout << "dt=" << dt << "\tsteps=" << steps << "\terror=" << abs_error << "\n";
    }

    std::cout << "wrote output/error_vs_dt.csv\n";
    return 0;
}