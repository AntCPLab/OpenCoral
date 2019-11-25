/*
 * ServerSocket.h
 *
 */

#ifndef NETWORKING_SERVERSOCKET_H_
#define NETWORKING_SERVERSOCKET_H_

#include <map>
#include <set>
 #include <queue>
using namespace std;

#include <pthread.h>

#include "Tools/WaitQueue.h"
#include "Tools/Signal.h"

class ServerSocket
{
protected:
    int main_socket, portnum;
    map<int,int> clients;
    std::set<int> used;
    Signal data_signal;
    pthread_t thread;

    // disable copying
    ServerSocket(const ServerSocket& other);

    virtual void process_client(int) {}

public:
    ServerSocket(int Portnum);
    virtual ~ServerSocket();

    virtual void init();

    virtual void accept_clients();

    // This depends on clients sending their id as int.
    // Has to be thread-safe.
    int get_connection_socket(int number);

    // How many client connections have been made.
    virtual int get_connection_count();

    void close_socket();
};

/*
 * ServerSocket where clients do not send any identifiers upon connecting.
 */
class AnonymousServerSocket : public ServerSocket
{
private:
    // No. of accepted connections in this instance
    int num_accepted_clients;
    queue<int> client_connection_queue;

    void process_client(int client_id);

public:
    AnonymousServerSocket(int Portnum) :
        ServerSocket(Portnum), num_accepted_clients(0) { };
    void init();

    virtual int get_connection_count();

    // Get socket and id for the last client who connected
    int get_connection_socket(int& client_id);
};

#endif /* NETWORKING_SERVERSOCKET_H_ */
