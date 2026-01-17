#pragma once
#include <string>

void network_connect();
void network_send(const std::string& msg);
void network_start_listener();
bool network_has_message();
std::string network_pop_message();