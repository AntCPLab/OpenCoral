
include CONFIG

MATH = $(patsubst %.cpp,%.o,$(wildcard Math/*.cpp))

TOOLS = $(patsubst %.cpp,%.o,$(wildcard Tools/*.cpp))

NETWORK = $(patsubst %.cpp,%.o,$(wildcard Networking/*.cpp))

AUTH = $(patsubst %.cpp,%.o,$(wildcard Auth/*.cpp))

PROCESSOR = $(patsubst %.cpp,%.o,$(wildcard Processor/*.cpp))

ifeq ($(USE_NTL),1)
FHEOFFLINE = $(patsubst %.cpp,%.o,$(wildcard FHEOffline/*.cpp FHE/*.cpp))
endif

GC = $(patsubst %.cpp,%.o,$(wildcard GC/*.cpp)) $(PROCESSOR)

# OT needed by Yao
OT = OT/BaseOT.o OT/BitMatrix.o OT/BitVector.o OT/OTExtension.o OT/OTExtensionWithMatrix.o OT/Tools.o
# OT stuff needs GF2N_LONG, so only compile if this is enabled
ifeq ($(USE_GF2N_LONG),1)
OT = $(patsubst %.cpp,%.o,$(filter-out OT/OText_main.cpp,$(wildcard OT/*.cpp)))
OT_EXE = ot.x ot-offline.x
endif

COMMON = $(MATH) $(TOOLS) $(NETWORK) $(AUTH)
COMPLETE = $(COMMON) $(PROCESSOR) $(FHEOFFLINE) $(TINYOTOFFLINE) $(GC) $(OT)
YAO = $(patsubst %.cpp,%.o,$(wildcard Yao/*.cpp)) $(OT) $(GC)
BMR = $(patsubst %.cpp,%.o,$(wildcard BMR/*.cpp BMR/network/*.cpp)) $(COMMON) $(PROCESSOR)


LIB = libSPDZ.a
LIBSIMPLEOT = SimpleOT/libsimpleot.a

# used for dependency generation
OBJS = $(BMR) $(FHEOFFLINE) $(TINYOTOFFLINE) $(YAO) $(COMPLETE)
DEPS := $(OBJS:.o=.d)


all: gen_input online offline externalIO yao replicated

ifeq ($(USE_GF2N_LONG),1)
all: bmr
endif

ifeq ($(USE_NTL),1)
all: overdrive she-offline
endif

-include $(DEPS)

%.o: %.cpp
	$(CXX) $(CFLAGS) -MMD -c -o $@ $<

online: Fake-Offline.x Server.x Player-Online.x Check-Offline.x

offline: $(OT_EXE) Check-Offline.x

gen_input: gen_input_f2n.x gen_input_fp.x

externalIO: client-setup.x bankers-bonus-client.x bankers-bonus-commsec-client.x

bmr: bmr-program-party.x bmr-program-tparty.x

yao: yao-player.x

she-offline: Check-Offline.x spdz2-offline.x

overdrive: simple-offline.x pairwise-offline.x cnc-offline.x

rep-field: malicious-rep-bin-party.x replicated-field-party.x Setup.x

rep-ring: replicated-ring-party.x Fake-Offline.x

rep-bin: replicated-bin-party.x malicious-rep-bin-party.x Fake-Offline.x

replicated: rep-field rep-ring rep-bin

tldr: malicious-rep-field-party.x Setup.x

Fake-Offline.x: Fake-Offline.cpp $(COMMON) $(PROCESSOR)
	$(CXX) $(CFLAGS) -o $@ Fake-Offline.cpp $(COMMON) $(PROCESSOR) $(LDLIBS)

Check-Offline.x: Check-Offline.cpp $(COMMON) $(PROCESSOR) Auth/fake-stuff.hpp
	$(CXX) $(CFLAGS) Check-Offline.cpp -o Check-Offline.x $(COMMON) $(PROCESSOR) $(LDLIBS)

Server.x: Server.cpp $(COMMON)
	$(CXX) $(CFLAGS) Server.cpp -o Server.x $(COMMON) $(LDLIBS)

Player-Online.x: Player-Online.cpp $(COMMON) $(PROCESSOR)
	$(CXX) $(CFLAGS) Player-Online.cpp -o Player-Online.x $(COMMON) $(PROCESSOR) $(LDLIBS)

Setup.x: Setup.cpp $(COMMON)
	$(CXX) $(CFLAGS) Setup.cpp -o Setup.x $(COMMON) $(LDLIBS)

ifeq ($(USE_GF2N_LONG),1)
ot.x: $(OT) $(COMMON) OT/OText_main.cpp $(LIBSIMPLEOT)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

ot-check.x: $(OT) $(COMMON)
	$(CXX) $(CFLAGS) -o ot-check.x OT/BitVector.o OT/OutputCheck.cpp $(COMMON) $(LDLIBS)

ot-bitmatrix.x: $(OT) $(COMMON) OT/BitMatrixTest.cpp
	$(CXX) $(CFLAGS) -o ot-bitmatrix.x OT/BitMatrixTest.cpp OT/BitMatrix.o OT/BitVector.o $(COMMON) $(LDLIBS)

ot-offline.x: $(OT) $(COMMON) ot-offline.cpp $(LIBSIMPLEOT)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)
endif

check-passive.x: $(COMMON) check-passive.cpp
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

gen_input_f2n.x: Scripts/gen_input_f2n.cpp $(COMMON)
	$(CXX) $(CFLAGS) Scripts/gen_input_f2n.cpp	-o gen_input_f2n.x $(COMMON) $(LDLIBS)

gen_input_fp.x: Scripts/gen_input_fp.cpp $(COMMON)
	$(CXX) $(CFLAGS) Scripts/gen_input_fp.cpp	-o gen_input_fp.x $(COMMON) $(LDLIBS)

gc-emulate.x: $(GC) $(COMMON) $(PROCESSOR) gc-emulate.cpp $(GC)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

ifeq ($(USE_GF2N_LONG),1)
bmr-program-party.x: $(BMR) bmr-program-party.cpp
	$(CXX) $(CFLAGS) -o $@ $^ $(BOOST) $(LDLIBS)

bmr-program-tparty.x: $(BMR) bmr-program-tparty.cpp
	$(CXX) $(CFLAGS) -o $@ $^ $(BOOST) $(LDLIBS)
endif

bmr-clean:
	-rm BMR/*.o BMR/*/*.o GC/*.o

client-setup.x: client-setup.cpp $(COMMON) $(PROCESSOR)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

bankers-bonus-client.x: ExternalIO/bankers-bonus-client.cpp $(COMMON) $(PROCESSOR)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

bankers-bonus-commsec-client.x: ExternalIO/bankers-bonus-commsec-client.cpp $(COMMON) $(PROCESSOR)
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

yao-player.x: $(YAO) $(COMMON) yao-player.cpp $(LIBSIMPLEOT)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

yao-clean:
	-rm Yao/*.o

galois-degree.x: $(COMMON) galois-degree.cpp
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

replicated-bin-party.x: $(COMMON) $(GC) replicated-bin-party.cpp
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

malicious-rep-bin-party.x: $(COMMON) $(GC) malicious-rep-bin-party.cpp
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

replicated-ring-party.x: replicated-ring-party.cpp $(PROCESSOR) $(COMMON)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

replicated-field-party.x: replicated-field-party.cpp $(PROCESSOR) $(COMMON)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

malicious-rep-field-party.x: malicious-rep-field-party.cpp $(PROCESSOR) $(COMMON)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDLIBS)

$(LIBSIMPLEOT): SimpleOT/Makefile
	$(MAKE) -C SimpleOT

OT/BaseOT.o: SimpleOT/Makefile
	$(CXX) $(CFLAGS) -MMD -c -o $@ $(@:.o=.cpp)

SimpleOT/Makefile:
	git submodule update --init SimpleOT

.PHONY: mpir
mpir:
	git submodule update --init mpir
	cd mpir; \
	libtoolize --force; \
	aclocal; \
	autoheader; \
	libtoolize --force; \
	automake --force-missing --add-missing; \
	autoconf; \
	./configure --enable-cxx;
	$(MAKE) -C mpir
	sudo $(MAKE) -C mpir install

clean:
	-rm */*.o *.o */*.d *.d *.x core.* *.a gmon.out */*/*.o
