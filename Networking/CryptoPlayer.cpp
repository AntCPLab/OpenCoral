/*
 * CryptoPlayer.cpp
 *
 */

#include "CryptoPlayer.h"
#include "Math/Setup.h"

void check_ssl_file(string filename)
{
    if (not ifstream(filename))
        throw runtime_error("Cannot access " + filename
                        + ". Have you set up SSL?\n"
                        "You can use `Scripts/setup-ssl.sh <nparties>`.");
}

void ssl_error(string side, string pronoun, int other, int server)
{
    cerr << side << "-side handshake with party " << other
            << " failed. Make sure " << pronoun
            << " have the necessary certificate (" << PREP_DIR "P" << server
            << ".pem in the default configuration),"
            << " and run `c_rehash <directory>` on its location." << endl;
}

CryptoPlayer::CryptoPlayer(const Names& Nms, int id_base) :
        MultiPlayer<ssl_socket*>(Nms, id_base), plaintext_player(Nms, id_base),
        ctx(boost::asio::ssl::context::tlsv12)
{
    string prefix = PREP_DIR "P" + to_string(my_num());
    string cert_file = prefix + ".pem";
    string key_file = prefix + ".key";
    check_ssl_file(cert_file);
    check_ssl_file(key_file);

    ctx.use_certificate_file(cert_file, ctx.pem);
    ctx.use_private_key_file(key_file, ctx.pem);
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
            try
            {
                sockets[i]->handshake(ssl_socket::client);
            }
            catch (...)
            {
                ssl_error("Client", "we", i, i);
                throw;
            }
        if (i > my_num())
            try
            {
                sockets[i]->handshake(ssl_socket::server);
            }
            catch (...)
            {
                ssl_error("Server", "they", i, my_num());
                throw;
            }
    }
}

CryptoPlayer::~CryptoPlayer()
{
    close_client_socket(plaintext_player.socket(my_num()));
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
