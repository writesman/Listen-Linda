#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cctype>

// Helper: trim whitespace
std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    size_t end = s.find_last_not_of(" \t\n\r");
    return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " -out|-rd|-in '(tuple)'\n";
        return 1;
    }

    std::string cmd = argv[1];
    std::string tuple;

    // Concatenate the rest of argv into tuple string
    for (int i = 2; i < argc; ++i) {
        tuple += argv[i];
        if (i + 1 < argc) tuple += " ";
    }
    tuple = trim(tuple);

    // Validate command (must be lowercase -out, -rd, -in)
    if (cmd != "-out" && cmd != "-rd" && cmd != "-in") {
        std::cerr << "ERROR: Unknown command " << cmd << "\n";
        return 1;
    }

    // Validate tuple format
    if (tuple.empty() || tuple.front() != '(' || tuple.back() != ')') {
        std::cerr << "ERROR: Tuple must be enclosed in parentheses\n";
        return 1;
    }

    // Check for wildcard in -out
    if (cmd == "-out" && tuple.find('?') != std::string::npos) {
        std::cerr << "ERROR: Wildcard ? is not allowed in -out\n";
        return 1;
    }

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    serv_addr.sin_addr.s_addr = INADDR_ANY; // localhost

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        return 1;
    }

    std::string msg = cmd + " " + tuple;
    write(sock, msg.c_str(), msg.size());

    char buffer[4096];
    ssize_t bytes = read(sock, buffer, sizeof(buffer) - 1);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        std::cout << buffer << std::endl;
    }

    close(sock);
    return 0;
}
