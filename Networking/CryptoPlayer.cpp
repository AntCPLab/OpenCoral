/*
 * CryptoPlayer.cpp
 *
 */

#include "CryptoPlayer.h"
#include "Math/Setup.h"

CryptoPlayer::CryptoPlayer(const Names& Nms, int id_base) :
        MultiPlayer<ssl_socket*>(Nms, id_base), plaintext_player(Nms, id_base),
        ctx(boost::asio::ssl::context::tls)
{
    string prefix = PREP_DIR "P" + to_string(my_num());
    ctx.use_certificate_file(prefix + ".pem", ctx.pem);
    ctx.use_private_key_file(prefix + ".key", ctx.pem);
    ctx.add_verify_path("Player-Data");

    sockets.resize(num_players());

    for (int i = 0; i < (int)sockets.size(); i++)
    {
        if (i == my_num())
        {
            sockets[i] = 0;
            continue;
        }

        sockets[i] = new ssl_socket(io_service, ctx);
        sockets[i]->lowest_layer().assign(boost::asio::ip::tcp::v4(), plaintext_player.socket(i));
        sockets[i]->set_verify_mode(boost::asio::ssl::verify_peer);
        sockets[i]->set_verify_callback(boost::asio::ssl::rfc2818_verification("P" + to_string(i)));
        if (i < my_num())
            sockets[i]->handshake(ssl_socket::client);
        if (i > my_num())
            sockets[i]->handshake(ssl_socket::server);
    }
}

CryptoPlayer::~CryptoPlayer()
{
    plaintext_player.sockets.clear();
    for (int i = 0; i < num_players(); i++)
        delete sockets[i];
}

template<>
void MultiPlayer<ssl_socket*>::setup_sockets(const vector<string>& names,
        const vector<int>& ports, int id_base, ServerSocket& server)
{
    (void)names, (void)ports, (void)id_base, (void)server;
}

template<>
void send(ssl_socket* socket, int a)
{
    octet x = a;
    send(socket, &x, 1);
}

template<>
void receive(ssl_socket* socket, int& a)
{
    octet x;
    receive(socket, &x, 1);
    a = x;
}
