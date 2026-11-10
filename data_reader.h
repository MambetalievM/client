#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>

typedef std::vector<std::vector<uint32_t>> VectorData;

class DataReader {
private:
    std::ifstream file;
public:
    explicit DataReader(const std::string& filename);
    VectorData readData();
};
