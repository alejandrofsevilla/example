#include <gtest/gtest.h>

#include <example/TcpClient.hpp>
#include <example/TcpServer.hpp>
#include <random>
#include <thread>

#include "TestHelper.hpp"

namespace example::tests {
TEST(TcpTest, ServerListens) {
  constexpr uint16_t port{1234};
  const auto protocol{boost::asio::ip::tcp::v4()};
  boost::asio::io_context context;
  TcpServer::Observer observer;
  TcpServer server{context, observer};
  EXPECT_EQ(server.listen(protocol, port), true);
}

TEST(TcpTest, ServerAcceptsAndClientConnects) {
  constexpr uint16_t port{1234};
  const auto protocol{boost::asio::ip::tcp::v4()};
  boost::asio::io_context context;
  struct : TcpServer::Observer {
    bool hasAccepted{false};
    void onConnectionAccepted(int) override { hasAccepted = true; };
  } serverObserver;
  TcpServer server{context, serverObserver};
  server.listen(protocol, port);
  server.startAcceptingConnections();
  std::thread thread{[&context]() { context.run(); }};
  struct : TcpClient::Observer {
    bool isConnected{false};
    void onConnected() override { isConnected = true; };
  } clientObserver;
  TcpClient client{context, clientObserver};
  client.connect({protocol, port});
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_EQ(serverObserver.hasAccepted, true);
  EXPECT_EQ(clientObserver.isConnected, true);
  context.stop();
  thread.join();
}

TEST(TcpTest, ClientSends) {
  constexpr uint16_t port{1234};
  constexpr size_t messageSize{1000};
  constexpr size_t messageCount{1000};
  const auto protocol{boost::asio::ip::tcp::v4()};
  boost::asio::io_context context;
  struct : TcpServer::Observer {
    std::string message{generateRandomString(messageSize)};
    size_t messageCount{0};
    void onReceived(int, const std::string &m) override {
      EXPECT_EQ(message, m);
      messageCount++;
    };
  } serverObserver;
  TcpServer server{context, serverObserver};
  server.listen(protocol, port);
  server.startAcceptingConnections();
  std::thread thread{[&context]() { context.run(); }};
  TcpClient::Observer clientObserver;
  TcpClient client{context, clientObserver};
  client.connect({protocol, port});
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  for (size_t i = 0; i < messageCount; i++) {
    client.send(serverObserver.message);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_EQ(serverObserver.messageCount, messageCount);
  context.stop();
  thread.join();
}

TEST(TcpTest, ServerSends) {
  constexpr uint16_t port{1234};
  constexpr size_t messageSize{1000};
  constexpr size_t messageCount{1000};
  const auto protocol{boost::asio::ip::tcp::v4()};
  boost::asio::io_context context;
  struct : TcpServer::Observer {
    int connectionId;
    void onConnectionAccepted(int id) override { connectionId = id; };
  } serverObserver;
  TcpServer server{context, serverObserver};
  server.listen(protocol, port);
  server.startAcceptingConnections();
  std::thread thread{[&context]() { context.run(); }};
  struct : TcpClient::Observer {
    std::string message{generateRandomString(messageSize)};
    size_t messageCount{0};
    void onReceived(const std::string &m) override {
      EXPECT_EQ(message, m);
      messageCount++;
    };
  } clientObserver;
  TcpClient client{context, clientObserver};
  client.connect({protocol, port});
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  for (size_t i = 0; i < messageCount; i++) {
    server.send(serverObserver.connectionId, clientObserver.message);
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_EQ(clientObserver.messageCount, messageCount);
  context.stop();
  thread.join();
}

TEST(TcpTest, ClientDisconnects) {
  constexpr uint16_t port{1234};
  const auto protocol{boost::asio::ip::tcp::v4()};
  boost::asio::io_context context;
  struct : TcpServer::Observer {
    bool clientIsConnected{false};
    void onConnectionAccepted(int) override { clientIsConnected = true; };
    void onConnectionClosed(int) override { clientIsConnected = false; };
  } serverObserver;
  TcpServer server{context, serverObserver};
  server.listen(protocol, port);
  server.startAcceptingConnections();
  std::thread thread{[&context]() { context.run(); }};
  TcpClient::Observer clientObserver;
  TcpClient client{context, clientObserver};
  client.connect({protocol, port});
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_EQ(serverObserver.clientIsConnected, true);
  client.disconnect();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_EQ(serverObserver.clientIsConnected, false);
  context.stop();
  thread.join();
}

TEST(TcpTest, ServerCloses) {
  constexpr uint16_t port{1234};
  const auto protocol{boost::asio::ip::tcp::v4()};
  boost::asio::io_context context;
  TcpServer::Observer serverObserver;
  TcpServer server{context, serverObserver};
  server.listen(protocol, port);
  server.startAcceptingConnections();
  std::thread thread{[&context]() { context.run(); }};
  struct : TcpClient::Observer {
    bool isConnected{false};
    void onConnected() override { isConnected = true; };
    void onDisconnected() override { isConnected = false; };
  } clientObserver;
  TcpClient client{context, clientObserver};
  client.connect({protocol, port});
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_EQ(clientObserver.isConnected, true);
  server.close();
  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  EXPECT_EQ(clientObserver.isConnected, false);
  context.stop();
  thread.join();
}
}  // namespace example::tests
