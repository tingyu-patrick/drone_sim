#pragma once

#include <fstream>
#include <stdexcept>
#include <vector>
#include <string>

class CSVLogger {
public:
    CSVLogger(const std::string& path, const std::vector<std::string>& headers)
     : file_(path) {
        if (!file_.is_open()) {
            throw std::runtime_error("Failed to open file: " + path);
        }
        // Write headers to the CSV file
        for (size_t i = 0; i < headers.size(); ++i) {
            file_ << headers[i];
            if (i < headers.size() - 1) {
                file_ << ",";
            }
        }
        file_ << std::endl;
    }

    ~CSVLogger() {
        if (file_.is_open()) {
            file_.close();
        }
    };

    CSVLogger(const CSVLogger&) = delete;
    CSVLogger& operator=(const CSVLogger&) = delete;

    void log(const std::vector<double>& data) {
        if (!file_.is_open()) return;
        for (size_t i = 0; i < data.size(); ++i) {
            file_ << data[i];
            if (i + 1 < data.size()) file_ << ",";
        }
        file_ << std::endl;
    }


private:
    std::ofstream file_;
};