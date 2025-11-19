#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  std::string server_hostname;
  int server_port;
  std::string username;

  server_hostname = argv[1];
  server_port = std::stoi(argv[2]);
  username = argv[3];

  Connection connection;
  try {
    connection.connect(server_hostname, server_port);
    if (!connection.is_open()) {
      std::cerr << "Failed to connect to server" << std::endl;
      return 1;
    }
  } catch (const std::exception &e) {
    std::cerr << "Failed to connect to server" << std::endl;
    return 1;
  }

  Message slogin_msg(TAG_SLOGIN, username);
  if (!connection.send(slogin_msg)) {
    std::cerr << "Connection lost during login" << std::endl;
    return 1;
  }

  Message response;
  if (!connection.receive(response)) {
    std::cerr << "Connection lost during login" << std::endl;
    return 1;
  }

  if (response.tag == TAG_ERR) {
    std::cerr << response.data << std::endl;
    return 1;
  } else if (response.tag != TAG_OK) {
    std::cerr << "Unexpected server response during login" << std::endl;
    return 1;
  }

  // TODO: loop reading commands from user, sending messages to
  //       server as appropriate

  return 0;
}
