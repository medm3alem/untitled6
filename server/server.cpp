#include "server.h"
#include <iostream>
#include <thread>
#include <vector>
#include <unistd.h>
#include <netinet/in.h>
#include <algorithm>
#include <mutex>

std::vector<int> clients;
std::mutex clients_mutex;


void handle_client(int client_socket) {
    char buffer[256];

    while (true) {
        int n = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (n <= 0) break;

        buffer[n] = '\0';
        std::string msg(buffer);
        std::cout << "Received: " << msg << std::endl;

        // Broadcast à tous les autres clients


    	std::lock_guard<std::mutex> lock(clients_mutex);
    	for (int c : clients) {
        	if (c != client_socket) {
            	send(c, msg.c_str(), msg.size(), 0);
        	}
    	}





    }

    close(client_socket);
    clients.erase(std::remove(clients.begin(), clients.end(), client_socket),
                  clients.end());
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(4242);

    bind(server_fd, (sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);

    std::cout << "Server listening on port 4242" << std::endl;

    while (true) {
        int client_socket = accept(server_fd, nullptr, nullptr);
		std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(client_socket);

        std::thread(handle_client, client_socket).detach();
    }

    close(server_fd);
    return 0;
}
