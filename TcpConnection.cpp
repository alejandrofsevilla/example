#include "TcpConnection.hpp"

#include <iostream>

namespace {
constexpr std::string_view f_messageHeaderSeparator{";"};
constexpr size_t f_messageMaxSize{std::numeric_limits<uint32_t>::max()};
}  // namespace

namespace example {
void TcpConnection::Observer::onReceived(
    [[maybe_unused]] int connectionId,
    [[maybe_unused]] const std::string &message) {}

void TcpConnection::Observer::onConnectionClosed(
    [[maybe_unused]] int connectionId) {}

TcpConnection::TcpConnection(boost::asio::ip::tcp::socket &&socket,
                             Observer &observer, int id)
    : m_socket{std::move(socket)},
      m_readBuffer{},
      m_writeBuffer{},
      m_writeMutex{},
      m_observer{observer},
      m_isWritting{false},
      m_id{id} {}

std::shared_ptr<TcpConnection> TcpConnection::create(
    boost::asio::ip::tcp::socket &&socket, Observer &observer, int id) {
  return std::shared_ptr<TcpConnection>(
      new TcpConnection{std::move(socket), observer, id});
}

void TcpConnection::startReceiving() { readHeader(); }

void TcpConnection::send(const std::string &message) {
  if (message.size() > f_messageMaxSize) {
    std::cerr << "TCP Connection Send error: message is too large" << std::endl;
  }
  std::lock_guard<std::mutex> guard{m_writeMutex};
  std::ostream bufferStream{&m_writeBuffer};
  bufferStream << message.size() << f_messageHeaderSeparator << message;
  if (!m_isWritting) {
    write();
  }
}

void TcpConnection::close() {
  try {
    m_socket.cancel();
    m_socket.close();
  } catch (const std::exception &e) {
    std::cerr << "TCP Connection Close exception: " << e.what() << std::endl;
    return;
  }
  m_observer.onConnectionClosed(m_id);
}

void TcpConnection::write() {
  m_isWritting = true;
  auto self = shared_from_this();
  m_socket.async_write_some(
      m_writeBuffer.data(),
      [this, self](const auto &error, auto bytesTransferred) {
        if (error) {
          std::cerr << "TCP Connection Write error: " << error.message()
                    << std::endl;
          return close();
        }
        std::lock_guard<std::mutex> guard{m_writeMutex};
        m_writeBuffer.consume(bytesTransferred);
        if (m_writeBuffer.size() == 0) {
          m_isWritting = false;
          return;
        }
        write();
      });
}

void TcpConnection::readHeader() {
  auto self = shared_from_this();
  boost::asio::async_read_until(
      m_socket, m_readBuffer, f_messageHeaderSeparator,
      [this, self](const auto &error, [[maybe_unused]] auto bytesTransferred) {
        if (error) {
          std::cerr << "TCP Connection Read error: " << error.message()
                    << std::endl;
          return close();
        }
        std::istream bufferStream{&m_readBuffer};
        size_t header;
        bufferStream >> header;
        bufferStream.ignore(f_messageHeaderSeparator.size());
        readBody(header);
      });
}

void TcpConnection::readBody(size_t bytesToRead) {
  int pendingBytesToRead = bytesToRead - m_readBuffer.size();
  auto buffers = m_readBuffer.prepare(std::max(pendingBytesToRead, 0));
  auto self = shared_from_this();
  m_socket.async_read_some(
      buffers,
      [this, self, bytesToRead](const auto &error, auto bytesTransferred) {
        if (error) {
          std::cerr << "TCP Connection Read error: " << error.message()
                    << std::endl;
          return close();
        }
        m_readBuffer.commit(bytesTransferred);
        if (m_readBuffer.size() >= bytesToRead) {
          auto bufferBegin = boost::asio::buffers_begin(m_readBuffer.data());
          std::string message{bufferBegin, bufferBegin + bytesToRead};
          m_readBuffer.consume(bytesToRead);
          m_observer.onReceived(m_id, message);
          return readHeader();
        }
        readBody(bytesToRead);
      });
}
}  // namespace example
