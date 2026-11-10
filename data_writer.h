#pragma once
#include <vector>
#include <string>
#include <cstdint>

class DataWriter {
private:
    std::string filename;
public:
    explicit DataWriter(const std::string& filename);
    void writeData(const std::vector<uint64_t>& results);
};
