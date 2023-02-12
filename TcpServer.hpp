#ifndef NETWORKING_TCP_SERVER_HPP
#define NETWORKING_TCP_SERVER_HPP

#include <boost/asio.hpp>
#include <map>

#include "TcpConnection.hpp"

namespace example {
/**
 * TcpServer class allows to accept TCP connections at any network interface,
 * with specified protocol, and specified port, and use associated connections
 * to send and receive string messages.
 */
class TcpServer : private TcpConnection::Observer {
 public:
  /**
   * Observer class allows monitoring of TcpServer events.
   */
  struct Observer {
    /**
     * virtual function called by TcpServer after a connection has been
     * accepted.
     *
     * @param connectionId unique identifier of the accepted connection.
     */
    virtual void onConnectionAccepted(int connectionId);
    /**
     * virtual function called by TcpServer after message has been received.
     *
     * @param connectionId unique identifier of the receiving connection.
     * @param message received at associated connection.
     */
    virtual void onReceived(int connectionId, const std::string &message);
    /**
     * virtual function called by TcpServer after a connection has been closed.
     *
     * @param connectionId unique identifier of the closed connection.
     */
    virtual void onConnectionClosed(int connectionId);
  };
  /**
   * Constructs a TcpServer object.
   *
   * @param ioContext required for asynchronous input and output operations.
   * @param observer to monitor TcpServer events.
   */
  TcpServer(boost::asio::io_context &ioContext, Observer &observer);
  /**
   * listen for connections at any interface with specified
   * protocol to specified port.
   *
   * @param protocol of the interfaces to listen at.
   * @param port to listen to.
   */
  bool listen(const boost::asio::ip::tcp &protocol, uint16_t port);
  /**
   * starts accepting connections and associated asynchronous read and
   * write operations.
   */
  void startAcceptingConnections();
  /**
   * Sends string message to peer associated to specified connection.
   *
   * @param connectionId unique identifier associated to receiving peer.
   * @param message to send.
   */
  void send(int connectionId, const std::string &message);
  /**
   * Close active connections and stops accepting new connections. In order to
   * restart operation, a call to functions listen() and
   * startAcceptingConnections() is required.
   */
  void close();

 private:
  void doAccept();
  void onReceived(int connectionId, const std::string &message) override;
  void onConnectionClosed(int connectionId) override;

  boost::asio::io_context &m_ioContext;
  boost::asio::ip::tcp::acceptor m_acceptor;
  std::unordered_map<int, std::shared_ptr<TcpConnection>> m_connections;
  Observer &m_observer;
  int m_connectionCount;
  bool m_isAccepting;
  bool m_isClosing;
};
}  // namespace example

#endif
