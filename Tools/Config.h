#include "Tools/octetStream.h"
#include "Networking/Player.h"
#include <sodium.h>
namespace Config {
    typedef vector<octet> public_key;
    typedef vector<octet> public_signing_key;
    typedef vector<octet> secret_key;
    typedef vector<octet> secret_signing_key;
    void write_player_config_file(string config_dir
                           ,int player_number, public_key my_pub, secret_key my_priv
                                             , public_signing_key my_signing_pub, secret_signing_key my_signing_priv
                                             , vector<public_key> client_pubs, vector<public_signing_key> client_signing_pubs
                                             , vector<public_key> player_pubs, vector<public_signing_key> player_signing_pubs);
    void putW64le(ofstream &outf, uint64_t nr);
}
