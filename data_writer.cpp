#include "data_writer.h"
#include "errors.h"
#include <fstream>

DataWriter::DataWriter(const std::string& filename) : filename(filename) {}

void DataWriter::writeData(const std::vector<uint64_t>& results) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw IOError("Cannot open output file: " + filename);
    }

    // Первое поле: 4 байта (uint32_t) - количество результатов
    uint32_t count = results.size();
    file.write(reinterpret_cast<const char*>(&count), sizeof(count));

    // Следующие поля: по 8 байт (uint64_t) - сами результаты
    for (uint64_t res : results) {
        file.write(reinterpret_cast<const char*>(&res), sizeof(res));
    }

    if (!file.good()) {
        throw IOError("Error writing to output file: " + filename);
    }
}
