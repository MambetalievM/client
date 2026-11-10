#include <UnitTest++/UnitTest++.h>
#include <fstream>
#include <cstdio>
#include <vector>
#include <string>

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
        createFile(validFile, "2\n3\n10 20 30\n2\n40 50");
        
        DataReader reader(validFile);
        VectorData data = reader.readData();
        
        CHECK_EQUAL(2, data.size());
        CHECK_EQUAL(3, data[0].size());
        CHECK_EQUAL(10, data[0][0]);
        CHECK_EQUAL(20, data[0][1]);
        CHECK_EQUAL(30, data[0][2]);
        CHECK_EQUAL(2, data[1].size());
        CHECK_EQUAL(40, data[1][0]);
        CHECK_EQUAL(50, data[1][1]);
        
        std::remove(validFile.c_str());
    }

    TEST(ReadNonExistentFile) {
        CHECK_THROW(DataReader("no_such_file.txt"), IOError);
    }

    TEST(ReadInvalidFormat) {
        createFile(invalidFile, "hello world");
        DataReader reader(invalidFile);
        CHECK_THROW(reader.readData(), IOError);
        
        std::remove(invalidFile.c_str());
    }
}

// 2. Тестирование DataWriter
SUITE(DataWriterTest)
{
    const std::string outFile = "test_output.bin";

    TEST(WriteValidData) {
        std::vector<uint64_t> results = {60, 90, 120};
        DataWriter writer(outFile);
        writer.writeData(results);

        std::ifstream file(outFile, std::ios::binary | std::ios::ate);
        CHECK(file.is_open());
        CHECK_EQUAL(28, file.tellg());

        file.seekg(0);
        uint32_t count;
        file.read(reinterpret_cast<char*>(&count), sizeof(count));
        CHECK_EQUAL(3, count);

        uint64_t val;
        file.read(reinterpret_cast<char*>(&val), sizeof(val));
        CHECK_EQUAL(60, val);

        file.close();
        std::remove(outFile.c_str());
    }

    TEST(WriteToInvalidPath) {
        DataWriter writer("/invalid_path/out.bin");
        std::vector<uint64_t> results = {1, 2};
        CHECK_THROW(writer.writeData(results), IOError);
    }
}

// 3. Тестирование Interface (Парсинг аргументов)
SUITE(InterfaceTest)
{
    TEST(ParseValidArguments) {
        const char* argv[] = {"client", "-a", "127.0.0.1", "-p", "33333", "-i", "in.txt", "-o", "out.bin", "-c", "conf.txt"};
        Interface iface;
        Params params = iface.parse(11, argv);
        
        CHECK_EQUAL("127.0.0.1", params.serverAddr);
        CHECK_EQUAL(33333, params.serverPort);
        CHECK_EQUAL("in.txt", params.inputFile);
        CHECK_EQUAL("out.bin", params.outputFile);
        CHECK_EQUAL("conf.txt", params.configFile);
        CHECK_EQUAL(false, params.showHelp);
    }

    TEST(ShowHelp) {
        const char* argv[] = {"client", "-h"};
        Interface iface;
        Params params = iface.parse(2, argv);
        CHECK_EQUAL(true, params.showHelp);
    }

    TEST(MissingRequiredArguments) {
        const char* argv[] = {"client", "-i", "in.txt", "-o", "out.bin"};
        Interface iface;
        CHECK_THROW(iface.parse(5, argv), InterfaceError);
    }
}

// 4. Тестирование Communicator (Чтение конфига)
SUITE(CommunicatorTest)
{
    const std::string confFile = "test_config.txt";

    TEST(InvalidConfigEmpty) {
        std::ofstream f(confFile);
        f.close();
        
        CHECK_THROW(Communicator("127.0.0.1", 33333, confFile), AuthError);
        
        std::remove(confFile.c_str());
    }

    TEST(InvalidConfigNoColon) {
        std::ofstream f(confFile);
        f << "user_password_without_colon";
        f.close();
        
        CHECK_THROW(Communicator("127.0.0.1", 33333, confFile), AuthError);
        
        std::remove(confFile.c_str());
    }

    TEST(NonExistentConfig) {
        CHECK_THROW(Communicator("127.0.0.1", 33333, "no_such_config.txt"), IOError);
    }
}

int main() {
    return UnitTest::RunAllTests();
}
