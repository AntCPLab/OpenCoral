Networking
----------

All protocols in MP-SPDZ rely on point-to-point connections between
all pairs of parties. This is realized using TCP, which means that
every party must be reachable under at least one TCP port. The default
is to set this port to a base plus the player number. This allows to
easily run all parties on the same host. The base defaults to 5000,
which can be changed with the command-line option
``--portnumbase``. There are two ways of communicating hosts and
individually setting ports:

1. All parties first to connect to a coordination server, which
   broadcasts the data for all parties. This is the default with the
   coordination server being run as a thread of party 0. The hostname
   of the coordination server has to be given with the command-line
   parameter ``--hostname``, and the coordination server runs on the
   base port number minus one, thus defaulting to 4999. Furthermore, you
   can specify a party's listening port using ``--my-port``.

2. The parties read the information from a local file, which needs to
   be same everywhere. The file can be specified using
   ``--ip-file-name`` and has the following format::

     <host0>[:<port0>]
     <host1>[:<port1>]
     ...

   The hosts can be both hostnames and IP addresses. If not given, the
   ports default to base plus party number.
