/*
 * CryptoPlayer.h
 *
 */

#ifndef NETWORKING_CRYPTOPLAYER_H_
#define NETWORKING_CRYPTOPLAYER_H_

#include "ssl_sockets.h"
#include "Networking/Player.h"

#include <boost/asio/ssl.hpp>
#include <boost/asio.hpp>

class CryptoPlayer : public MultiPlayer<ssl_socket*>
{
    PlainPlayer plaintext_player;
    boost::asio::ssl::context ctx;
    boost::asio::io_service io_service;

public:
    CryptoPlayer(const Names& Nms, int id_base=0);
    ~CryptoPlayer();

    bool is_encrypted() { return true; }
};

#endif /* NETWORKING_CRYPTOPLAYER_H_ */
