#pragma once
#include <string>
#include <boost/program_options.hpp>

struct Params {
    std::string serverAddr;
    uint16_t serverPort = 33333;
    std::string inputFile;
    std::string outputFile;
    std::string configFile = "~/.config/vclient.conf";
    bool showHelp = false;
};

class Interface {
private:
    boost::program_options::options_description desc;
    boost::program_options::variables_map vm;
    Params params;
public:
    Interface();
    Params parse(int argc, const char** argv);
    std::string getHelp() const;
};

