#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "data_reader.h"

class Communicator {
private:
    std::string address;
    uint16_t port;
    int sockfd;
    std::string login;
    std::string password;

    void connectToServer();
    void authenticate();
    void readConfig(const std::string& configFile);

public:
    Communicator(const std::string& address, uint16_t port, const std::string& configFile);
    ~Communicator();
    std::vector<uint64_t> sendAndReceive(const VectorData& data);
};
