#include "tuplespace.h"
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <sstream>
#include <netinet/in.h>
#include <unistd.h>
#include <cctype>
#include <stdexcept>

// Helper: trim whitespace
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

// Parse tuple string like ("foo", 42, 3.14, ?) into TupleSpace::Tuple
TupleSpace::Tuple parse_tuple(const std::string& s, bool allow_wildcard) {
    TupleSpace::Tuple tuple;
    std::string str = s;
    if (!str.empty() && str.front() == '(') str = str.substr(1);
    if (!str.empty() && str.back() == ')') str.pop_back();

    std::istringstream iss(str);
    std::string token;
    while (std::getline(iss, token, ',')) {
        token = trim(token);
        if (token.empty()) continue;

        if (token == "?") {
            if (!allow_wildcard)
                throw std::runtime_error("OUT command cannot contain wildcard ?");
            tuple.push_back(TupleSpace::Value{}); // wildcard
        } else if (token.front() == '"' && token.back() == '"') {
            tuple.push_back(token.substr(1, token.size() - 2)); // string
        } else {
            try {
                // First try double
                double dval = std::stod(token);
                // If it has no decimal point, store as int64_t
                if (token.find('.') == std::string::npos && token.find('e') == std::string::npos && token.find('E') == std::string::npos) {
                    tuple.push_back(static_cast<int64_t>(dval));
                } else {
                    tuple.push_back(dval);
                }
            } catch (...) {
                // Fallback string
                tuple.push_back(token);
            }
        }
    }
    return tuple;
}

// Convert tuple to Linda-style output
std::string tuple_to_output(const TupleSpace::Tuple& t) {
    std::ostringstream oss;
    oss << std::fixed;
    for (size_t i = 0; i < t.size(); ++i) {
        if (t[i].type() == typeid(std::string))
            oss << "string \"" << std::any_cast<std::string>(t[i]) << "\"";
        else if (t[i].type() == typeid(int64_t))
            oss << "int64 " << std::any_cast<int64_t>(t[i]);
        else if (t[i].type() == typeid(double))
            oss << "double " << std::any_cast<double>(t[i]);
        else if (!t[i].has_value())
            oss << "?";

        if (i + 1 < t.size()) {
            if (i + 1 == t.size() - 1) oss << " and ";
            else oss << ", ";
        }
    }
    return oss.str();
}

TupleSpace ts;

void handle_client(int client_sock) {
    char buffer[4096];
    while (true) {
        ssize_t bytes = read(client_sock, buffer, sizeof(buffer) - 1);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';
        std::string line(buffer);
        line = trim(line);

        std::istringstream iss(line);
        std::string cmd, tuple_str;
        iss >> cmd;
        std::getline(iss, tuple_str);
        tuple_str = trim(tuple_str);

        std::string response;
        try {
            if (cmd == "-out") {
                auto tuple = parse_tuple(tuple_str, false); // wildcard not allowed
                ts.out(tuple);
                response = "Tuple with " + tuple_to_output(tuple) + " stored in tuple space.";
            } else if (cmd == "-rd") {
                auto tuple = parse_tuple(tuple_str, true);
                auto result = ts.rd(tuple);
                response = "Matched tuple: " + tuple_to_output(result);
            } else if (cmd == "-in") {
                auto tuple = parse_tuple(tuple_str, true);
                auto result = ts.in(tuple);
                response = "Remove tuple: " + tuple_to_output(result);
            } else {
                response = "ERROR Unknown command";
            }
        } catch (const std::exception& e) {
            response = std::string("ERROR: ") + e.what();
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
