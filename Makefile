
include CONFIG

MATH = $(patsubst %.cpp,%.o,$(wildcard Math/*.cpp))

TOOLS = $(patsubst %.cpp,%.o,$(wildcard Tools/*.cpp))

NETWORK = $(patsubst %.cpp,%.o,$(wildcard Networking/*.cpp))

PROCESSOR = $(patsubst %.cpp,%.o,$(wildcard Processor/*.cpp))

FHEOFFLINE = $(patsubst %.cpp,%.o,$(wildcard FHEOffline/*.cpp FHE/*.cpp))

GC = $(patsubst %.cpp,%.o,$(wildcard GC/*.cpp)) $(PROCESSOR)

OT = $(patsubst %.cpp,%.o,$(filter-out OT/OText_main.cpp,$(wildcard OT/*.cpp)))
OT_EXE = ot.x ot-offline.x

COMMON = $(MATH) $(TOOLS) $(NETWORK)
COMPLETE = $(COMMON) $(PROCESSOR) $(FHEOFFLINE) $(TINYOTOFFLINE) $(GC) $(OT)
YAO = $(patsubst %.cpp,%.o,$(wildcard Yao/*.cpp)) $(OT) $(GC) BMR/Key.o
BMR = $(patsubst %.cpp,%.o,$(wildcard BMR/*.cpp BMR/network/*.cpp)) $(COMMON) $(PROCESSOR) $(OT)
VM = $(PROCESSOR) $(COMMON)


LIB = libSPDZ.a
LIBRELEASE = librelease.a
LIBSIMPLEOT = SimpleOT/libsimpleot.a

# used for dependency generation
OBJS = $(BMR) $(FHEOFFLINE) $(TINYOTOFFLINE) $(YAO) $(COMPLETE) $(patsubst %.cpp,%.o,$(wildcard Machines/*.cpp))
DEPS := $(wildcard */*.d)

# never delete
.SECONDARY: $(OBJS)


all: gen_input online offline externalIO bmr yao replicated shamir real-bmr spdz2k-party.x brain-party.x semi-party.x semi2k-party.x mascot-party.x

ifeq ($(USE_NTL),1)
all: overdrive she-offline cowgear-party.x
endif

-include $(DEPS)
include $(wildcard *.d static/*.d)

%.o: %.cpp
	$(CXX) -o $@ $< $(CFLAGS) -MMD -MP -c

online: Fake-Offline.x Server.x Player-Online.x Check-Offline.x

offline: $(OT_EXE) Check-Offline.x

gen_input: gen_input_f2n.x gen_input_fp.x

externalIO: client-setup.x bankers-bonus-client.x bankers-bonus-commsec-client.x

bmr: bmr-program-party.x bmr-program-tparty.x

real-bmr: $(patsubst Machines/%.cpp,%.x,$(wildcard Machines/*-bmr-party.cpp))

yao: yao-party.x

she-offline: Check-Offline.x spdz2-offline.x

overdrive: simple-offline.x pairwise-offline.x cnc-offline.x

rep-field: malicious-rep-field-party.x replicated-field-party.x ps-rep-field-party.x Setup.x

rep-ring: replicated-ring-party.x brain-party.x malicious-rep-ring-party.x ps-rep-ring-party.x Fake-Offline.x

rep-bin: replicated-bin-party.x malicious-rep-bin-party.x Fake-Offline.x

replicated: rep-field rep-ring rep-bin

spdz2k: spdz2k-party.x ot-offline.x Check-Offline-Z2k.x galois-degree.x Fake-Offline.x

tldr:
	-echo ARCH = -march=native >> CONFIG.mine
	$(MAKE) Player-Online.x

ifeq ($(OS), Darwin)
tldr: mac-setup
else
tldr: mpir
endif

shamir: shamir-party.x malicious-shamir-party.x galois-degree.x

ecdsa: $(patsubst ECDSA/%.cpp,%.x,$(wildcard ECDSA/*-ecdsa-party.cpp))
ecdsa-static: static-dir $(patsubst ECDSA/%.cpp,static/%.x,$(wildcard ECDSA/*-ecdsa-party.cpp))

$(LIBRELEASE): $(patsubst %.cpp,%.o,$(wildcard Machines/S*.cpp)) $(patsubst %.cpp,%.o,$(wildcard Protocols/*.cpp)) $(YAO) $(PROCESSOR) $(COMMON) $(BMR) $(FHEOFFLINE)
	$(AR) -csr $@ $^

static/%.x: Machines/%.o $(LIBRELEASE) $(LIBSIMPLEOT)
	$(CXX) $(CFLAGS) -o $@ $^ -Wl,-Map=$<.map -Wl,-Bstatic -static-libgcc -static-libstdc++ $(BOOST) $(LDLIBS) -Wl,-Bdynamic -ldl

static/%.x: ECDSA/%.o ECDSA/P256Element.o Machines/ShamirMachine.o $(VM) $(OT) $(LIBSIMPLEOT)
	$(CXX) $(CFLAGS) -o $@ $^ -Wl,-Map=$<.map -Wl,-Bstatic -static-libgcc -static-libstdc++ $(BOOST) $(LDLIBS) $(ECLIB) -Wl,-Bdynamic -ldl

static-dir:
	@ mkdir static 2> /dev/null; true

static-release: static-dir $(patsubst Machines/%.cpp, static/%.x, $(wildcard Machines/*-party.cpp))

Fake-Offline.x: Fake-Offline.cpp $(COMMON)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

Fake-ECDSA.x: ECDSA/Fake-ECDSA.cpp ECDSA/P256Element.o $(COMMON)
	$(CXX) -o $@ $^ $(CFLAGS) $(LDLIBS) $(ECLIB)

Check-Offline.x: Check-Offline.o $(COMMON) $(PROCESSOR)
	$(CXX) $(CFLAGS) -o Check-Offline.x $^ $(LDLIBS)

Check-Offline-Z2k.x: Check-Offline-Z2k.cpp $(COMMON)
	$(CXX) $(CFLAGS) -o Check-Offline-Z2k.x $^ $(LDLIBS)

Server.x: Server.cpp $(COMMON)
	$(CXX) $(CFLAGS) Server.cpp -o Server.x $(COMMON) $(LDLIBS)

Setup.x: Setup.cpp $(COMMON)
	$(CXX) $(CFLAGS) Setup.cpp -o Setup.x $(COMMON) $(LDLIBS)

ot.x: $(OT) $(COMMON) OT/OText_main.cpp $(LIBSIMPLEOT)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

ot-check.x: $(OT) $(COMMON)
	$(CXX) $(CFLAGS) -o ot-check.x OT/OutputCheck.cpp $(COMMON) $(LDLIBS)

ot-bitmatrix.x: $(OT) $(COMMON) OT/BitMatrixTest.cpp
	$(CXX) $(CFLAGS) -o ot-bitmatrix.x OT/BitMatrixTest.cpp OT/BitMatrix.o $(COMMON) $(LDLIBS)

ot-offline.x: $(OT) $(COMMON) ot-offline.cpp $(LIBSIMPLEOT)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

check-passive.x: $(COMMON) check-passive.cpp
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

gen_input_f2n.x: Scripts/gen_input_f2n.cpp $(COMMON)
	$(CXX) $(CFLAGS) Scripts/gen_input_f2n.cpp	-o gen_input_f2n.x $(COMMON) $(LDLIBS)

gen_input_fp.x: Scripts/gen_input_fp.cpp $(COMMON)
	$(CXX) $(CFLAGS) Scripts/gen_input_fp.cpp	-o gen_input_fp.x $(COMMON) $(LDLIBS)

gc-emulate.x: $(GC) $(COMMON) $(PROCESSOR) gc-emulate.cpp $(GC)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

bmr-%.x: $(BMR) Machines/bmr-%.cpp $(LIBSIMPLEOT)
	$(CXX) $(CFLAGS) -o $@ $^ $(BOOST) $(LDLIBS)

%-bmr-party.x: Machines/%-bmr-party.o $(BMR) $(LIBSIMPLEOT)
	$(CXX) $(CFLAGS) -o $@ $^ $(BOOST) $(LDLIBS)

bmr-clean:
	-rm BMR/*.o BMR/*/*.o GC/*.o

client-setup.x: client-setup.cpp $(COMMON)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

bankers-bonus-client.x: ExternalIO/bankers-bonus-client.cpp $(COMMON)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

bankers-bonus-commsec-client.x: ExternalIO/bankers-bonus-commsec-client.cpp $(COMMON)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

ifeq ($(USE_NTL),1)
simple-offline.x: $(COMMON) $(FHEOFFLINE) simple-offline.cpp
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

pairwise-offline.x: $(COMMON) $(FHEOFFLINE) pairwise-offline.cpp
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

cnc-offline.x: $(COMMON) $(FHEOFFLINE) cnc-offline.cpp
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

spdz2-offline.x: $(COMMON) $(FHEOFFLINE) spdz2-offline.cpp
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)
endif

yao-party.x: $(YAO)

yao-clean:
	-rm Yao/*.o

galois-degree.x: galois-degree.cpp
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

default-prime-length.x: default-prime-length.cpp
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

%.x: Machines/%.o $(VM) OT/OTTripleSetup.o OT/BaseOT.o $(LIBSIMPLEOT)
	$(CXX) -o $@ $(CFLAGS) $^ $(LDLIBS)

%-ecdsa-party.x: ECDSA/%-ecdsa-party.o ECDSA/P256Element.o $(VM)
	$(CXX) -o $@ $(CFLAGS) $^ $(LDLIBS) $(ECLIB)

replicated-bin-party.x: $(GC)
malicious-rep-bin-party.x: $(GC)
shamir-party.x: Machines/ShamirMachine.o
malicious-shamir-party.x: Machines/ShamirMachine.o
spdz2k-party.x: $(OT)
semi-party.x: $(OT)
semi2k-party.x: $(OT)
cowgear-party.x: $(FHEOFFLINE) Protocols/CowGearOptions.o
mascot-party.x: Machines/SPDZ.o $(OT)
Player-Online.x: Machines/SPDZ.o $(OT)
ps-rep-ring-party.x: Protocols/MalRepRingOptions.o
malicious-rep-ring-party.x: Protocols/MalRepRingOptions.o
mal-shamir-ecdsa-party.x: Machines/ShamirMachine.o
shamir-ecdsa-party.x: Machines/ShamirMachine.o
semi-ecdsa-party.x: $(OT) $(LIBSIMPLEOT)
mascot-ecdsa-party.x: $(OT) $(LIBSIMPLEOT)

$(LIBSIMPLEOT): SimpleOT/Makefile
	$(MAKE) -C SimpleOT

OT/BaseOT.o: SimpleOT/Makefile

SimpleOT/Makefile:
	git submodule update --init SimpleOT

.PHONY: mpir-setup mpir-global mpir
mpir-setup:
	git submodule update --init mpir
	cd mpir; \
	autoreconf -i; \
	autoreconf -i
	- $(MAKE) -C mpir clean

mpir-global: mpir-setup
	cd mpir; \
	./configure --enable-cxx;
	$(MAKE) -C mpir
	sudo $(MAKE) -C mpir install

mpir: mpir-setup
	cd mpir; \
	./configure --enable-cxx --prefix=$(CURDIR)/local
	$(MAKE) -C mpir install
	-echo MY_CFLAGS += -I./local/include >> CONFIG.mine
	-echo MY_LDLIBS += -Wl,-rpath -Wl,./local/lib -L./local/lib >> CONFIG.mine

mac-setup:
	brew install openssl boost libsodium mpir yasm ntl
	-echo MY_CFLAGS += -I/usr/local/opt/openssl/include >> CONFIG.mine
	-echo MY_LDLIBS += -L/usr/local/opt/openssl/lib >> CONFIG.mine
	-echo USE_NTL = 1 >> CONFIG.mine

clean:
	-rm */*.o *.o */*.d *.d *.x core.* *.a gmon.out */*/*.o static/*.x
