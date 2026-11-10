#include "communicator.h"
#include "errors.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <cryptopp/sha.h>
#include <cryptopp/hex.h>

Communicator::Communicator(const std::string& address, uint16_t port, const std::string& configFile) 
    : address(address), port(port), sockfd(-1) {
    readConfig(configFile);
    connectToServer();
    authenticate();
}

Communicator::~Communicator() {
    if (sockfd != -1) {
        close(sockfd);
    }
}

void Communicator::readConfig(const std::string& configFile) {
    std::ifstream file(configFile);
    if (!file.is_open()) {
        throw IOError("Cannot open config file: " + configFile);
    }
    std::string line;
    if (std::getline(file, line)) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            login = line.substr(0, pos);
            password = line.substr(pos + 1);
        } else {
            throw AuthError("Invalid config format (expected LOGIN:PASSWORD)");
        }
    } else {
        throw AuthError("Config file is empty");
    }
}

void Communicator::connectToServer() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        throw NetworkError("Failed to create socket");
    }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, address.c_str(), &serv_addr.sin_addr) <= 0) {
        throw NetworkError("Invalid address/ Address not supported");
    }

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        throw NetworkError("Connection failed");
    }
}

void Communicator::authenticate() {
    // 1. Передача LOGIN
    if (send(sockfd, login.c_str(), login.length(), 0) < 0) {
        throw NetworkError("Failed to send login");
    }

    // 2. Получение SALT
    char saltBuf[17] = {0};
    ssize_t bytesRead = recv(sockfd, saltBuf, 16, 0);
    if (bytesRead <= 0) {
        throw NetworkError("Failed to receive salt or server disconnected");
    }
    std::string salt(saltBuf, bytesRead);
    
    if (salt == "ERR") {
        throw AuthError("Server rejected login");
    }

    // 3. Вычисление хеша SHA-256 (SALT || PASSWORD)
    std::string msg = salt + password;
    std::string hashHex;
    
    CryptoPP::SHA256 sha256;
    CryptoPP::StringSource ss(msg, true,
        new CryptoPP::HashFilter(sha256,
            new CryptoPP::HexEncoder(
                new CryptoPP::StringSink(hashHex)
            )
        )
    );

    // 4. Передача HASH
    if (send(sockfd, hashHex.c_str(), hashHex.length(), 0) < 0) {
        throw NetworkError("Failed to send hash");
    }

    // 5. Получение OK / ERR
    char authBuf[4] = {0};
    bytesRead = recv(sockfd, authBuf, 3, 0);
    if (bytesRead <= 0) {
        throw NetworkError("Failed to receive auth result");
    }
    std::string authResult(authBuf, bytesRead);
    
    if (authResult != "OK") {
        throw AuthError("Authentication failed (invalid password)");
    }
}

std::vector<uint64_t> Communicator::sendAndReceive(const VectorData& data) {
    std::vector<uint64_t> results;
    
    // 6. Передача количества векторов (4 байта, uint32_t)
    uint32_t numVectors = data.size();
    if (send(sockfd, &numVectors, sizeof(numVectors), 0) != sizeof(numVectors)) {
        throw NetworkError("Failed to send number of vectors");
    }

    for (const auto& vec : data) {
        // 7. Передача размера вектора (4 байта, uint32_t)
        uint32_t vecSize = vec.size();
        if (send(sockfd, &vecSize, sizeof(vecSize), 0) != sizeof(vecSize)) {
            throw NetworkError("Failed to send vector size");
        }

        // 8. Передача всех значений вектора одним блоком
        if (vecSize > 0) {
            size_t dataSize = vecSize * sizeof(uint32_t);
            if (send(sockfd, vec.data(), dataSize, 0) != (ssize_t)dataSize) {
                throw NetworkError("Failed to send vector data");
            }
        }

        // 9. Получение результата вычислений по вектору (8 байт, uint64_t)
        uint64_t result;
        if (recv(sockfd, &result, sizeof(result), MSG_WAITALL) != sizeof(result)) {
            throw NetworkError("Failed to receive result for vector");
        }
        results.push_back(result);
    }

    return results;
}
