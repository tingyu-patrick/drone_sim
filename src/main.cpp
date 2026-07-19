#include <cmath>
#include <iostream>
#include "csv_logger.hpp"

int main() {
    std::cout << "Logging data to CSV...\n";

    std::vector<std::string> headers = {"X", "Y",};
    CSVLogger logger("output//drone_data.csv", headers);

    std::vector<double> data = {};

    std::cout << "Logging sine wave data...\n";

    for (double i = 0; i < 4*std::acos(-1.0); i += 0.001) {
        data = {i, std::sin(i)};
        logger.log(data);
    }

    std::cout << "logged!\n";

    return 0;
}