#ifndef _Online_Thread
#define _Online_Thread

#include "Networking/Player.h"
#include "Math/gf2n.h"
#include "Math/gfp.h"
#include "Math/Integer.h"
#include "Processor/Data_Files.h"

#include <vector>
using namespace std;

template<class sint, class sgf2n> class Machine;

template<class sint, class sgf2n>
class thread_info
{
  public: 

  int thread_num;
  int covert;
  Names*  Nms;
  gf2n *alpha2i;
  typename sint::value_type *alphapi;
  int prognum;
  bool finished;
  bool ready;

  // rownums for triples, bits, squares, and inverses etc
  DataPositions pos;
  // Integer arg (optional)
  int arg;

  Machine<sint, sgf2n>* machine;

  static void* Main_Func(void *ptr);

  static void purge_preprocessing(Machine<sint, sgf2n>& machine);
};

#endif

