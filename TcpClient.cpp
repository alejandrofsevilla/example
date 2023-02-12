#include "TcpClient.hpp"

#include <iostream>

#include "TcpConnection.hpp"

namespace example {
void TcpClient::Observer::onConnected() {}

void TcpClient::Observer::onReceived(
    [[maybe_unused]] const std::string &message) {}

void TcpClient::Observer::onDisconnected() {}

TcpClient::TcpClient(boost::asio::io_context &ioContext, Observer &observer)
    : m_ioContext{ioContext}, m_connection{}, m_observer{observer} {}

void TcpClient::connect(const boost::asio::ip::tcp::endpoint &endpoint) {
  if (m_connection) {
    return;
  }
  auto socket = std::make_shared<boost::asio::ip::tcp::socket>(m_ioContext);
  socket->async_connect(endpoint, [this, socket](const auto &error) {
    if (error) {
      std::cerr << "TCP Client Connect error: " << error.message() << std::endl;
      return;
    }
    m_connection = TcpConnection::create(std::move(*socket), *this);
    m_connection->startReceiving();
    std::cout << "TCP Client was connected" << std::endl;
    m_observer.onConnected();
  });
}

void TcpClient::send(const std::string &message) {
  if (!m_connection) {
    std::cerr << "TCP Client Send error: no connection" << std::endl;
    return;
  }
  m_connection->send(message);
}

void TcpClient::disconnect() {
  if (m_connection) {
    m_connection->close();
  }
}

void TcpClient::onReceived([[maybe_unused]] int connectionId,
                           const std::string &message) {
  m_observer.onReceived(message);
}

void TcpClient::onConnectionClosed([[maybe_unused]] int connectionId) {
  if (m_connection) {
    m_connection.reset();
    std::cout << "TCP Client was disconnected" << std::endl;
    m_observer.onDisconnected();
  }
}
}  // namespace example
