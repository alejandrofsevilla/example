#ifndef EXAMPLE_TCP_CONNECTION_HPP
#define EXAMPLE_TCP_CONNECTION_HPP

#include <boost/asio.hpp>
#include <mutex>

namespace example {
/**
 * TcpConnection class controls asynchronous operations of a connected TCP
 * socket.
 */
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
 public:
  /**
   * Observer class allows monitoring of TcpConnection events.
   */
  struct Observer {
    /**
     * virtual function called by TcpConnection after message is received.
     *
     * @param connectionId unique identifier of the TcpConnection.
     * @param message received.
     */
    virtual void onReceived(int connectionId, const std::string &message);
    /**
     * virtual function called by TcpConnection after socket has been closed.
     *
     * @param connectionId unique identifier of the TcpConnection.
     */
    virtual void onConnectionClosed(int connectionId);
  };
  /**
   * Creates a TcpConnection instance.
   *
   * @param socket associated to the TcpConnection.
   * @param observer to monitor TcpConnection events.
   * @param id unique identifier the TcpConnection.
   */
  static std::shared_ptr<TcpConnection> create(
      boost::asio::ip::tcp::socket &&socket, Observer &observer, int id = 0);
  /**
   * starts asynchronous read operations.
   */
  void startReceiving();
  /**
   * sends string message to peer connected to socket.
   *
   * @param message to send.
   */
  void send(const std::string &message);
  /**
   * closes socket.
   */
  void close();

 private:
  TcpConnection(boost::asio::ip::tcp::socket &&socket, Observer &observer,
                int id);

  void write();
  void readHeader();
  void readBody(size_t bytesToRead);

  boost::asio::ip::tcp::socket m_socket;
  boost::asio::streambuf m_readBuffer;
  boost::asio::streambuf m_writeBuffer;
  std::mutex m_writeMutex;
  Observer &m_observer;
  bool m_isWritting;
  int m_id;
};
}  // namespace example

#endif
