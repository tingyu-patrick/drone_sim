#pragma once

class PIDController {
public:
    PIDController(double kp, double ki, double kd): 
        kp_(kp), ki_(ki), kd_(kd), integral_(0.0), previous_error_(0.0) {}

    double update(double expect, double actual, double dt) {
        double error = expect - actual;
        integral_ += error * dt;
        double derivative = (error - previous_error_) / dt;
        previous_error_ = error;

        return kp_ * error + ki_ * integral_ + kd_ * derivative;
    }

    void reset() {
        integral_ = 0.0;
        previous_error_ = 0.0;
    }


private:
    double kp_;
    double ki_;
    double kd_;
    double integral_;
    double previous_error_;
};