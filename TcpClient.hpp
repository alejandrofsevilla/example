#ifndef EXAMPLE_TCP_CLIENT_HPP
#define EXAMPLE_TCP_CLIENT_HPP

#include <boost/asio.hpp>
#include <memory>

#include "TcpConnection.hpp"

namespace example {
/**
 * TcpClient class allows to connect to specified TCP endpoint, and use
 * associated connection to send and receive string messages.
 */
class TcpClient : private TcpConnection::Observer {
 public:
  /**
   * Observer class allows monitoring of TcpClient events.
   */
  struct Observer {
    /**
     * virtual function called by TcpClient after connection success.
     */
    virtual void onConnected();
    /**
     * virtual function called by TcpClient after message is recevied.
     *
     * @param message received.
     */
    virtual void onReceived(const std::string &message);
    /**
     * virtual function called by TcpClient after connection is closed or
     * after an attempted connection fails.
     */
    virtual void onDisconnected();
  };
  /**
   * Constructs a TcpClient object.
   *
   * @param ioContext required for asynchronous input and ouput operations.
   * @param observer to monitor TcpClient events.
   */
  TcpClient(boost::asio::io_context &ioContext, Observer &observer);
  /**
   * attempts to connect to specified endpoint.
   *
   * @param endpoint to connect to.
   */
  void connect(const boost::asio::ip::tcp::endpoint &endpoint);
  /**
   * Sends string message to peer associated to TcpClient connection if
   * exists.
   *
   * @param message to send.
   */
  void send(const std::string &message);
  /**
   * Closes connection.
   */
  void disconnect();

 private:
  void onReceived(int connectionId, const std::string &message) override;
  void onConnectionClosed(int connectionId) override;

  boost::asio::io_context &m_ioContext;
  std::shared_ptr<TcpConnection> m_connection;
  Observer &m_observer;
};
}  // namespace example

#endif
