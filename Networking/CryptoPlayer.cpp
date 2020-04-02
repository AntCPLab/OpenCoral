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

void ssl_error(string side, string pronoun, string other, string server)
{
    cerr << side << "-side handshake with " << other
            << " failed. Make sure " << pronoun
            << " have the necessary certificate (" << PREP_DIR << server
            << ".pem in the default configuration),"
            << " and run `c_rehash <directory>` on its location." << endl;
}

CryptoPlayer::CryptoPlayer(const Names& Nms, int id_base) :
        MultiPlayer<ssl_socket*>(Nms, id_base), plaintext_player(Nms, id_base),
        ctx("P" + to_string(my_num()))
{
    sockets.resize(num_players());
    senders.resize(num_players());

    for (int i = 0; i < (int)sockets.size(); i++)
    {
        if (i == my_num())
        {
            sockets[i] = 0;
            senders[i] = 0;
            continue;
        }

        sockets[i] = new ssl_socket(io_service, ctx, plaintext_player.socket(i),
                "P" + to_string(i), "P" + to_string(my_num()), i < my_num());

        senders[i] = new Sender<ssl_socket*>(sockets[i]);
    }
}

CryptoPlayer::~CryptoPlayer()
{
    close_client_socket(plaintext_player.socket(my_num()));
    plaintext_player.sockets.clear();
    for (int i = 0; i < num_players(); i++)
    {
        delete sockets[i];
        delete senders[i];
    }
}

void CryptoPlayer::pass_around_no_stats(octetStream& to_send,
        octetStream& to_receive, int offset) const
{
    if (&to_send == &to_receive or (get_player(offset) == get_player(-offset)))
    {
        MultiPlayer<ssl_socket*>::pass_around_no_stats(to_send, to_receive, offset);
    }
    else
    {
#ifdef TIME_ROUNDS
        Timer recv_timer;
        TimeScope ts(recv_timer);
#endif
        senders[get_player(offset)]->request(to_send);
        to_receive.Receive(sockets[get_player(-offset)]);
        senders[get_player(offset)]->wait(to_send);
#ifdef TIME_ROUNDS
        cout << "Exchange time: " << recv_timer.elapsed() << " seconds to receive "
            << 1e-3 * to_receive.get_length() << " KB" << endl;
#endif
    }
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
