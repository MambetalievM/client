#include <UnitTest++/UnitTest++.h>
#include <fstream>
#include <cstdio>
#include <vector>
#include <string>
#include <iostream>

#include "data_reader.h"
#include "data_writer.h"
#include "interface.h"
#include "communicator.h"
#include "errors.h"

// 1. Тестирование DataReader
SUITE(DataReaderTest) 
{
    const std::string validFile = "test_valid_input.txt";
    const std::string invalidFile = "test_invalid_input.txt";

    void createFile(const std::string& filename, const std::string& content) {
        std::ofstream f(filename);
        f << content;
        f.close();
    }

    TEST(ReadValidFile) {
        std::cout << "[Test 1] DataReader: Чтение корректного файла" << std::endl;
        createFile(validFile, "2\n3\n10 20 30\n2\n40 50");
        DataReader reader(validFile);
        VectorData data = reader.readData();
        
        std::cout << "  Ожидается векторов: 2, Получено: " << data.size() << std::endl;
        CHECK_EQUAL(2, data.size());
        CHECK_EQUAL(3, data[0].size());
        CHECK_EQUAL(10, data[0][0]);
        CHECK_EQUAL(20, data[0][1]);
        CHECK_EQUAL(30, data[0][2]);
        CHECK_EQUAL(2, data[1].size());
        CHECK_EQUAL(40, data[1][0]);
        CHECK_EQUAL(50, data[1][1]);
        
        std::remove(validFile.c_str());
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(ReadZeroVectors) {
        std::cout << "[Test 2] DataReader: Чтение файла с 0 векторов" << std::endl;
        createFile(validFile, "0");
        DataReader reader(validFile);
        VectorData data = reader.readData();
        std::cout << "  Ожидается векторов: 0, Получено: " << data.size() << std::endl;
        CHECK_EQUAL(0, data.size());
        std::remove(validFile.c_str());
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(ReadEmptyVector) {
        std::cout << "[Test 3] DataReader: Чтение файла с пустым вектором" << std::endl;
        createFile(validFile, "1\n0");
        DataReader reader(validFile);
        VectorData data = reader.readData();
        std::cout << "  Ожидается векторов: 1, Получено: " << data.size() << std::endl;
        std::cout << "  Ожидается размер вектора: 0, Получено: " << data[0].size() << std::endl;
        CHECK_EQUAL(1, data.size());
        CHECK_EQUAL(0, data[0].size());
        std::remove(validFile.c_str());
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(ReadNonExistentFile) {
        std::cout << "[Test 4] DataReader: Обработка несуществующего файла" << std::endl;
        std::cout << "  Ожидается исключение IOError..." << std::endl;
        REQUIRE CHECK_THROW(DataReader("no_such_file.txt"), IOError);
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(ReadInvalidFormat) {
        std::cout << "[Test 5] DataReader: Обработка неверного формата файла" << std::endl;
        createFile(invalidFile, "hello world");
        DataReader reader(invalidFile);
        std::cout << "  Ожидается исключение IOError..." << std::endl;
        REQUIRE CHECK_THROW(reader.readData(), IOError);
        std::remove(invalidFile.c_str());
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(ReadTruncatedFile) {
        std::cout << "[Test 6] DataReader: Обработка обрезанного файла" << std::endl;
        createFile(invalidFile, "2\n3\n10 20"); // Заявлено 3 элемента, дано 2
        DataReader reader(invalidFile);
        std::cout << "  Ожидается исключение IOError..." << std::endl;
        REQUIRE CHECK_THROW(reader.readData(), IOError);
        std::remove(invalidFile.c_str());
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }
}

// 2. Тестирование DataWriter
SUITE(DataWriterTest)
{
    const std::string outFile = "test_output.bin";

    TEST(WriteValidData) {
        std::cout << "[Test 7] DataWriter: Запись корректных данных" << std::endl;
        std::vector<uint64_t> results = {60, 90, 120};
        DataWriter writer(outFile);
        writer.writeData(results);
        std::ifstream file(outFile, std::ios::binary | std::ios::ate);
        CHECK(file.is_open());
        std::cout << "  Ожидаемый размер файла: 28 байт, Получено: " << (int)file.tellg() << " байт" << std::endl;
        CHECK_EQUAL(28, file.tellg());
        file.close();
        std::remove(outFile.c_str());
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(WriteEmptyData) {
        std::cout << "[Test 8] DataWriter: Запись пустого вектора результатов" << std::endl;
        std::vector<uint64_t> results = {};
        DataWriter writer(outFile);
        writer.writeData(results);
        std::ifstream file(outFile, std::ios::binary | std::ios::ate);
        std::cout << "  Ожидаемый размер файла: 4 байта, Получено: " << (int)file.tellg() << " байт" << std::endl;
        CHECK_EQUAL(4, file.tellg());
        file.close();
        std::remove(outFile.c_str());
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(WriteToInvalidPath) {
        std::cout << "[Test 9] DataWriter: Обработка неверного пути для записи" << std::endl;
        DataWriter writer("/invalid_path/out.bin");
        std::vector<uint64_t> results = {1, 2};
        std::cout << "  Ожидается исключение IOError..." << std::endl;
        REQUIRE CHECK_THROW(writer.writeData(results), IOError);
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }
}

// 3. Тестирование Interface (Парсинг аргументов)
SUITE(InterfaceTest)
{
    TEST(ParseValidArguments) {
        std::cout << "[Test 10] Interface: Парсинг корректных аргументов" << std::endl;
        const char* argv[] = {"client", "-a", "127.0.0.1", "-p", "33333", "-i", "in.txt", "-o", "out.bin", "-c", "conf.txt"};
        Interface iface;
        Params params = iface.parse(11, argv);
        CHECK_EQUAL("127.0.0.1", params.serverAddr);
        CHECK_EQUAL(33333, params.serverPort);
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(DefaultPortValue) {
        std::cout << "[Test 11] Interface: Использование порта по умолчанию" << std::endl;
        const char* argv[] = {"client", "-a", "127.0.0.1", "-i", "in.txt", "-o", "out.bin"};
        Interface iface;
        Params params = iface.parse(7, argv);
        std::cout << "  Ожидаемый порт по умолчанию: 33333, Получено: " << params.serverPort << std::endl;
        CHECK_EQUAL(33333, params.serverPort);
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(ShowHelp) {
        std::cout << "[Test 12] Interface: Обработка флага -h" << std::endl;
        const char* argv[] = {"client", "-h"};
        Interface iface;
        Params params = iface.parse(2, argv);
        CHECK_EQUAL(true, params.showHelp);
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(MissingRequiredArguments) {
        std::cout << "[Test 13] Interface: Обработка отсутствия обязательных аргументов" << std::endl;
        const char* argv[] = {"client", "-i", "in.txt", "-o", "out.bin"};
        Interface iface;
        std::cout << "  Ожидается исключение InterfaceError..." << std::endl;
        REQUIRE CHECK_THROW(iface.parse(5, argv), InterfaceError);
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(InvalidPortFormat) {
        std::cout << "[Test 14] Interface: Обработка неверного формата порта" << std::endl;
        const char* argv[] = {"client", "-a", "127.0.0.1", "-p", "not_a_number", "-i", "in.txt", "-o", "out.bin"};
        Interface iface;
        std::cout << "  Ожидается исключение InterfaceError..." << std::endl;
        REQUIRE CHECK_THROW(iface.parse(9, argv), InterfaceError);
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(UnknownOption) {
        std::cout << "[Test 15] Interface: Обработка неизвестного флага" << std::endl;
        // Массив содержит 8 элементов!
        const char* argv[] = {"client", "-a", "127.0.0.1", "-z", "-i", "in.txt", "-o", "out.bin"};
        Interface iface;
        std::cout << "  Ожидается исключение InterfaceError..." << std::endl;
        // Передаем 8, а не 9!
        REQUIRE CHECK_THROW(iface.parse(8, argv), InterfaceError);
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }
}

// 4. Тестирование Communicator (Чтение конфига)
SUITE(CommunicatorTest)
{
    const std::string confFile = "test_config.txt";

    TEST(InvalidConfigEmpty) {
        std::cout << "[Test 16] Communicator: Чтение пустого конфигурационного файла" << std::endl;
        std::ofstream f(confFile); f.close();
        std::cout << "  Ожидается исключение AuthError..." << std::endl;
        REQUIRE CHECK_THROW(Communicator("127.0.0.1", 33333, confFile), AuthError);
        std::remove(confFile.c_str());
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(InvalidConfigNoColon) {
        std::cout << "[Test 17] Communicator: Чтение конфига без разделителя ':'" << std::endl;
        std::ofstream f(confFile); f << "user_password_without_colon"; f.close();
        std::cout << "  Ожидается исключение AuthError..." << std::endl;
        REQUIRE CHECK_THROW(Communicator("127.0.0.1", 33333, confFile), AuthError);
        std::remove(confFile.c_str());
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(NonExistentConfig) {
        std::cout << "[Test 18] Communicator: Обработка несуществующего файла" << std::endl;
        std::cout << "  Ожидается исключение IOError..." << std::endl;
        REQUIRE CHECK_THROW(Communicator("127.0.0.1", 33333, "no_such_config.txt"), IOError);
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(ConfigOnlyNewline) {
        std::cout << "[Test 19] Communicator: Конфиг состоит только из переноса строки" << std::endl;
        std::ofstream f(confFile); f << "\n"; f.close();
        std::cout << "  Ожидается исключение AuthError..." << std::endl;
        REQUIRE CHECK_THROW(Communicator("127.0.0.1", 33333, confFile), AuthError);
        std::remove(confFile.c_str());
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(ConfigOnlySpaces) {
        std::cout << "[Test 20] Communicator: Конфиг состоит только из пробелов" << std::endl;
        std::ofstream f(confFile); f << "   "; f.close();
        std::cout << "  Ожидается исключение AuthError..." << std::endl;
        REQUIRE CHECK_THROW(Communicator("127.0.0.1", 33333, confFile), AuthError);
        std::remove(confFile.c_str());
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }
}

int main() {
    return UnitTest::RunAllTests();
}
