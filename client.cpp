#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

// Helper: trim whitespace
std::string trim(const std::string &s) {
  size_t start = s.find_first_not_of(" \t\n\r");
  size_t end = s.find_last_not_of(" \t\n\r");
  return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

// Send a string with '\n' at the end
void sendLine(int sock, const std::string &line) {
  std::string msg = line + "\n";
  size_t total_sent = 0;
  while (total_sent < msg.size()) {
    ssize_t sent =
        write(sock, msg.data() + total_sent, msg.size() - total_sent);
    if (sent <= 0)
      break;
    total_sent += sent;
  }
}

// Read a full line ending with '\n' from socket
std::string recvLine(int sock) {
  std::string line;
  char c;
  while (true) {
    ssize_t bytes = read(sock, &c, 1);
    if (bytes <= 0)
      break;
    if (c == '\n')
      break;
    line += c;
  }
  return line;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " -out|-rd|-in '(tuple)'\n";
    return 1;
  }

  std::string cmd = argv[1];

  // Assemble tuple string
  std::string tuple;
  for (int i = 2; i < argc; ++i) {
    if (i > 2)
      tuple += ' ';
    tuple += argv[i];
  }
  tuple = trim(tuple);

  // Validate command
  if (cmd != "-out" && cmd != "-rd" && cmd != "-in") {
    std::cerr << "ERROR: Unknown command " << cmd << "\n";
    return 1;
  }

  // Validate tuple formatting
  if (tuple.empty() || tuple.front() != '(' || tuple.back() != ')') {
    std::cerr << "ERROR: Tuple must be enclosed in parentheses\n";
    return 1;
  }

  if (cmd == "-out" && tuple.find('?') != std::string::npos) {
    std::cerr << "ERROR: Wildcard ? is not allowed in -out\n";
    return 1;
  }

  // Connect to server
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("socket");
    return 1;
  }

  sockaddr_in serv_addr{};
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(8080);
  serv_addr.sin_addr.s_addr = INADDR_ANY;

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("connect");
    return 1;
  }

  // Send command and tuple
  sendLine(sock, cmd + " " + tuple);

  // Receive and print response
  std::string response = recvLine(sock);
  if (!response.empty())
    std::cout << response << std::endl;

  close(sock);
  return 0;
}
