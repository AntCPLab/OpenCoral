/*
 * Client.h
 *
 */

#ifndef EXTERNALIO_CLIENT_H_
#define EXTERNALIO_CLIENT_H_

#include "Networking/ssl_sockets.h"

/**
 * Client-side interface
 */
class Client
{
    vector<int> plain_sockets;
    ssl_ctx ctx;
    ssl_service io_service;

public:
    /**
     * Sockets for cleartext communication
     */
    vector<ssl_socket*> sockets;

    /**
     * Specification of computation domain
     */
    octetStream specification;

    /**
     * Start a new set of connections to computing parties.
     * @param hostnames location of computing parties
     * @param port_base port base
     * @param my_client_id client identifier
     */
    Client(const vector<string>& hostnames, int port_base, int my_client_id);
    ~Client();

    /**
     * Securely input private values.
     * @param values vector of integer-like values
     */
    template<class T>
    void send_private_inputs(const vector<T>& values);

    /**
     * Securely receive output values.
     * @param n number of values
     * @returns vector of integer-like values
     */
    template<class T, class U = T>
    vector<U> receive_outputs(int n);
};

#endif /* EXTERNALIO_CLIENT_H_ */
