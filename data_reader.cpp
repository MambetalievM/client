#include "data_reader.h"
#include "errors.h"

DataReader::DataReader(const std::string& filename) {
    file.open(filename);
    if (!file.is_open()) {
        throw IOError("Cannot open input file: " + filename);
    }
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
}

VectorData DataReader::readData() {
    VectorData data;
    try {
        uint32_t numVectors;
        file >> numVectors;
        
        for (uint32_t i = 0; i < numVectors; ++i) {
            uint32_t vecSize;
            file >> vecSize;
            
            std::vector<uint32_t> vec(vecSize);
            for (uint32_t j = 0; j < vecSize; ++j) {
                file >> vec[j];
            }
            data.push_back(vec);
        }
    } catch (const std::ios_base::failure& e) {
        throw IOError("Error reading input file: " + std::string(e.what()));
    }
    return data;
}
