#include "network.h"
#include <thread>
#include <queue>
#include <mutex>
#include <unistd.h>
#include <arpa/inet.h>


bool network_alive = true;

int sock;
std::queue<std::string> messages;
std::mutex msg_mutex;

void network_connect() {
    sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(4242);
    //inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
//192.168.1.14
//192.168.56.1
//192.168.1.14
	inet_pton(AF_INET, "192.168.1.14", &server.sin_addr);

    connect(sock, (sockaddr*)&server, sizeof(server));
}

void network_send(const std::string& msg) {
    if (!network_alive) return;
	send(sock, msg.c_str(), msg.size(), 0);

}

void network_start_listener() {
    std::thread([](){
        char buffer[256];
        while (true) {
            int n = recv(sock, buffer, sizeof(buffer)-1, 0);
            if (n <= 0) {
    			network_alive = false;
    			break;
			}

            buffer[n] = '\0';

            std::lock_guard<std::mutex> lock(msg_mutex);
            messages.push(std::string(buffer));
        }
    }).detach();
}

bool network_has_message() {
    std::lock_guard<std::mutex> lock(msg_mutex);
    return !messages.empty();
}

std::string network_pop_message() {
    std::lock_guard<std::mutex> lock(msg_mutex);
    std::string msg = messages.front();
    messages.pop();
    return msg;
}
