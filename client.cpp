#include <iostream>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " -out| -rd| -in '(tuple)'\n";
        return 1;
    }

    std::string cmd = argv[1];
    std::string tuple = argv[2];

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
    ssize_t bytes = read(sock, buffer, sizeof(buffer)-1);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        std::cout << buffer << std::endl;
    }

    close(sock);
    return 0;
}