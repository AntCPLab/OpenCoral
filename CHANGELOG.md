The changelog explains changes pulled through from the private development repository. Bug fixes and small enhancements are committed between releases and not documented here.

## 0.1.8 (June 15, 2020)

- Half-gate garbling
- Native 2D convolution
- Inference with some TensorFlow graphs
- MASCOT with several MACs to increase security

## 0.1.7 (May 8, 2020)

- Possibility of using global keyword in loops instead of MemValue
- IEEE754 floating-point functionality using Bristol Fashion circuits

## 0.1.6 (Apr 2, 2020)

- Bristol Fashion circuits
- Semi-honest computation with somewhat homomorphic encryption
- Use SSL for client connections
- Client facilities for all arithmetic protocols

## 0.1.5 (Mar 20, 2020)

- Faster conversion between arithmetic and binary secret sharing using [extended daBits](https://eprint.iacr.org/2020/338)
- Optimized daBits
- Optimized logistic regression
- Faster compilation of repetitive code (compiler option `-C`)
- ChaiGear: [HighGear](https://eprint.iacr.org/2017/1230) with covert key generation
- [TopGear](https://eprint.iacr.org/2019/035) zero-knowledge proofs
- Binary computation based on Shamir secret sharing
- Fixed security bug: Prove correctness of ciphertexts in input tuple generation
- Fixed security bug: Missing check in MASCOT bit generation and various binary computations

## 0.1.4 (Dec 23, 2019)

- Mixed circuit computation with secret sharing
- Binary computation for dishonest majority using secret sharing as in [FKOS15](https://eprint.iacr.org/2015/901)
- Fixed security bug: insufficient OT correlation check in SPDZ2k
- This version breaks bytecode compatibilty.

## 0.1.3 (Nov 21, 2019)

- Python 3
- Semi-honest computation based on semi-homomorphic encryption
- Access to player information in high-level language

## 0.1.2 (Oct 11, 2019)

- Machine learning capabilities used for [MobileNets inference](https://eprint.iacr.org/2019/131) and the iDASH submission
- Binary computation for dishonest majority using secret sharing
- Mathematical functions from [SCALE-MAMBA](https://github.com/KULeuven-COSIC/SCALE-MAMBA)
- Fixed security bug: CowGear would reuse triples.

## 0.1.1 (Aug 6, 2019)

- ECDSA
- Loop unrolling with budget as in [HyCC](https://thomaschneider.de/papers/BDKKS18.pdf)
- Malicious replicated secret sharing for binary circuits
- New variants of malicious replicated secret over rings in [Use your Brain!](https://eprint.iacr.org/2019/164)
- MASCOT for any prime larger than 2^64
- Private fixed- and floating-point inputs

## 0.1.0 (Jun 7, 2019)

- CowGear protocol (LowGear with covert security)
- Protocols that sacrifice after than before
- More protocols for replicated secret sharing over rings
- Fixed security bug: Some protocols with supposed malicious security wouldn't check players' inputs when generating random bits.

## 0.0.9 (Apr 30, 2019)

- Complete BMR for all GF(2^n) protocols
- [Use your Brain!](https://eprint.iacr.org/2019/164)
- Semi/Semi2k for semi-honest OT-based computation
- Branching on revealed values in garbled circuits
- Fixed security bug: Potentially revealing too much information when opening linear combinations of private inputs in MASCOT and SPDZ2k with more than two parties

## 0.0.8 (Mar 28, 2019)

- SPDZ2k
- Integration of MASCOT and SPDZ2k preprocessing
- Integer division

## 0.0.7 (Feb 14, 2019)

- Simplified installation on macOS
- Optimized matrix multiplication
- Data type for quantization

## 0.0.6 (Jan 5, 2019)

- Shamir secret sharing

## 0.0.5 (Nov 5, 2018)

- More three-party replicated secret sharing
- Encrypted communication for replicated secret sharing

## 0.0.4 (Oct 11, 2018)

- Added BMR, Yao's garbled circuits, and semi-honest 3-party replicated secret sharing for arithmetic and binary circuits.
- Use inline assembly instead of MPIR for arithmetic modulo primes up length upt to 128 bit.
- Added a secure multiplication instruction to the instruction set in order to accommodate protocols that don't use Beaver randomization.

## 0.0.3 (Mar 2, 2018)

- Added offline phases based on homomorphic encryption, used in the [SPDZ-2 paper](https://eprint.iacr.org/2012/642) and the [Overdrive paper](https://eprint.iacr.org/2017/1230).
- On macOS, the minimum requirement is now Sierra.
- Compilation with LLVM/clang is now possible (tested with 3.8).

## 0.0.2 (Sep 13, 2017)

### Support sockets based external client input and output to a SPDZ MPC program.

See the [ExternalIO directory](./ExternalIO/README.md) for more details and examples.

Note that [libsodium](https://download.libsodium.org/doc/) is now a dependency on the SPDZ build. 

Added compiler instructions:

* LISTEN
* ACCEPTCLIENTCONNECTION
* CONNECTIPV4
* WRITESOCKETSHARE
* WRITESOCKETINT

Removed instructions:

* OPENSOCKET
* CLOSESOCKET
 
Modified instructions:

* READSOCKETC
* READSOCKETS
* READSOCKETINT
* WRITESOCKETC
* WRITESOCKETS

Support secure external client input and output with new instructions:

* READCLIENTPUBLICKEY
* INITSECURESOCKET
* RESPSECURESOCKET

### Read/Write secret shares to disk to support persistence in a SPDZ MPC program.

Added compiler instructions:

* READFILESHARE
* WRITEFILESHARE

### Other instructions

Added compiler instructions:

* DIGESTC - Clear truncated hash computation
* PRINTINT - Print register value

## 0.0.1 (Sep 2, 2016)

### Initial Release

* See `README.md` and `tutorial.md`.
