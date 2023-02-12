#include "TcpServer.hpp"

#include <iostream>

#include "TcpConnection.hpp"

namespace example {
void TcpServer::Observer::onConnectionAccepted(
    [[maybe_unused]] int connectionId) {}

void TcpServer::Observer::onReceived(
    [[maybe_unused]] int connectionId,
    [[maybe_unused]] const std::string &message) {}

void TcpServer::Observer::onConnectionClosed(
    [[maybe_unused]] int connectionId) {}

TcpServer::TcpServer(boost::asio::io_context &ioContext, Observer &observer)
    : m_ioContext{ioContext},
      m_acceptor{ioContext},
      m_connections{},
      m_observer{observer},
      m_connectionCount{0},
      m_isAccepting{false},
      m_isClosing{false} {}

bool TcpServer::listen(const boost::asio::ip::tcp &protocol, uint16_t port) {
  try {
    m_acceptor.open(protocol);
    m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
    m_acceptor.bind({protocol, port});
    m_acceptor.listen(boost::asio::socket_base::max_connections);
  } catch (const std::exception &e) {
    std::cerr << "TCP Server Listen exception: " << e.what() << std::endl;
    return false;
  }
  return true;
}

void TcpServer::startAcceptingConnections() {
  if (!m_isAccepting) {
    doAccept();
  }
}

void TcpServer::send(int connectionId, const std::string &message) {
  if (m_connections.count(connectionId) == 0) {
    std::cerr << "TCP Server Send error: connection not found" << std::endl;
    return;
  }
  m_connections.at(connectionId)->send(message);
}

void TcpServer::close() {
  m_isClosing = true;
  m_acceptor.cancel();
  for (const auto &connection : m_connections) {
    connection.second->close();
  }
  m_connections.clear();
  m_isClosing = false;
  std::cout << "TCP Server was closed" << std::endl;
}

void TcpServer::doAccept() {
  m_isAccepting = true;
  m_acceptor.async_accept([this](const auto &error, auto socket) {
    if (error) {
      std::cerr << "TCP Server Accept error: " << error.message() << std::endl;
      m_isAccepting = false;
      return;
    } else {
      auto connection{
          TcpConnection::create(std::move(socket), *this, m_connectionCount)};
      connection->startReceiving();
      m_connections.insert({m_connectionCount, std::move(connection)});
      std::cout << "TCP Server accepted connection" << std::endl;
      m_observer.onConnectionAccepted(m_connectionCount);
      m_connectionCount++;
    }
    doAccept();
  });
}

void TcpServer::onReceived(int connectionId, const std::string &message) {
  m_observer.onReceived(connectionId, message);
}

void TcpServer::onConnectionClosed(int connectionId) {
  if (m_isClosing) {
    return;
  }
  if (m_connections.erase(connectionId) > 0) {
    std::cout << "TCP Server removed connection" << std::endl;
    m_observer.onConnectionClosed(connectionId);
  }
}
}  // namespace example
