# Multi-Protocol SPDZ

Software to benchmark various secure multi-party computation (MPC)
protocols such as SPDZ, MASCOT, Overdrive, BMR garbled circuits
(evaluation only), Yao's garbled circuits, and computation based on
semi-honest three-party replicated secret sharing (with an honest majority).

#### TL;DR (Binary Distribution on Linux or Source Distribution on macOS)

This requires either a Linux distribution originally released 2011 or
later (glibc 2.12) or macOS High Sierra or later as well as Python 2
and basic command-line utilities.

Download and unpack the [distribution](releases), then execute the following from
the top folder:

```
Scripts/tldr.sh
./compile.py tutorial
echo 1 2 3 > Player-Data/Input-P0-0
echo 1 2 3 > Player-Data/Input-P1-0
Scripts/mal-rep-field.sh tutorial
```

This runs [the tutorial](Programs/Source/tutorial.mpc) with three
parties, an honest majority, and malicious security.

#### TL;DR (Source Distribution)

On Linux, this requires a working toolchain and [all
requirements](#requirements). On Ubuntu, the following might suffice:
```
apt-get install automake build-essential git libboost-dev libboost-thread-dev libsodium-dev libssl-dev libtool m4 python texinfo yasm
```
On MacOS, this requires [brew](https://brew.sh) to be installed,
which will be used for all dependencies.
It will execute [the
tutorial](Programs/Source/tutorial.mpc) with three
parties, an honest majority, and malicious security.

```
make -j 8 tldr
./compile.py tutorial
Scripts/setup-replicated.sh
echo 1 2 3 > Player-Data/Input-P0-0
echo 1 2 3 > Player-Data/Input-P1-0
Scripts/mal-rep-field.sh tutorial
```

#### Preface

The primary aim of this software is to benchmark the same computation
in various protocols in order to compare the performance. In order to
do, it sometimes uses functionality that is not secure. Many MPC protocols
involve several phases that have to be executed in a secure manner for
the whole protocol to be sure. However, for benchmarking it does not
make a difference whether a previous phase was executed securely or
whether its output were generated insecurely. The focus on this
software is to benchmark each phases individually rather than running
the whole sequence of phases at once.

In order to make it clear where insecure functionality is used, it is
disabled by default but can be activated as explained in the section
on compilation. Some parts of the software will not work without doing so.

#### History

The software started out as an implementation of [the improved SPDZ
protocol](https://eprint.iacr.org/2012/642). The name SPDZ is derived
from the authors of the [original
protocol](https://eprint.iacr.org/2011/535).

This repository combines the functionality previously published in the
following repositories:
 - https://github.com/bristolcrypto/SPDZ-2
 - https://github.com/mkskeller/SPDZ-BMR-ORAM
 - https://github.com/mkskeller/SPDZ-Yao

#### Alternatives

There is another fork of SPDZ-2 called
[SCALE-MAMBA](https://github.com/KULeuven-COSIC/SCALE-MAMBA). It
focuses on providing an integrated system for computations modulo a
large prime, using the SPDZ protocol (based on lattic-based
homomorphic encryption) for a dishonest majority or using secret
sharing for an honest majority. More information can be found here:
https://homes.esat.kuleuven.be/~nsmart/SCALE

#### Overview

For the actual computation, the software implements a virtual machine
that executes programs in a specific bytecode. Such code can be
generated from high-level Python code using a compiler that optimizes
the computation with a particular focus on minimizing the number of
communication rounds (for protocol based on secret sharing) or on
AES-NI pipelining (for garbled circuits).

The software implements uses two different bytecode sets, one for
arithmetic circuits and one for boolean circuits. The high-level code
slightly differs between the two variants, but we aim to keep these
differences a at minimum.

The SPDZ protocol uses preprocessing, that is, in a first (sometimes
called offline) phase correlated randomness is generated independent
of the actual inputs of the computation. Only the second ("online")
phase combines this randomness with the actual inputs in order to
produce the desired results. The preprocessed data can only be used
once, thus more computation requires more preprocessing. MASCOT and
Overdrive are the names for two alternative preprocessing phases to go
with the SPDZ online phase.

In the section on computation we will explain how to run the SPDZ
online phase and the various honest-majority three-party comptuation as well
as BMR and Yao's garbled circuits.

The section on offline phases will then explain how to benchmark the
offline phases required for the SPDZ protocol. Running the online
phase outputs the amount of offline material required, which allows to
compute the preprocessing time for a particulor computation.

#### Requirements
 - GCC 4.8 or later (tested with 7.3) or LLVM (tested with 6.0)
 - MPIR library, compiled with C++ support (use flag --enable-cxx when running configure)
 - libsodium library, tested against 1.0.16
 - OpenSSL, tested against and 1.0.2 and 1.1.0
 - Boost.Asio with SSL support (`libboost-dev` on Ubuntu), tested against 1.65
 - Boost.Thread for BMR (`libboost-thread-dev` on Ubuntu), tested against 1.65
 - CPU supporting AES-NI, PCLMUL, AVX2
 - Python 2.x
 - NTL library for the SPDZ-2 and Overdrive offline phases (optional; tested with NTL 10.5)
 - If using macOS, Sierra or later (see comment about LLVM)

#### Compilation

1) Edit `CONFIG` or `CONFIG.mine` to your needs:

 - To benchmark malicious SPDZ or BMR, add the following line at the top: `MY_CFLAGS = -DINSECURE`
 - `PREP_DIR` should point to should be a local, unversioned directory to store preprocessing data (default is `Player-Data` in the current directory).
 - For the SPDZ-2 and Overdrive offline phases, set `USE_NTL = 1` and `MOD = -DMAX_MOD_SZ=6`.
 - To use GF(2^40), in particular for the SPDZ-2 offline phase, set `USE_GF2N_LONG = 0`. This will deactive anything that requires GF(2^128) such as MASCOT.

2) Run make to compile all the software (use the flag -j for faster
compilation multiple threads). See below on how to compile specific
parts only. Remember to run `make clean` first after changing `CONFIG`
or `CONFIG.mine`.

# Benchmarking computation

See `Programs/Source/` for some example MPC programs, in particular
`tutorial.mpc`.

## Arithmetic circuits

### SPDZ

All programs required in this section can be compiled with the target `online`:

`make -j 8 online`

#### To setup for benchmarking the online phase

This requires the INSECURE flag to be set before compilation as explained above. For a secure offline phase, see the section on SPDZ-2 below.

Run the command below. **If you haven't added `MY_CFLAGS = -DINSECURE` to `CONFIG.mine` before compiling, it will fail.**

`Scripts/setup-online.sh`

This sets up parameters for the online phase for 2 parties with a 128-bit prime field and 128-bit binary field, and creates fake offline data (multiplication triples etc.) for these parameters.

Parameters can be customised by running

`Scripts/setup-online.sh <nparties> <nbitsp> <nbits2>`


#### To compile a program

To compile for example the program in `./Programs/Source/tutorial.mpc`, run:

`./compile.py tutorial`

This creates the bytecode and schedule files in Programs/Bytecode/ and Programs/Schedules/

#### To run a program

To run the above program with two parties on one machine, run:

`./Player-Online.x -N 2 0 tutorial`

`./Player-Online.x -N 2 1 tutorial` (in a separate terminal)

Or, you can use a script to do the above automatically:

`Scripts/run-online.sh tutorial`

To run a program on two different machines, firstly the preprocessing data must be
copied across to the second machine (or shared using sshfs), and secondly, Player-Online.x
needs to be passed the machine where the first party is running.
e.g. if this machine is name `diffie` on the local network:

`./Player-Online.x -N 2 -h diffie 0 test_all`

`./Player-Online.x -N 2 -h diffie 1 test_all`

The software uses TCP ports around 5000 by default, use the `-pn`
argument to change that.

#### Compiling and running programs from external directories

Programs can also be edited, compiled and run from any directory with the above basic structure. So for a source file in `./Programs/Source/`, all SPDZ scripts must be run from `./`. The `setup-online.sh` script must also be run from `./` to create the relevant data. For example:

```
spdz$ cd ../
$ mkdir myprogs
$ cd myprogs
$ mkdir -p Programs/Source
$ vi Programs/Source/test.mpc
$ ../spdz/compile.py test.mpc
$ ls Programs/
Bytecode  Public-Input  Schedules  Source
$ ../spdz/Scripts/setup-online.sh
$ ls
Player-Data Programs
$ ../spdz/Scripts/run-online.sh test
```

### Honest-majority computation based on Shamir secret sharing

Compile the virtual machines:

`make -j 8 shamir`

Run setup to generate SSL keys and certificates. See the section replicated secret sharing for binary circuits below for details.

`Scripts/setup-ssl.sh <nparties>`

In order to compile a program, use `./compile.py`, for example:

`./compile.py tutorial`

Running the computation is similar to SPDZ but you will need to start
at least three parties:

`./malicious-shamir-party.x -N 3 -I 0 tutorial`

`./malicious-shamir-party.x -N 3 -I 1 tutorial` (in a separate terminal)

`./malicious-shamir-party.x -N 3 -I 2 tutorial` (in a separate terminal)

The `-I` enable interactive inputs, and in the tutorial party 0 and 1
will be asked to provide three numbers. Using
`./shamir-party.x` will provide semi-honest security instead
of malicious.

You can run all parties at once with

`Scripts/mal-shamir.sh tutorial`

for malicious security or

`Scripts/shamir.sh tutorial`

for semi-honest security. In this case, the inputs are read from
`Player-Data/Input-P<playerno>-0`.

### Three-party honest-majority computation modulo a prime

Compile the virtual machines:

`make -j 8 rep-field`

Run setup to generate a 128-bit prime. This will also generate SSL keys and certificates. See the section replicated secret sharing for binary circuits below for details.

`Scripts/setup-replicated.sh`

In order to compile a program, use `./compile.py`, for example:

`./compile.py tutorial`

Running the computation is similar to SPDZ but you will need to start
three parties:

`./malicious-rep-field-party.x -I 0 tutorial`

`./malicious-rep-field-party.x -I 1 tutorial` (in a separate terminal)

`./malicious-rep-field-party.x -I 2 tutorial` (in a separate terminal)

The `-I` enable interactive inputs, and in the tutorial party 0 and 1
will be asked to provide three numbers. Using
`./replicated-field-party.x` will provide semi-honest security instead
of malicious.

You can run all parties at once with

`Scripts/mal-rep-field.sh tutorial`

for malicious security or

`Scripts/rep-field.sh tutorial`

for semi-honest security. In this case, the inputs are read from
`Player-Data/Input-P<playerno>-0`.

### Semi-honest honest-majority computation modulo 2^64

Compile the necessary programs:

`make -j 8 rep-ring`

Run setup to generate SSL keys and certificates. See the section replicated secret sharing for binary circuits below for details.

`Scripts/setup-ssl.sh`

In order to compile a program, use `./compile.py -R 64`, for example:

`./compile.py -R 64 tutorial`

Then, run the three parties as follows:

`./replicated-ring-party.x -I 0 tutorial`

`./replicated-ring-party.x -I 1 tutorial` (in a separate terminal)

`./replicated-ring-party.x -I 2 tutorial` (in a separate terminal)

or

`Scripts/ring.sh tutorial`

Again, `-I` activates interactive input, otherwise inputs are read
from `Player-Data/Input-P<playerno>-0`.

## Binary circuits

For binary circuits, you can compile your programs giving the desired
integer length, for example:

`./compile.py -B 32 tutorial`

for using 32-bit integers with `sint` and 16/16-bit fixed-point
numbers for `sfix`. The latter is independent of the `-B` option and
can be changed with `sfix.set_precision`. See [the
tutorial](Programs/Source/tutorial.mpc).

Alternatively, you can directly use `sbitint.get_type(n)` and
`sbitfix` instead of `sint`and `sfix`, respectively.

### Honest-majority three-party computation

Compile the virtual machines:

`make -j 8 rep-bin`

Set up SSL certificate and keys:

`Scripts/setup-ssl.sh`

The programs expect the keys and certificates to be in `Player-Data/P<i>.key` and `Player-Data/P<i>.pem`, respectively, and the certificates to have the common name `P<i>` for player `<i>`. Furthermore, the relevant root certificates have to be in `Player-Data` such that OpenSSL can find them (run `c_rehash Player-Data`). The script above takes care of all this by generating self-signed certificates. Therefore, if you are running the programs on different hosts you will need to copy the certificate files.

After compilating the mpc file, run as follows:

`replicated-bin-party.x [-I] -h <host of party 0> -p <0/1/2> tutorial`

When running locally, you can omit the host argument. As above, `-I`
activates interactive input, otherwise inputs are read from
`Player-Data/Input-P<playerno>-0`.

The program above runs a semi-honest computation. For malicious
security you have to generate some preprocessing data (requires
compilation with the INSECURE flag):

`Scripts/setup-online.sh 3`

and then use `malicious-rep-bin-party.x` instead of
`replicated-bin-party.x`.

### Yao's garbled circuits

We use the implementation optimized for AES-NI by [Bellare et al.](https://eprint.iacr.org/2013/426)

Compile the virtual machine:

`make -j 8 yao`

After compilating the mpc file, run as follows:
  - Garbler: ```./yao-player.x [-I] -p 0 <program>```
  - Evaluator: ```./yao-player.x [-I] -p 1 -h <garbler host> <program>```

When running locally, you can omit the host argument. As above, `-I`
activates interactive input, otherwise inputs are read from
`Player-Data/Input-P<playerno>-0`.

By default, the circuit is garbled at once and stored on the evaluator
side before evaluating. You can activate a more continuous operation
by adding `-C` to the command line on both sides.

### BMR

This part has been developed to benchmark ORAM for the [Eurocrypt 2018
paper](https://eprint.iacr.org/2017/981) by Marcel Keller and Avishay
Yanay. It only allows to benchmark the data-dependent phase. The
data-independent and function-independent phases are emulated
insecurely.

By default, the implementations is optimized for two parties. You can
change this by defining `N_PARTIES` accordingly in `BMR/config.h`. If
you entirely delete the definition, it will be able to run for any
number of parties albeit slower.

Compile the virtual machine:

`make -j 8 bmr`

After compiling the mpc file:
- Run everything locally: `Scripts/bmr-program-run.sh <program>
<number of parties>`.
- Run on different hosts: `Scripts/bmr-program-run-remote.sh <program>
<host1> <host2> [...]`

#### Oblivious RAM

You can benchmark the ORAM implementation as follows:

1) Edit `Program/Source/gc_oram.mpc` to change size and to choose
Circuit ORAM or linear scan without ORAM. 
2) Run `./compile.py -D gc_oram`. The `-D` argument instructs the
compiler to remove dead code. This is useful for more complex programs
such as this one.
3) Run `gc_oram` in the virtual machines as explained above.

# Benchmarking offline phases

#### SPDZ-2 offline phase

This implementation is suitable to generate the preprocessed data used in the online phase.

It requires compilation with `USE_GF2N_LONG = 0` in `CONFIG` or `CONFIG.mine`. Remember to run `make clean` before recompiling.

For quick run on one machine, you can call the following:

`./spdz2-offline.x -p 0 & ./spdz2-offline.x -p 1`

More generally, run the following on every machine:

`./spdz2-offline.x -p <number of party> -N <total number of parties> -h <hostname of party 0> -c <covert security parameter>`

The number of parties are counted from 0. As seen in the quick example, you can omit the total number of parties if it is 2 and the hostname if all parties run on the same machine. Invoke `./spdz2-offline.x` for more explanation on the options.

`./spdz2-offline.x` provides covert security according to some parameter c (at least 2). A malicious adversary will get caught with probability 1-1/c. There is a linear correlation between c and the running time, that is, running with 2c takes twice as long as running with c. The default for c is 10.

The program will generate every kind of randomness required by the online phase until you stop it. You can shut it down gracefully pressing Ctrl-c (or sending the interrupt signal `SIGINT`), but only after an initial phase, the end of which is marked by the output `Starting to produce gf2n`. Note that the initial phase has been reported to take up to an hour. Furthermore, 3 GB of RAM are required per party.

#### Benchmarking the MASCOT offline phase

The MASCOT implementation is not suitable to generate the preprocessed data for the online phase because it can only generate either multiplication triples or bits. Nevertheless, an online computation only using data of one kind can run from the output of MASCOT offline phase if `Player-Online.x` is run with the options `-lg2 128 -lgp 128`.

In order to compile the MASCOT code, the following must be set in CONFIG or CONFIG.mine:

`USE_GF2N_LONG = 1`

If SPDZ has been built before with `USE_GF2N_LONG = 0`, any compiled code needs to be removed:

`make clean`

HOSTS must contain the hostnames or IPs of the players, see HOSTS.example for an example.

Then, MASCOT can be run as follows:

`host1:$ ./ot-offline.x -p 0 -c`

`host2:$ ./ot-offline.x -p 1 -c`

#### Benchmarking Overdrive offline phases

We have implemented several protocols to measure the maximal throughput for the [Overdrive paper](https://eprint.iacr.org/2017/1230). As for MASCOT, these implementations are not suited to generate data for the online phase because they only generate one type at a time.

Binary | Protocol
------ | --------
`simple-offline.x` | SPDZ-1 and High Gear (with command-line argument `-g`)
`pairwise-offline.x` | Low Gear
`cnc-offline.x` | SPDZ-2 with malicious security (covert security with command-line argument `-c`)

These programs can be run similarly to `spdz2-offline.x`, for example:

`host1:$ ./simple-offline.x -p 0 -h host1`

`host2:$ ./simple-offline.x -p 1 -h host1`

Running any program without arguments describes all command-line arguments.

##### Memory usage

Lattice-based ciphertexts are relatively large (in the order of megabytes), and the zero-knowledge proofs we use require storing some hundred of them. You must therefore expect to use at least some hundred megabytes of memory per thread. The memory usage is linear in `MAX_MOD_SZ` (determining the maximum integer size for computations in steps of 64 bits), so you can try to reduce it (see the compilation section for how set it). For some choices of parameters, 4 is enough while others require up to 8. The programs above indicate the minimum `MAX_MOD_SZ` required, and they fail during the parameter generation if it is too low.
