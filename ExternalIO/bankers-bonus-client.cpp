/*
 * Demonstrate external client inputing and receiving outputs from a SPDZ process, 
 * following the protocol described in https://eprint.iacr.org/2015/1006.pdf.
 *
 * Provides a client to bankers_bonus.mpc program to calculate which banker pays for lunch based on
 * the private value annual bonus. Up to 8 clients can connect to the SPDZ engines running 
 * the bankers_bonus.mpc program.
 *
 * Each connecting client:
 * - sends an increasing id to identify the client, starting with 0
 * - sends an integer (0 meaining more players will join this round or 1 meaning stop the round and calc the result).
 * - sends an integer input (bonus value to compare)
 *
 * The result is returned authenticated with a share of a random value:
 * - share of winning unique id [y]
 * - share of random value [r]
 * - share of winning unique id * random value [w]
 *   winning unique id is valid if ∑ [y] * ∑ [r] = ∑ [w]
 *
 * To run with 2 parties / SPDZ engines:
 *   ./Scripts/setup-online.sh to create triple shares for each party (spdz engine).
 *   ./Scripts/setup-clients.sh to create SSL keys and certificates for clients
 *   ./compile.py bankers_bonus
 *   ./Scripts/run-online.sh bankers_bonus to run the engines.
 *
 *   ./bankers-bonus-client.x 0 2 100 0
 *   ./bankers-bonus-client.x 1 2 200 0
 *   ./bankers-bonus-client.x 2 2 50 1
 *
 *   Expect winner to be second client with id 1.
 */

#include "Math/gfp.h"
#include "Math/gf2n.h"
#include "Networking/sockets.h"
#include "Networking/ssl_sockets.h"
#include "Tools/int.h"
#include "Math/Setup.h"
#include "Protocols/fake-stuff.h"

#include <sodium.h>
#include <iostream>
#include <sstream>
#include <fstream>

// Send the private inputs masked with a random value.
// Receive shares of a preprocessed triple from each SPDZ engine, combine and check the triples are valid.
// Add the private input value to triple[0] and send to each spdz engine.
template<class T>
void send_private_inputs(const vector<T>& values, vector<ssl_socket*>& sockets, int nparties)
{
    int num_inputs = values.size();
    octetStream os;
    vector< vector<T> > triples(num_inputs, vector<T>(3));
    vector<T> triple_shares(3);

    // Receive num_inputs triples from SPDZ
    for (int j = 0; j < nparties; j++)
    {
        os.reset_write_head();
        os.Receive(sockets[j]);

#ifdef VERBOSE_COMM
        cerr << "received " << os.get_length() << " from " << j << endl;
#endif

        for (int j = 0; j < num_inputs; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                triple_shares[k].unpack(os);
                triples[j][k] += triple_shares[k];
            }
        }
    }

    // Check triple relations (is a party cheating?)
    for (int i = 0; i < num_inputs; i++)
    {
        if (T(triples[i][0] * triples[i][1]) != triples[i][2])
        {
            cerr << triples[i][2] << " != " << triples[i][0] << " * " << triples[i][1] << endl;
            cerr << "Incorrect triple at " << i << ", aborting\n";
            throw mac_fail();
        }
    }
    // Send inputs + triple[0], so SPDZ can compute shares of each value
    os.reset_write_head();
    for (int i = 0; i < num_inputs; i++)
    {
        T y = values[i] + triples[i][0];
        y.pack(os);
    }
    for (int j = 0; j < nparties; j++)
        os.Send(sockets[j]);
}

// Receive shares of the result and sum together.
// Also receive authenticating values.
template<class T>
T receive_result(vector<ssl_socket*>& sockets, int nparties)
{
    vector<T> output_values(3);
    octetStream os;
    for (int i = 0; i < nparties; i++)
    {
        os.reset_write_head();
        os.Receive(sockets[i]);
        for (unsigned int j = 0; j < 3; j++)
        {
            T value;
            value.unpack(os);
            output_values[j] += value;            
        }
    }

    if (T(output_values[0] * output_values[1]) != output_values[2])
    {
        cerr << "Unable to authenticate output value as correct, aborting." << endl;
        throw mac_fail();
    }
    return output_values[0];
}

template<class T>
void run(int salary_value, vector<ssl_socket*>& sockets, int nparties)
{
    // Run the computation
    send_private_inputs<T>({salary_value}, sockets, nparties);
    cout << "Sent private inputs to each SPDZ engine, waiting for result..." << endl;

    // Get the result back (client_id of winning client)
    T result = receive_result<T>(sockets, nparties);

    cout << "Winning client id is : " << result << endl;
}

int main(int argc, char** argv)
{
    int my_client_id;
    int nparties;
    int salary_value;
    int finish;
    int port_base = 14000;
    string host_name = "localhost";

    if (argc < 5) {
        cout << "Usage is bankers-bonus-client <client identifier> <number of spdz parties> "
           << "<salary to compare> <finish (0 false, 1 true)> <optional host name, default localhost> "
           << "<optional spdz party port base number, default 14000>" << endl;
        exit(0);
    }

    my_client_id = atoi(argv[1]);
    nparties = atoi(argv[2]);
    salary_value = atoi(argv[3]);
    finish = atoi(argv[4]);
    if (argc > 5)
        host_name = argv[5];
    if (argc > 6)
        port_base = atoi(argv[6]);

    bigint::init_thread();

    // Setup connections from this client to each party socket
    vector<int> plain_sockets(nparties);
    vector<ssl_socket*> sockets(nparties);
    ssl_ctx ctx("C" + to_string(my_client_id));
    ssl_service io_service;
    octetStream specification;
    for (int i = 0; i < nparties; i++)
    {
        set_up_client_socket(plain_sockets[i], host_name.c_str(), port_base + i);
        send(plain_sockets[i], (octet*) &my_client_id, sizeof(int));
        sockets[i] = new ssl_socket(io_service, ctx, plain_sockets[i],
                "P" + to_string(i), "C" + to_string(my_client_id), true);
        if (i == 0)
            specification.Receive(sockets[0]);
        octetStream os;
        os.store(finish);
        os.Send(sockets[i]);
    }
    cout << "Finish setup socket connections to SPDZ engines." << endl;

    int type = specification.get<int>();
    switch (type)
    {
    case 'p':
    {
        gfp::init_field(specification.get<bigint>());
        cerr << "using prime " << gfp::pr() << endl;
        run<gfp>(salary_value, sockets, nparties);
        break;
    }
    case 'R':
    {
        int R = specification.get<int>();
        switch (R)
        {
        case 64:
            run<Z2<64>>(salary_value, sockets, nparties);
            break;
        case 104:
            run<Z2<104>>(salary_value, sockets, nparties);
            break;
        case 128:
            run<Z2<128>>(salary_value, sockets, nparties);
            break;
        default:
            cerr << R << "-bit ring not implemented";
            exit(1);
        }
        break;
    }
    default:
        cerr << "Type " << type << " not implemented";
        exit(1);
    }

    for (int i = 0; i < nparties; i++)
        delete sockets[i];

    return 0;
}
