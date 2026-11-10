#include <iostream>
#include <string>
#include <memory>
#include "interface.h"
#include "data_reader.h"
#include "data_writer.h"
#include "communicator.h"
#include "errors.h"

int main(int argc, const char** argv) {
    try {
        Interface iface;
        Params params = iface.parse(argc, argv);

        if (params.showHelp) {
            std::cout << iface.getHelp() << std::endl;
            return 0;
        }

        // 1. Чтение исходных данных
        DataReader reader(params.inputFile);
        VectorData data = reader.readData();

        // 2. Взаимодействие с сервером (установка сеанса, аутентификация, обмен)
        Communicator comm(params.serverAddr, params.serverPort, params.configFile);
        std::vector<uint64_t> results = comm.sendAndReceive(data);

        // 3. Сохранение результатов
        DataWriter writer(params.outputFile);
        writer.writeData(results);

        std::cout << "Success: " << results.size() << " results saved to " << params.outputFile << std::endl;

    } catch (const ProgramError& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unknown error: " << e.what() << std::endl;
        return 2;
    }
    return 0;
}
