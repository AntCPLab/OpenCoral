
#include "GC/RmfeMultiplier.h"
#include "Machines/Rmfe.hpp"
#include "Tools/debug.hpp"

void check_ole_result(const vector<BitMatrix>& output, const vector<BitMatrix>& input, const BitVector& choices, int n_choices,
    TwoPartyPlayer& P, OT_ROLE role) {
    if (role == SENDER) {
        octetStream os;
        for (size_t i = 0; i < input.size(); i++)
            input[i].pack(os);
        for (size_t i = 0; i < output.size(); i++)
            output[i].pack(os);
        P.send(os);
        P.receive(os);
        
        BitVector choices_(n_choices);
        choices_.unpack(os);

        vector<BitMatrix> other_output(output.size());
        for (size_t i = 0; i < other_output.size(); i++) {
            other_output[i].resize(output[i].vertical_size());
            other_output[i].unpack(os);
        }

        for (int i = 0; i < n_choices; i++) {
            for (size_t j = 0; j < other_output[i/128].squares.size(); j++) {
                emp::block blk = other_output[i/128].squares[j].rows[i%128];
                if (choices_.get_bit(i))
                    blk = blk ^ input[i/128].squares[j].rows[i%128];
                if (!emp::cmpBlock(&blk, &output[i/128].squares[j].rows[i%128], 1))
                    cout << "Error!" << endl;
            }
        }

    }
    else {
        octetStream os;
        choices.pack(os);
        for (size_t i = 0; i < output.size(); i++)
            output[i].pack(os);
        P.send(os);
        P.receive(os);
    }
}

void test_silent_ole(int argc, char** argv)
{
    // set up networking on localhost
    int my_number = atoi(argv[1]);
    int n_parties = atoi(argv[2]);
    int port_base = 9999;
    Names N(my_number, n_parties, "localhost", port_base);
    PlainPlayer P(N);
    TwoPartyPlayer* player = new VirtualTwoPartyPlayer(P, 1-my_number);

    Fole<RmfeShare> ole(player, BOTH);
    print_total_comm(P, "Before OLE");

    SeededPRNG prng;
    BitVector keyBits(225);
    keyBits.randomize(prng);
    ole.init(keyBits);
    
    int n = 128 * 100;
    vector<BitMatrix> senderInput(DIV_CEIL(keyBits.size(), 128));
    for (size_t i = 0; i < senderInput.size(); i++) {
        senderInput[i].resize(n);
        senderInput[i].randomize(prng);
    }

    if (my_number == 0)
        ole.set_role(SENDER);
    else   
        ole.set_role(RECEIVER);
    vector<BitMatrix> output;
    ole.correlate(output, senderInput, n);

    print_total_comm(P, "After OLE1");

    check_ole_result(output, senderInput, keyBits, keyBits.size(), *player, ole.get_role());
    cout << "Round 1 all good!" << endl;

    if (my_number == 0)
        ole.set_role(RECEIVER);
    else   
        ole.set_role(SENDER);
    ole.correlate(output, senderInput, n);

    print_total_comm(P, "After OLE2");

    check_ole_result(output, senderInput, keyBits, keyBits.size(), *player, ole.get_role());
    cout << "Round 2 all good!" << endl;

    delete player;
}

int main(int argc, char** argv)
{
    test_silent_ole(argc, argv);
}