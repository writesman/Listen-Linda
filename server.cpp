#include "tuplespace.h"
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <sstream>
#include <netinet/in.h>
#include <unistd.h>
#include <cctype>

// Helper: trim whitespace
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

// Parse tuple string like ("foo", 42, ?) into std::any vector
std::vector<std::any> parse_tuple(const std::string& s) {
    std::vector<std::any> tuple;
    std::string str = s;
    if (str.front() == '(') str = str.substr(1);
    if (str.back() == ')') str.pop_back();

    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ',')) {
        token = trim(token);
        if (token == "?") {
            tuple.push_back(std::any{}); // wildcard
        } else if (token.front() == '"' && token.back() == '"') {
            tuple.push_back(token.substr(1, token.size() - 2)); // string
        } else {
            try {
                int64_t val = std::stoll(token);
                tuple.push_back(val);
            } catch (...) {
                tuple.push_back(token); // fallback string
            }
        }
    }
    return tuple;
}

// Convert tuple to example output string with types
std::string tuple_to_output(const std::vector<std::any>& t) {
    std::ostringstream oss;
    for (size_t i = 0; i < t.size(); ++i) {
        if (t[i].type() == typeid(std::string)) oss << "string \"" << std::any_cast<std::string>(t[i]) << "\"";
        else if (t[i].type() == typeid(int64_t)) oss << "int64 " << std::any_cast<int64_t>(t[i]);
        else if (!t[i].has_value()) oss << "?";
        if (i + 1 < t.size()) oss << ", ";
    }
    return oss.str();
}

TupleSpace ts;

void handle_client(int client_sock) {
    char buffer[4096];
    while (true) {
        ssize_t bytes = read(client_sock, buffer, sizeof(buffer)-1);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';
        std::string line(buffer);
        line = trim(line);

        // Parse command (-out, -rd, -in) and tuple
        std::istringstream iss(line);
        std::string cmd, tuple_str;
        iss >> cmd;
        std::getline(iss, tuple_str);
        tuple_str = trim(tuple_str);

        auto tuple = parse_tuple(tuple_str);
        std::string response;

        if (cmd == "-out") {
            ts.out(tuple);
            response = "Tuple with " + tuple_to_output(tuple) + " stored in tuple space.";
        } else if (cmd == "-rd") {
            auto result = ts.rd(tuple);
            response = "Matched tuple: " + tuple_to_output(result);
        } else if (cmd == "-in") {
            auto result = ts.in(tuple);
            response = "Remove tuple: " + tuple_to_output(result);
        } else {
            response = "ERROR Unknown command";
        }

        write(client_sock, response.c_str(), response.size());
    }
    close(client_sock);
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) { perror("socket"); exit(1); }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); exit(1); }
    if (listen(server_fd, 5) < 0) { perror("listen"); exit(1); }

    std::cout << "Server running, listening on port 8080..." << std::endl;

    while (true) {
        int client_sock = accept(server_fd, nullptr, nullptr);
        if (client_sock >= 0) std::thread(handle_client, client_sock).detach();
    }

    close(server_fd);
    return 0;
}