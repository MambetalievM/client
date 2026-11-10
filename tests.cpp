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
        std::cout << "  Ожидается размер 1-го вектора: 3, Получено: " << data[0].size() << std::endl;
        std::cout << "  Значения 1-го вектора: " << data[0][0] << ", " << data[0][1] << ", " << data[0][2] << std::endl;

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

    TEST(ReadNonExistentFile) {
        std::cout << "[Test 2] DataReader: Обработка несуществующего файла" << std::endl;
        std::cout << "  Ожидается исключение IOError..." << std::endl;
        CHECK_THROW(DataReader("no_such_file.txt"), IOError);
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(ReadInvalidFormat) {
        std::cout << "[Test 3] DataReader: Обработка неверного формата файла" << std::endl;
        createFile(invalidFile, "hello world");
        std::cout << "  Создан файл с текстом 'hello world' вместо чисел" << std::endl;
        std::cout << "  Ожидается исключение IOError..." << std::endl;
        DataReader reader(invalidFile);
        CHECK_THROW(reader.readData(), IOError);
        
        std::remove(invalidFile.c_str());
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }
}

// 2. Тестирование DataWriter
SUITE(DataWriterTest)
{
    const std::string outFile = "test_output.bin";

    TEST(WriteValidData) {
        std::cout << "[Test 4] DataWriter: Запись корректных данных" << std::endl;
        std::vector<uint64_t> results = {60, 90, 120};
        std::cout << "  Записываем результаты: 60, 90, 120" << std::endl;
        DataWriter writer(outFile);
        writer.writeData(results);

        std::ifstream file(outFile, std::ios::binary | std::ios::ate);
        CHECK(file.is_open());
        
        std::cout << "  Ожидаемый размер файла: 28 байт, Получено: " << (int)file.tellg() << " байт" << std::endl;
        CHECK_EQUAL(28, file.tellg());

        file.seekg(0);
        uint32_t count;
        file.read(reinterpret_cast<char*>(&count), sizeof(count));
        std::cout << "  Ожидаемое количество элементов: 3, Получено: " << count << std::endl;
        CHECK_EQUAL(3, count);

        uint64_t val;
        file.read(reinterpret_cast<char*>(&val), sizeof(val));
        std::cout << "  Ожидаемый первый элемент: 60, Получено: " << val << std::endl;
        CHECK_EQUAL(60, val);

        file.close();
        std::remove(outFile.c_str());
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(WriteToInvalidPath) {
        std::cout << "[Test 5] DataWriter: Обработка неверного пути для записи" << std::endl;
        std::cout << "  Попытка записи в несуществующую папку '/invalid_path/out.bin'" << std::endl;
        DataWriter writer("/invalid_path/out.bin");
        std::vector<uint64_t> results = {1, 2};
        std::cout << "  Ожидается исключение IOError..." << std::endl;
        CHECK_THROW(writer.writeData(results), IOError);
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }
}

// 3. Тестирование Interface (Парсинг аргументов)
SUITE(InterfaceTest)
{
    TEST(ParseValidArguments) {
        std::cout << "[Test 6] Interface: Парсинг корректных аргументов командной строки" << std::endl;
        const char* argv[] = {"client", "-a", "127.0.0.1", "-p", "33333", "-i", "in.txt", "-o", "out.bin", "-c", "conf.txt"};
        Interface iface;
        Params params = iface.parse(11, argv);
        
        std::cout << "  Ожидался IP: 127.0.0.1, Получен: " << params.serverAddr << std::endl;
        std::cout << "  Ожидался порт: 33333, Получен: " << params.serverPort << std::endl;
        std::cout << "  Ожидался input: in.txt, Получен: " << params.inputFile << std::endl;
        std::cout << "  Ожидался output: out.bin, Получен: " << params.outputFile << std::endl;

        CHECK_EQUAL("127.0.0.1", params.serverAddr);
        CHECK_EQUAL(33333, params.serverPort);
        CHECK_EQUAL("in.txt", params.inputFile);
        CHECK_EQUAL("out.bin", params.outputFile);
        CHECK_EQUAL("conf.txt", params.configFile);
        CHECK_EQUAL(false, params.showHelp);
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(ShowHelp) {
        std::cout << "[Test 7] Interface: Обработка флага -h (вызов справки)" << std::endl;
        const char* argv[] = {"client", "-h"};
        Interface iface;
        Params params = iface.parse(2, argv);
        std::cout << "  Ожидается showHelp = true, Получено: " << (params.showHelp ? "true" : "false") << std::endl;
        CHECK_EQUAL(true, params.showHelp);
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(MissingRequiredArguments) {
        std::cout << "[Test 8] Interface: Обработка отсутствия обязательных аргументов" << std::endl;
        std::cout << "  Запуск без аргумента -a (адрес)" << std::endl;
        const char* argv[] = {"client", "-i", "in.txt", "-o", "out.bin"};
        Interface iface;
        std::cout << "  Ожидается исключение InterfaceError..." << std::endl;
        CHECK_THROW(iface.parse(5, argv), InterfaceError);
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }
}

// 4. Тестирование Communicator (Чтение конфига)
SUITE(CommunicatorTest)
{
    const std::string confFile = "test_config.txt";

    TEST(InvalidConfigEmpty) {
        std::cout << "[Test 9] Communicator: Чтение пустого конфигурационного файла" << std::endl;
        std::ofstream f(confFile);
        f.close();
        
        std::cout << "  Ожидается исключение AuthError..." << std::endl;
        CHECK_THROW(Communicator("127.0.0.1", 33333, confFile), AuthError);
        
        std::remove(confFile.c_str());
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(InvalidConfigNoColon) {
        std::cout << "[Test 10] Communicator: Чтение конфига без разделителя ':'" << std::endl;
        std::ofstream f(confFile);
        f << "user_password_without_colon";
        f.close();
        
        std::cout << "  В файле нет символа ':', ожидается исключение AuthError..." << std::endl;
        CHECK_THROW(Communicator("127.0.0.1", 33333, confFile), AuthError);
        
        std::remove(confFile.c_str());
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }

    TEST(NonExistentConfig) {
        std::cout << "[Test 11] Communicator: Обработка несуществующего файла конфигурации" << std::endl;
        std::cout << "  Ожидается исключение IOError..." << std::endl;
        CHECK_THROW(Communicator("127.0.0.1", 33333, "no_such_config.txt"), IOError);
        std::cout << "  -> ТЕСТ ПРОЙДЕН" << std::endl;
    }
}

int main() {
    return UnitTest::RunAllTests();
}
