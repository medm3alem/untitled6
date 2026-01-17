#include "network.h"
#include <thread>
#include <queue>
#include <mutex>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>

bool network_alive = true;

int sock;
std::queue<std::string> messages;
std::mutex msg_mutex;

void network_connect() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return;
    }

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(4242);

    // Changez cette IP selon votre configuration
    inet_pton(AF_INET, "192.168.1.14", &server.sin_addr);

    if (connect(sock, (sockaddr*)&server, sizeof(server)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        network_alive = false;
        return;
    }

    std::cout << "Connected to server" << std::endl;
}

void network_send(const std::string& msg) {
    if (!network_alive) {
        std::cerr << "Network not alive, cannot send" << std::endl;
        return;
    }

    int sent = send(sock, msg.c_str(), msg.size(), 0);
    if (sent < 0) {
        std::cerr << "Send failed" << std::endl;
        network_alive = false;
    } else {
        std::cout << "Sent: " << msg;
    }
}

void network_start_listener() {
    std::thread([](){
        char buffer[256];
        std::string accumulated;

        while (network_alive) {
            int n = recv(sock, buffer, sizeof(buffer)-1, 0);
            if (n <= 0) {
                std::cout << "Connection closed by server" << std::endl;
                network_alive = false;
                break;
            }

            buffer[n] = '\0';
            accumulated += std::string(buffer);

            // Traiter tous les messages complets (terminés par \n)
            size_t pos;
            while ((pos = accumulated.find('\n')) != std::string::npos) {
                std::string msg = accumulated.substr(0, pos);
                accumulated = accumulated.substr(pos + 1);

                if (!msg.empty()) {
                    std::cout << "Received: " << msg << std::endl;
                    std::lock_guard<std::mutex> lock(msg_mutex);
                    messages.push(msg);
                }
            }
        }
        close(sock);
    }).detach();
}

bool network_has_message() {
    std::lock_guard<std::mutex> lock(msg_mutex);
    return !messages.empty();
}

std::string network_pop_message() {
    std::lock_guard<std::mutex> lock(msg_mutex);
    if (messages.empty()) return "";
    std::string msg = messages.front();
    messages.pop();
    return msg;
}