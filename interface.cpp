#include "interface.h"
#include "errors.h"
#include <sstream>
#include <iostream>

namespace po = boost::program_options;

Interface::Interface() : desc("Доступные параметры командной строки") {
    desc.add_options()
        ("help,h", "Показать справку")
        ("address,a", po::value<std::string>(&params.serverAddr)->required(), "Сетевой адрес сервера")
        ("port,p", po::value<uint16_t>(&params.serverPort)->default_value(33333), "Порт сервера")
        ("input,i", po::value<std::string>(&params.inputFile)->required(), "Имя файла с исходными данными")
        ("output,o", po::value<std::string>(&params.outputFile)->required(), "Имя файла для сохранения результатов")
        ("config,c", po::value<std::string>(&params.configFile)->default_value("~/.config/vclient.conf"), "Имя файла конфигурации");
}

Params Interface::parse(int argc, const char** argv) {
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help") || argc == 1) {
            params.showHelp = true;
            return params;
        }
        po::notify(vm);
    } catch (const po::error& e) {
        throw InterfaceError(e.what());
    }
    return params;
}

std::string Interface::getHelp() const {
    std::ostringstream ss;
    ss << "Использование: ./client [опции]\n\n";
    ss << "Пример запуска:\n";
    ss << "./client -a 127.0.0.1 -p 33333 -i input.txt -o result.bin -c config.txt\n\n";
    ss << desc;
    return ss.str();
}
