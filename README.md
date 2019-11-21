# Multi-Protocol SPDZ

Software to benchmark various secure multi-party computation (MPC)
protocols such as SPDZ, SPDZ2k, MASCOT, Overdrive, BMR garbled circuits,
Yao's garbled circuits, and computation based on
three-party replicated secret sharing as well as Shamir's secret
sharing (with an honest majority).

#### Contact

[Filing an issue on GitHub](../../issues) is the preferred way of contacting
us, but you can also write an email to mp-spdz@googlegroups.com
([archive](https://groups.google.com/forum/#!forum/mp-spdz)).

#### TL;DR (Binary Distribution on Linux or Source Distribution on macOS)

This requires either a Linux distribution originally released 2011 or
later (glibc 2.12) or macOS High Sierra or later as well as Python 3
and basic command-line utilities.

Download and unpack the [distribution](https://github.com/n1analytics/MP-SPDZ/releases),
then execute the following from
the top folder:

```
Scripts/tldr.sh
./compile.py tutorial
echo 1 2 3 4 > Player-Data/Input-P0-0
echo 1 2 3 4 > Player-Data/Input-P1-0
Scripts/mascot.sh tutorial
```

This runs [the tutorial](Programs/Source/tutorial.mpc) with two
parties parties and malicious security.

#### TL;DR (Source Distribution)

On Linux, this requires a working toolchain and [all
requirements](#requirements). On Ubuntu, the following might suffice:
```
apt-get install automake build-essential git libboost-dev libboost-thread-dev libsodium-dev libssl-dev libtool m4 python texinfo yasm
```
On MacOS, this requires [brew](https://brew.sh) to be installed,
which will be used for all dependencies.
It will execute [the
tutorial](Programs/Source/tutorial.mpc) with two parties and malicious
security.

```
make -j 8 tldr
./compile.py tutorial
echo 1 2 3 4 > Player-Data/Input-P0-0
echo 1 2 3 4 > Player-Data/Input-P1-0
Scripts/mascot.sh tutorial
```

#### Preface

The primary aim of this software is to run the same computation in
various protocols in order to compare the performance. All protocols
in the matrix below are fully implemented. In addition, there are
further protocols implemented only partially, most notably the
Overdrive protocols. They are deactivated by default in order to avoid
confusion over security. See the [section on compilation](#Compilation)
on how to activate them.

#### Protocols

The following table lists all protocols that are fully supported.

| Security model | Mod prime / GF(2^n) | Mod 2^k | Bin. SS | Garbling |
| --- | --- | --- | --- | --- |
| Malicious, dishonest majority | [MASCOT](#secret-sharing) | [SPDZ2k](#secret-sharing) | [Tiny](#secret-sharing) | [BMR](#bmr) |
| Covert, dishonest majority | [CowGear](#secret-sharing) | N/A | N/A | N/A |
| Semi-honest, dishonest majority | [Semi / Hemi](#secret-sharing) | [Semi2k](#secret-sharing) | [SemiBin](#secret-sharing) | [Yao's GC](#yaos-garbled-circuits) / [BMR](#bmr) |
| Malicious, honest majority | [Shamir / Rep3 / PS](#honest-majority) | [Brain / Rep3 / PS](#honest-majority) | [Rep3](#honest-majority) | [BMR](#bmr) |
| Semi-honest, honest majority | [Shamir / Rep3](#honest-majority) | [Rep3](#honest-majority) | [Rep3](#honest-majority) | [BMR](#bmr) |

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
[SCALE-MAMBA](https://github.com/KULeuven-COSIC/SCALE-MAMBA).
The main differences at the time of writing are as follows:
- It provides honest-majority computation for any Q2 structure.
- For dishonest majority computation, it provides integration of
SPDZ/Overdrive offline and online phases but without secure key
generation.
- It only provides computation modulo a prime.
- It only provides malicious security.

More information can be found here:
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

In the section on computation we will explain how to compile a
high-level program for the various computation domains and then how to
run it with different protocols.

The section on offline phases will then explain how to benchmark the
offline phases required for the SPDZ protocol. Running the online
phase outputs the amount of offline material required, which allows to
compute the preprocessing time for a particular computation.

#### Requirements
 - GCC 5 or later (tested with 8.2) or LLVM/clang 5 or later (tested with 7). We recommend clang because it performs better.
 - MPIR library, compiled with C++ support (use flag --enable-cxx when running configure)
 - libsodium library, tested against 1.0.16
 - OpenSSL, tested against and 1.0.2 and 1.1.0
 - Boost.Asio with SSL support (`libboost-dev` on Ubuntu), tested against 1.65
 - Boost.Thread for BMR (`libboost-thread-dev` on Ubuntu), tested against 1.65
 - 64-bit CPU
 - Python 3.5 or later
 - NTL library for CowGear and the SPDZ-2 and Overdrive offline phases (optional; tested with NTL 10.5)
 - If using macOS, Sierra or later

#### Compilation

1) Edit `CONFIG` or `CONFIG.mine` to your needs:

 - By default, a CPU supporting AES-NI, PCLMUL, AVX2, BMI2, ADX is
   required. This includes mainstream processors released 2014 or later.
   For older models you need to deactivate the respective
   extensions in the `ARCH` variable.
 - To benchmark online-only protocols or Overdrive, add the following line at the top: `MY_CFLAGS = -DINSECURE`
 - `PREP_DIR` should point to should be a local, unversioned directory to store preprocessing data (default is `Player-Data` in the current directory).
 - For CowGear and the SPDZ-2 and Overdrive offline phases, set `USE_NTL = 1`.

2) Run make to compile all the software (use the flag -j for faster
compilation multiple threads). See below on how to compile specific
parts only. Remember to run `make clean` first after changing `CONFIG`
or `CONFIG.mine`.

# Running computation

See `Programs/Source/` for some example MPC programs, in particular
`tutorial.mpc`.

### Compiling high-level programs

There are three computation domains, and the high-level programs have
to be compiled accordingly.

#### Arithmetic modulo a prime

```./compile.py [-F <integer bit length>] <program>```

The integer bit length defaults to 64.

Note that in this context integers do not wrap around as expected, so
it is the responsibility of the user to make sure that they don't grow
too large. If necessary `sint.Mod2m()` can be used to wrap around
manually.

The integer bit length together with the computation mandate a minimum
for the size of the prime, which will be output by the compiler. It is
also communicated to the virtual machine in the bytecode, which will
fail if the minimum is not met.

#### Arithmetic modulo 2^k

```./compile.py -R <integer bit length> <program>```

Currently, 64 is the only supported bit length, but it still has to be
specified for future compatibility.

#### Binary circuits

```./compile.py -B <integer bit length> <program>```

The integer length can be any number up to a maximum depending on the
protocol. All protocols support at least 64-bit integers.

Fixed-point numbers (`sfix`) always use 16/16-bit precision by default in
binary circuits. This can be changed with `sfix.set_precision`. See
[the tutorial](Programs/Source/tutorial.mpc).

If you would like to use integers of various precisions, you can use
`sbitint.get_type(n)` to get a type for `n`-bit arithmetic.

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

## Dishonest majority

Some full implementations require oblivious transfer, which is
implemented as OT extension based on
https://github.com/mkskeller/SimpleOT.

### Secret sharing

The following table shows all programs for dishonest-majority computation using secret sharing:

| Program | Protocol | Domain | Security | Script |
| --- | --- | --- | --- | --- |
| `mascot-party.x` | [MASCOT](https://eprint.iacr.org/2016/505) | Mod prime | Malicious | `mascot.sh` |
| `spdz2k-party.x` | [SPDZ2k](https://eprint.iacr.org/2018/482) | Mod 2^k | Malicious | `spdk2k.sh` |
| `semi-party.x` | OT-based | Mod prime | Semi-honest | `semi.sh` |
| `semi2k-party.x` | OT-based | Mod 2^k | Semi-honest | `semi2k.sh` |
| `cowgear-party.x` | Adapted [LowGear](https://eprint.iacr.org/2017/1230) | Mod prime | Covert | `cowgear.sh` |
| `hemi-party.x` | Semi-homomorphic encryption | Mod prime | Semi-honest | `hemi.sh` |
| `semi-bin-party.x` | OT-based | Binary | Semi-honest | `semi-bin.sh` |
| `tiny-party.x` | Adapted SPDZ2k | Binary | Malicious | `tiny.sh` |

Semi and Semi2k denote the result of stripping MASCOT/SPDZ2k of all
steps required for malicious security, namely amplifying, sacrificing,
MAC generation, and OT correlation checks. What remains is the
generation of additively shared Beaver triples using OT.

Similarly, SemiBin denotes a protocol that generates bit-wise
multiplication triples using OT without any element of malicious
security.

Tiny denotes the adaption of SPDZ2k to the binary setting. In
particular, the SPDZ2k sacrifice does not work for bits, so we replace
it by cut-and-choose according to [Furukawa et
al.](https://eprint.iacr.org/2016/944)

CowGear denotes a covertly secure version of LowGear. The reason for
this is the key generation that only achieves covert security. It is
possible however to run full LowGear for triple generation by using
`-s` with the desired security parameter.

Hemi denotes the stripped version version of LowGear for semi-honest
security similar to Semi, that is, generating additively shared Beaver
triples using semi-homomorphic encryption.

We will use MASCOT to demonstrate the use, but the other protocols
work similarly.

First compile the virtual machine:

`make -j8 mascot-party.x`

and a high-level program, for example the tutorial (use `-R 64` for
SPDZ2k and Semi2k and `-B <precision>` for SemiBin):

`./compile.py -F 64 tutorial`

To run the tutorial with two parties on one machine, run:

`./mascot-party.x -N 2 -I -p 0 tutorial`

`./mascot-party.x -N 2 -I -p 1 tutorial` (in a separate terminal)

Using `-I` activates interactive mode, which means that inputs are
solicitated from standard input, and outputs are given to any
party. Omitting `-I` leads to inputs being read from
`Player-Data/Input-P<party number>-0` in text format.

Or, you can use a script to do run two parties in non-interactive mode
automatically:

`Scripts/mascot.sh tutorial`

To run a program on two different machines, `mascot-party.x`
needs to be passed the machine where the first party is running,
e.g. if this machine is name `diffie` on the local network:

`./mascot-party.x -N 2 -h diffie 0 tutorial`

`./mascot-party.x -N 2 -h diffie 1 tutorial`

The software uses TCP ports around 5000 by default, use the `-pn`
argument to change that.

### Yao's garbled circuits

We use the implementation optimized for AES-NI by [Bellare et al.](https://eprint.iacr.org/2013/426)

Compile the virtual machine:

`make -j 8 yao`

and the high-level program:

`./compile.py -B <integer bit length> <program>`

Then run as follows:
  - Garbler: ```./yao-party.x [-I] -p 0 <program>```
  - Evaluator: ```./yao-party.x [-I] -p 1 -h <garbler host> <program>```

When running locally, you can omit the host argument. As above, `-I`
activates interactive input, otherwise inputs are read from
`Player-Data/Input-P<playerno>-0`.

By default, the circuit is garbled in chunks that are evaluated
whenever received.You can activate garbling all at once by adding
`-O` to the command line on both sides.

## Honest majority

The following table shows all programs for honest-majority computation:

| Program | Sharing | Domain | Malicious | \# parties | Script |
| --- | --- | --- | --- | --- | --- |
| `replicated-ring-party.x` | Replicated | Mod 2^k | N | 3 | `ring.sh` |
| `brain-party.x` | Replicated | Mod 2^k | Y | 3 | `brain.sh` |
| `ps-rep-ring-party.x` | Replicated | Mod 2^k | Y | 3 | `ps-rep-ring.sh` |
| `malicious-rep-ring-party.x` | Replicated | Mod 2^k | Y | 3 | `mal-rep-ring.sh` |
| `replicated-bin-party.x` | Replicated | Binary | N | 3 | `replicated.sh` |
| `malicious-rep-bin-party.x` | Replicated | Binary | Y | 3 | `mal-rep-bin.sh` |
| `replicated-field-party.x` | Replicated | Mod prime | N | 3 | `rep-field.sh` |
| `ps-rep-field-party.x` | Replicated | Mod prime | Y | 3 | `ps-rep-field.sh` |
| `malicious-rep-field-party.x` | Replicated | Mod prime | Y | 3 | `mal-rep-field.sh` |
| `shamir-party.x` | Shamir | Mod prime | N | 3 or more | `shamir.sh` |
| `malicious-shamir-party.x` | Shamir | Mod prime | Y | 3 or more | `mal-shamir.sh` |

We use the "generate random triple optimistically/sacrifice/Beaver"
methodology described by [Lindell and
Nof](https://eprint.iacr.org/2017/816) to achieve malicious
security, except for the "PS" (post-sacrifice) protocols where the
actual multiplication is executed optimistally and checked later as
also described by Lindell and Nof.
The implementations used by `brain-party.x`,
`malicious-rep-ring-party.x -S`, `malicious-rep-ring-party.x`,
and `ps-rep-ring-party.x` correspond to the protocols called DOS18
preprocessing (single), ABF+17 preprocessing, CDE+18 preprocessing,
and postprocessing, respectively,
by [Eerikson et al.](https://eprint.iacr.org/2019/164)
Otherwise, we use resharing by [Cramer et
al.](https://eprint.iacr.org/2000/037) for Shamir's secret sharing and
the optimized approach by [Araki et
al.](https://eprint.iacr.org/2016/768) for replicated secret sharing.

All protocols in this section require encrypted channels because the
information received by the honest majority suffices the reconstruct
all secrets. Therefore, an eavesdropper on the network could learn all
information.

MP-SPDZ uses OpenSSL for secure channels. You can generate the
necessary certificates and keys as follows:

`Scripts/setup-ssl.sh [<number of parties>]`

The programs expect the keys and certificates to be in
`Player-Data/P<i>.key` and `Player-Data/P<i>.pem`, respectively, and
the certificates to have the common name `P<i>` for player
`<i>`. Furthermore, the relevant root certificates have to be in
`Player-Data` such that OpenSSL can find them (run `c_rehash
Player-Data`). The script above takes care of all this by generating
self-signed certificates. Therefore, if you are running the programs
on different hosts you will need to copy the certificate files.

In the following, we will walk through running the tutorial modulo
2^k with three parties. The other programs work similarly.

First, compile the virtual machine:

`make -j 8 replicated-ring-party.x`

In order to compile a high-level program, use `./compile.py -R 64`:

`./compile.py -R 64 tutorial`

If using another computation domain, use `-F` or `-B` as described in
[the relevant section above](#compiling-high-level-programs).

Finally, run the three parties as follows:

`./replicated-ring-party.x -I 0 tutorial`

`./replicated-ring-party.x -I 1 tutorial` (in a separate terminal)

`./replicated-ring-party.x -I 2 tutorial` (in a separate terminal)

or

`Scripts/ring.sh tutorial`

The `-I` enable interactive inputs, and in the tutorial party 0 and 1
will be asked to provide three numbers. Otherwise, and when using the
script, the inputs are read from `Player-Data/Input-P<playerno>-0`.

When using programs based on Shamir's secret sharing, you can specify
the number of parties with `-N` and the maximum number of corrupted
parties with `-T`. The latter can be at most half the number of
parties.

### BMR

BMR (Bellare-Micali-Rogaway) is a method of generating a garbled circuit
using another secure computation protocol. We have implemented BMR
based on all available implementations using GF(2^128) because the nature
of this field particularly suits the Free-XOR optimization for garbled
circuits. Our implementation is based on the [SPDZ-BMR-ORAM
construction](https://eprint.iacr.org/2017/981). The following table
lists the available schemes.

| Program | Protocol | Dishonest Maj. | Malicious | \# parties | Script |
| --- | --- | --- | --- | --- | --- |
| `real-bmr-party.x` | MASCOT | Y | Y | 2 or more | `real-bmr.sh` |
| `shamir-bmr-party.x` | Shamir | N | N | 3 or more | `shamir-bmr.sh` |
| `mal-shamir-bmr-party.x` | Shamir | N | Y | 3 or more | `mal-shamir-bmr.sh` |
| `rep-bmr-party.x` | Replicated | N | N | 3 | `rep-bmr.sh` |
| `mal-rep-bmr-party.x` | Replicated | N | Y | 3 | `mal-rep-bmr.sh` |

In the following, we will walk through running the tutorial with BMR
based on MASCOT and two parties. The other programs work similarly.

First, compile the virtual machine. In order to run with more than
three parties, change the definition of `MAX_N_PARTIES` in
`BMR/config.h` accordingly.

`make -j 8 real-bmr-party.x`

In order to compile a high-level program, use `./compile.py -B`:

`./compile.py -B 32 tutorial`

Finally, run the two parties as follows:

`./real-bmr-party.x -I 0 tutorial`

`./real-bmr-party.x -I 1 tutorial` (in a separate terminal)

or

`Scripts/real-bmr.sh tutorial`

The `-I` enable interactive inputs, and in the tutorial party 0 and 1
will be asked to provide three numbers. Otherwise, and when using the
script, the inputs are read from `Player-Data/Input-P<playerno>-0`.

## Online-only benchmarking

In this section we show how to benchmark purely the data-dependent
(often called online) phase of some protocols. This requires to
generate the output of a previous phase insecurely. You will have to
(re)compile the software after adding `MY_CFLAGS = -DINSECURE` to
`CONFIG.mine` in order to run this insecure generation.

### SPDZ

The SPDZ protocol uses preprocessing, that is, in a first (sometimes
called offline) phase correlated randomness is generated independent
of the actual inputs of the computation. Only the second ("online")
phase combines this randomness with the actual inputs in order to
produce the desired results. The preprocessed data can only be used
once, thus more computation requires more preprocessing. MASCOT and
Overdrive are the names for two alternative preprocessing phases to go
with the SPDZ online phase.

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

### Honest-majority three-party computation of binary circuits with malicious security

Compile the virtual machines:

`make -j 8 rep-bin`

Generate preprocessing data:

`Scripts/setup-online.sh 3`

After compilating the mpc file, run as follows:

`malicious-rep-bin-party.x [-I] -h <host of party 0> -p <0/1/2> tutorial`

When running locally, you can omit the host argument. As above, `-I`
activates interactive input, otherwise inputs are read from
`Player-Data/Input-P<playerno>-0`.

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

## Benchmarking offline phases

#### SPDZ-2 offline phase

This implementation is suitable to generate the preprocessed data used in the online phase.

For quick run on one machine, you can call the following:

`./spdz2-offline.x -p 0 & ./spdz2-offline.x -p 1`

More generally, run the following on every machine:

`./spdz2-offline.x -p <number of party> -N <total number of parties> -h <hostname of party 0> -c <covert security parameter>`

The number of parties are counted from 0. As seen in the quick example, you can omit the total number of parties if it is 2 and the hostname if all parties run on the same machine. Invoke `./spdz2-offline.x` for more explanation on the options.

`./spdz2-offline.x` provides covert security according to some parameter c (at least 2). A malicious adversary will get caught with probability 1-1/c. There is a linear correlation between c and the running time, that is, running with 2c takes twice as long as running with c. The default for c is 10.

The program will generate every kind of randomness required by the online phase until you stop it. You can shut it down gracefully pressing Ctrl-c (or sending the interrupt signal `SIGINT`), but only after an initial phase, the end of which is marked by the output `Starting to produce gf2n`. Note that the initial phase has been reported to take up to an hour. Furthermore, 3 GB of RAM are required per party.

#### Benchmarking the MASCOT or SPDZ2k offline phase

These implementations are not suitable to generate the preprocessed
data for the online phase because they can only generate either
multiplication triples or bits.

HOSTS must contain the hostnames or IPs of the players, see HOSTS.example for an example.

Then, MASCOT can be run as follows:

`host1:$ ./ot-offline.x -p 0 -c`

`host2:$ ./ot-offline.x -p 1 -c`

For SPDZ2k, use `-Z <k>` to set the computation domain to Z_{2^k}, and
`-S` to set the security parameter. The latter defaults to k. At the
time of writing, the following combinations are available: 32/32,
64/64, 64/48, and 66/48.

Running `./ot-offline.x` without parameters give the full menu of
options such as how many items to generate in how many threads and
loops.

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
