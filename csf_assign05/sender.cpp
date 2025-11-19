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

  while (true) {
    std::cout << "> ";
    std::string input;
    std::getline(std::cin, input);

    input = trim(input);
    if (input.empty()) {
      continue;
    }

    std::istringstream iss(input);
    std::string command;
    iss >> command;

    if (command == "quit") {
      Message quit_msg(TAG_QUIT, "");
      if (!connection.send(quit_msg)) {
        std::cerr << "Connection lost, exiting" << std::endl;
        return 1;
      }
      connection.close();
      return 0;

    } else if (command == "join") {
      std::string room_name;
      iss >> room_name;
      if (room_name.empty()) {
        std::cout << "Usage: join [room_name]" << std::endl;
        continue;
      }

      Message join_msg(TAG_JOIN, room_name);
      if (!connection.send(join_msg) || !connection.receive(response)) {
        std::cerr << "Connection lost, exiting" << std::endl;
        return 1;
      }

      if (response.tag == TAG_OK) {
        std::cout << "Joined room: " << room_name << std::endl;
      } else if (response.tag == TAG_ERR) {
        std::cout << "Server error: " << response.data << std::endl;
      }

    } else if (command == "leave") {
      std::string room_name;
      iss >> room_name;
      if (room_name.empty()) {
        std::cout << "Usage: leave [room_name]" << std::endl;
        continue;
      }

      Message leave_msg(TAG_LEAVE, room_name);
      if (!connection.send(leave_msg) || !connection.receive(response)) {
        std::cerr << "Connection lost, exiting" << std::endl;
        return 1;
      }

      if (response.tag == TAG_OK) {
        std::cout << "Left room: " << room_name << std::endl;
      } else if (response.tag == TAG_ERR) {
        std::cout << "Server error: " << response.data << std::endl;
      }

    } else if (command == "sendall") {
      std::string message;
      std::getline(iss >> std::ws, message);
      if (message.empty()) {
        std::cout << "Usage: sendall [message]" << std::endl;
        continue;
      }

      Message sendall_msg(TAG_SENDALL, message);
      if (!connection.send(sendall_msg) || !connection.receive(response)) {
        std::cerr << "Connection lost, exiting" << std::endl;
        return 1;
      }

      if (response.tag == TAG_OK) {
        std::cout << "Message sent" << std::endl;
      } else if (response.tag == TAG_ERR) {
        std::cout << "Server error: " << response.data << std::endl;
      }

    } else if (command == "senduser") {
      std::string username_param;
      iss >> username_param;
      if (username_param.empty()) {
        std::cout << "Usage: senduser [username] [message]" << std::endl;
        continue;
      }

      std::string message;
      std::getline(iss >> std::ws, message);
      if (message.empty()) {
        std::cout << "Usage: senduser [username] [message]" << std::endl;
        continue;
      }

      std::string senduser_data = username_param + ":" + message;
      Message senduser_msg(TAG_SENDUSER, senduser_data);
      if (!connection.send(senduser_msg) || !connection.receive(response)) {
        std::cerr << "Connection lost, exiting" << std::endl;
        return 1;
      }

      if (response.tag == TAG_OK) {
        std::cout << "Message sent to " << username_param << std::endl;
      } else if (response.tag == TAG_ERR) {
        std::cout << "Server error: " << response.data << std::endl;
      }

    } else if (command == "help") {
      std::cout << "Available commands:" << std::endl;
      std::cout << "  join [room_name]    - Join a chat room" << std::endl;
      std::cout << "  leave [room_name]   - Leave a chat room" << std::endl;
      std::cout << "  sendall [message]   - Send message to all users in current room" << std::endl;
      std::cout << "  senduser [username] [message] - Send private message to user" << std::endl;
      std::cout << "  quit                - Exit the client" << std::endl;
      std::cout << "  help                - Show this help message" << std::endl;

    } else {
      std::cout << "Unknown command. Type 'help' for available commands." << std::endl;
    }
  }

  return 0;
}
