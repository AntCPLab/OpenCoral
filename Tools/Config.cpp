// Client key file format:
//      X25519  Public Key
//      X25519  Secret Key
//      Ed25519 Public Key
//      Ed25519 Secret Key
//      Server 1 X25519 Public Key
//      Server 1 Ed25519 Public Key
//      ... 
//      Server N Public Key
//      Server N Ed25519 Public Key
//
// Player key file format:
//      X25519  Public Key
//      X25519  Secret Key
//      Ed25519 Public Key
//      Ed25519 Secret Key
//      Number of clients [64 bit little endian]
//      Client 1 X25519 Public Key
//      Client 1 Ed25519 Public Key
//      ...
//      Client N X25519 Public Key
//      Client N Ed25519 Public Key
//      Number of servers [64 bit little endian]
//      Server 1 X25519 Public Key
//      Server 1 Ed25519 Public Key
//      ...
//      Server N X25519 Public Key
//      Server N Ed25519 Public Key
#include "Tools/octetStream.h"
#include "Networking/Player.h"
#include "Math/gf2n.h"
#include "Config.h"
#include <sodium.h>
#include <vector>
#include <iomanip>

namespace Config {
    static void output(const vector<octet> &vec, ofstream &of)
    {
        copy(vec.begin(), vec.end(), ostreambuf_iterator<char>(of));
    }

    void putW64le(ofstream &outf, uint64_t nr)
    {
        char buf[8];
        for(int i=0;i<8;i++) {
            char byte = (uint8_t)(nr >> (i*8));
            buf[i] = (char)byte;
        }
        outf.write(buf,sizeof buf);
    }

    void write_player_config_file(string config_dir
                           ,int player_number, public_key my_pub, secret_key my_priv
                                             , public_signing_key my_signing_pub, secret_signing_key my_signing_priv
                                             , vector<public_key> client_pubs, vector<public_signing_key> client_signing_pubs
                                             , vector<public_key> player_pubs, vector<public_signing_key> player_signing_pubs)
    {
        stringstream filename;
        filename << config_dir << "Player-SPDZ-Keys-P" << player_number;
        ofstream outf(filename.str().c_str(), ios::out | ios::binary);
        if (outf.fail())
            throw file_error(filename.str().c_str());
        if(crypto_box_PUBLICKEYBYTES != my_pub.size()  ||
           crypto_box_SECRETKEYBYTES != my_priv.size() ||
           crypto_sign_PUBLICKEYBYTES != my_signing_pub.size() ||
           crypto_sign_SECRETKEYBYTES != my_signing_priv.size()) {
            throw "Invalid key sizes";
        } else if(client_pubs.size() != client_signing_pubs.size()) {
            throw "Incorrect number of client keys";
        } else if(player_pubs.size() != player_signing_pubs.size()) {
            throw "Incorrect number of player keys";
        } else {
            for(size_t i = 0; i < client_pubs.size(); i++) {
                if(crypto_box_PUBLICKEYBYTES != client_pubs[i].size() ||
                   crypto_sign_PUBLICKEYBYTES != client_signing_pubs[i].size()) {
                       throw "Incorrect size of client key.";
                   }
            }
            for(size_t i = 0; i < player_pubs.size(); i++) {
                if(crypto_box_PUBLICKEYBYTES != player_pubs[i].size() ||
                   crypto_sign_PUBLICKEYBYTES != player_signing_pubs[i].size()) {
                       throw "Incorrect size of player key.";
                   }
            }
        }
        // Write public and secret X25519 keys
        output(my_pub, outf);
        output(my_priv, outf);
        output(my_signing_pub, outf);
        output(my_signing_priv, outf);

        putW64le(outf, (uint64_t)client_pubs.size());
        // Write all client public keys
        for (size_t j = 0; j < client_pubs.size(); j++) {
            output(client_pubs[j], outf);
            output(client_signing_pubs[j], outf);
        }
        putW64le(outf, (uint64_t)player_pubs.size());
        for (size_t j = 0; j < player_pubs.size(); j++) {
            output(player_pubs[j], outf);
            output(player_signing_pubs[j], outf);
        }
        outf.flush();
        outf.close();
    }
}
