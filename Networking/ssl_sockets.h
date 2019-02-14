/*
 * sockets.h
 *
 */

#ifndef CRYPTO_SSL_SOCKETS_H_
#define CRYPTO_SSL_SOCKETS_H_

#include "Tools/int.h"
#include "sockets.h"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

inline size_t send_non_blocking(ssl_socket* socket, octet* data, size_t length)
{
    return socket->write_some(boost::asio::buffer(data, length));
}

template<>
inline void send(ssl_socket* socket, octet* data, size_t length)
{
    size_t sent = 0;
    while (sent < length)
        sent += send_non_blocking(socket, data + sent, length - sent);
}

template<>
inline void receive(ssl_socket* socket, octet* data, size_t length)
{
    size_t received = 0;
    while (received < length)
        received += socket->read_some(boost::asio::buffer(data + received, length - received));
}

inline size_t receive_non_blocking(ssl_socket* socket, octet* data, int length)
{
    return socket->read_some(boost::asio::buffer(data, length));
}

inline size_t receive_all_or_nothing(ssl_socket* socket, octet* data, size_t length)
{
    receive(socket, data, length);
    return length;
}

#endif /* CRYPTO_SSL_SOCKETS_H_ */
