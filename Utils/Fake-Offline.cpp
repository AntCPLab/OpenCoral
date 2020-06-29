
#include "Math/gf2n.h"
#include "Math/gfp.h"
#include "Protocols/Share.h"
#include "Math/Setup.h"
#include "Protocols/Spdz2kShare.h"
#include "Protocols/BrainShare.h"
#include "Protocols/MaliciousRep3Share.h"
#include "Protocols/PostSacriRepRingShare.h"
#include "Protocols/PostSacriRepFieldShare.h"
#include "Protocols/SemiShare.h"
#include "Protocols/MaliciousShamirShare.h"
#include "Protocols/fake-stuff.h"
#include "Exceptions/Exceptions.h"
#include "GC/MaliciousRepSecret.h"
#include "GC/SemiSecret.h"
#include "GC/TinySecret.h"
#include "GC/TinierSecret.h"

#include "Math/Setup.h"
#include "Processor/Data_Files.h"
#include "Tools/mkpath.h"
#include "Tools/ezOptionParser.h"
#include "Tools/benchmarking.h"

#include "Protocols/fake-stuff.hpp"
#include "Processor/Data_Files.hpp"
#include "Math/Z2k.hpp"
#include "Math/gfp.hpp"
#include "GC/Secret.hpp"

#include <sstream>
#include <fstream>
using namespace std;


string prep_data_prefix;

void make_bit_triples(const gf2n& key,int N,int ntrip,Dtype dtype,bool zero)
{
  PRNG G;
  G.ReSeed();

  ofstream* outf=new ofstream[N];
  gf2n a,b,c, one;
  one.assign_one();
  vector<Share<gf2n> > Sa(N),Sb(N),Sc(N);
  /* Generate Triples */
  for (int i=0; i<N; i++)
    { stringstream filename;
      filename << get_prep_sub_dir<Share<gf2n>>(prep_data_prefix, N)
          << DataPositions::dtype_names[dtype] << "-2-P" << i;
      cout << "Opening " << filename.str() << endl;
      outf[i].open(filename.str().c_str(),ios::out | ios::binary);
      if (outf[i].fail()) { throw file_error(filename.str().c_str()); }
    }
  for (int i=0; i<ntrip; i++)
    {
      if (!zero)
        a.randomize(G);
      a.AND(a, one);
      make_share(Sa,a,N,key,G);
      if (!zero)
        b.randomize(G);
      if (dtype == DATA_BITTRIPLE)
        b.AND(b, one);
      make_share(Sb,b,N,key,G);
      c.mul(a,b);
      make_share(Sc,c,N,key,G);
      for (int j=0; j<N; j++)
        { Sa[j].output(outf[j],false);
          Sb[j].output(outf[j],false);
          Sc[j].output(outf[j],false);
        }
    }
  for (int i=0; i<N; i++)
    { outf[i].close(); }
  delete[] outf;
}


/* N      = Number players
 * ntrip  = Number tuples needed
 * str    = "2" or "p"
 */
template<class T>
void make_square_tuples(const typename T::mac_type& key,int N,int ntrip,const string& str,bool zero)
{
  (void) str;

  PRNG G;
  G.ReSeed();

  ofstream* outf=new ofstream[N];
  typename T::clear a,c;
  vector<T> Sa(N),Sc(N);
  /* Generate Squares */
  for (int i=0; i<N; i++)
    { stringstream filename;
      filename << get_prep_sub_dir<T>(prep_data_prefix, N) << "Squares-"
          << T::type_short() << "-P" << i;
      cout << "Opening " << filename.str() << endl;
      outf[i].open(filename.str().c_str(),ios::out | ios::binary);
      if (outf[i].fail()) { throw file_error(filename.str().c_str()); }
    }
  for (int i=0; i<ntrip; i++)
    {
      if (!zero)
        a.randomize(G);
      make_share(Sa,a,N,key,G);
      c.mul(a,a);
      make_share(Sc,c,N,key,G);
      for (int j=0; j<N; j++)
        { Sa[j].output(outf[j],false);
          Sc[j].output(outf[j],false);
        }
    }
  for (int i=0; i<N; i++)
    { outf[i].close(); }
  delete[] outf;
}

/* N      = Number players
 * ntrip  = Number bits needed
 */
template<class T>
void make_bits(const typename T::mac_type& key, int N, int ntrip, bool zero,
    int thread_num = -1)
{
  PRNG G;
  G.ReSeed();

  ofstream* outf=new ofstream[N];
  typename T::clear a;
  vector<T> Sa(N);
  /* Generate Bits */
  for (int i=0; i<N; i++)
    { stringstream filename;
      filename << get_prep_sub_dir<T>(prep_data_prefix, N) << "Bits-"
          << T::type_short() << "-P" << i
          << Sub_Data_Files<T>::get_suffix(thread_num);
      cout << "Opening " << filename.str() << endl;
      outf[i].open(filename.str().c_str(),ios::out | ios::binary);
      if (outf[i].fail()) { throw file_error(filename.str().c_str()); }
    }
  for (int i=0; i<ntrip; i++)
    { if ((G.get_uchar()&1)==0 || zero) { a.assign_zero(); }
      else                       { a.assign_one();  }
      make_share(Sa,a,N,key,G);
      for (int j=0; j<N; j++)
        { Sa[j].output(outf[j],false); }
    }
  for (int i=0; i<N; i++)
    { outf[i].close(); }
  delete[] outf;
}


/* N      = Number players
 * ntrip  = Number inputs needed
 * str    = "2" or "p"
 *
 */
template<class T>
void make_inputs(const typename T::mac_type& key,int N,int ntrip,const string& str,bool zero)
{
  (void) str;

  PRNG G;
  G.ReSeed();

  ofstream* outf=new ofstream[N];
  typename T::open_type a;
  vector<T> Sa(N);
  /* Generate Inputs */
  for (int player=0; player<N; player++)
    { for (int i=0; i<N; i++)
        { stringstream filename;
          filename << get_prep_sub_dir<T>(prep_data_prefix, N) << "Inputs-"
              << T::type_short() << "-P" << i << "-" << player;
          cout << "Opening " << filename.str() << endl;
          outf[i].open(filename.str().c_str(),ios::out | ios::binary);
          if (outf[i].fail()) { throw file_error(filename.str().c_str()); }
        }
      for (int i=0; i<ntrip; i++)
        {
          if (!zero)
            a.randomize(G);
          make_share(Sa,a,N,key,G);
          for (int j=0; j<N; j++)
            { Sa[j].output(outf[j],false); 
              if (j==player)
	        { a.output(outf[j],false);  }
            }
        }
      for (int i=0; i<N; i++)
        { outf[i].close(); }
    }
  delete[] outf;
}


template<class T>
void make_PreMulC(const typename T::mac_type& key, int N, int ntrip, bool zero)
{
  stringstream ss;
  ss << get_prep_sub_dir<T>(prep_data_prefix, N) << "PreMulC-" << T::type_short();
  Files<T> files(N, key, ss.str());
  PRNG G;
  G.ReSeed();
  typename T::clear a, b, c;
  c = 1;
  for (int i=0; i<ntrip; i++)
    {
      // close the circle
      if (i == ntrip - 1 || zero)
        a.assign_one();
      else
        do
          a.randomize(G);
        while (a.is_zero());
      files.output_shares(a);
      b = a;
      b.invert();
      files.output_shares(b);
      files.output_shares(a * c);
      c = b;
    }
}

template<class T>
void make_minimal(const typename T::mac_type& key, int nplayers, int nitems, bool zero)
{
    make_mult_triples<T>(key, nplayers, nitems, zero, prep_data_prefix);
    make_bits<T>(key, nplayers, nitems, zero);
    make_inputs<T>(key, nplayers, nitems, T::type_short(), zero);
}

template<class T>
void make_basic(const typename T::mac_type& key, int nplayers, int nitems, bool zero)
{
    make_minimal<T>(key, nplayers, nitems, zero);
    make_square_tuples<T>(key, nplayers, nitems, T::type_short(), zero);
    if (T::clear::invertible)
    {
        make_inverse<T>(key, nplayers, nitems, zero, prep_data_prefix);
        make_PreMulC<T>(key, nplayers, nitems, zero);
    }
}

template<class T>
int generate(ez::ezOptionParser& opt);

int main(int argc, const char** argv)
{
  insecure("preprocessing");
  bigint::init_thread();

  ez::ezOptionParser opt;

  opt.syntax = "./Fake-Offline.x <nplayers> [OPTIONS]\n\nOptions with 2 arguments take the form '-X <#gf2n tuples>,<#modp tuples>'";
  opt.example = "./Fake-Offline.x 2 -lgp 128 -lg2 128 --default 10000\n./Fake-Offline.x 3 -trip 50000,10000 -btrip 100000\n";

  opt.add(
        "128", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Bit length of GF(p) field (default: 128)", // Help description.
        "-lgp", // Flag token.
        "--lgp" // Flag token.
  );
  opt.add(
        to_string(gf2n::default_degree()).c_str(), // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        ("Bit length of GF(2^n) field (default: " + to_string(gf2n::default_degree()) + ")").c_str(), // Help description.
        "-lg2", // Flag token.
        "--lg2" // Flag token.
  );
  opt.add(
        "1000", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Default number of tuples to generate for ALL data types (default: 1000)", // Help description.
        "-d", // Flag token.
        "--default" // Flag token.
  );
  opt.add(
        "", // Default.
        0, // Required?
        2, // Number of args expected.
        ',', // Delimiter if expecting multiple args.
        "Number of triples, for gf2n / modp types", // Help description.
        "-trip", // Flag token.
        "--ntriples" // Flag token.
  );
  opt.add(
        "", // Default.
        0, // Required?
        2, // Number of args expected.
        ',', // Delimiter if expecting multiple args.
        "Number of random bits, for gf2n / modp types", // Help description.
        "-bit", // Flag token.
        "--nbits" // Flag token.
  );
  opt.add(
        "", // Default.
        0, // Required?
        2, // Number of args expected.
        ',', // Delimiter if expecting multiple args.
        "Number of input tuples, for gf2n / modp types", // Help description.
        "-inp", // Flag token.
        "--ninputs" // Flag token.
  );
  opt.add(
        "", // Default.
        0, // Required?
        2, // Number of args expected.
        ',', // Delimiter if expecting multiple args.
        "Number of square tuples, for gf2n / modp types", // Help description.
        "-sq", // Flag token.
        "--nsquares" // Flag token.
  );
  opt.add(
        "", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Number of inverse tuples (modp only)", // Help description.
        "-inv", // Flag token.
        "--ninverses" // Flag token.
  );
  opt.add(
        "", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Number of GF(2) triples", // Help description.
        "-btrip", // Flag token.
        "--nbittriples" // Flag token.
  );
  opt.add(
        "", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Number of GF(2) x GF(2^n) triples", // Help description.
        "-mixed", // Flag token.
        "--nbitgf2ntriples" // Flag token.
  );
  opt.add(
        "", // Default.
        0, // Required?
        0, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Set all values to zero, but not the shares", // Help description.
        "-z", // Flag token.
        "--zero" // Flag token.
  );
  opt.add(
        "", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "Generate for SPDZ2k with parameter", // Help description.
        "-Z", // Flag token.
        "--spdz2k" // Flag token.
  );
  opt.add(
        "", // Default.
        0, // Required?
        1, // Number of args expected.
        0, // Delimiter if expecting multiple args.
        "SPDZ2k security parameter (default: k)", // Help description.
        "-S", // Flag token.
        "--security" // Flag token.
  );
  opt.parse(argc, argv);

  if (opt.isSet("-Z"))
    {
      int k, s;
      opt.get("-Z")->getInt(k);
      s = k;
      if (opt.isSet("-S"))
        opt.get("-S")->getInt(s);
      if (k == 32 and s == 32)
        return generate<Spdz2kShare<32, 32>>(opt);
      else if (k == 64 and s == 64)
        return generate<Spdz2kShare<64, 64>>(opt);
      else if (k == 64 and s == 48)
        return generate<Spdz2kShare<64, 48>>(opt);
      else
        throw runtime_error("not compiled for k=" + to_string(k) + " and s=" + to_string(s));
    }
  else
    return generate<Share<gfp>>(opt);
}

template<class T>
int generate(ez::ezOptionParser& opt)
{
  vector<string> badOptions;
  string usage;
  unsigned int i;
  if(!opt.gotRequired(badOptions))
  {
    for (i=0; i < badOptions.size(); ++i)
      cerr << "ERROR: Missing required option " << badOptions[i] << ".";
    opt.getUsage(usage);
    cout << usage;
    return 1;
  }

  if(!opt.gotExpected(badOptions))
  {
    for(i=0; i < badOptions.size(); ++i)
      cerr << "ERROR: Got unexpected number of arguments for option " << badOptions[i] << ".";
    opt.getUsage(usage);
    cout << usage;
    return 1;
  }

  int nplayers;
  if (opt.firstArgs.size() == 2)
  {
    nplayers = atoi(opt.firstArgs[1]->c_str());
  }
  else if (opt.lastArgs.size() == 1)
  {
    nplayers = atoi(opt.lastArgs[0]->c_str());
  }
  else
  {
    cerr << "ERROR: invalid number of arguments\n";
    opt.getUsage(usage);
    cout << usage;
    return 1;
  }

  int default_num = 0;
  int ntrip2=0, ntripp=0, nbits2=0,nbitsp=0,nsqr2=0,nsqrp=0,ninp2=0,ninpp=0,ninv=0, nbittrip=0, nbitgf2ntrip=0;
  vector<int> list_options;
  int lg2, lgp;

  opt.get("--lgp")->getInt(lgp);
  opt.get("--lg2")->getInt(lg2);

  opt.get("--default")->getInt(default_num);
  ntrip2 = ntripp = nbits2 = nbitsp = nsqr2 = nsqrp = ninp2 = ninpp = ninv =
  nbittrip = nbitgf2ntrip = default_num;
  
  if (opt.isSet("--ntriples"))
  {
    opt.get("--ntriples")->getInts(list_options);
    ntrip2 = list_options[0];
    ntripp = list_options[1];
  }
  if (opt.isSet("--nbits"))
  {
    opt.get("--nbits")->getInts(list_options);
    nbits2 = list_options[0];
    nbitsp = list_options[1];
  }
  if (opt.isSet("--ninputs"))
  {
    opt.get("--ninputs")->getInts(list_options);
    ninp2 = list_options[0];
    ninpp = list_options[1];
  }
  if (opt.isSet("--nsquares"))
  {
    opt.get("--nsquares")->getInts(list_options);
    nsqr2 = list_options[0];
    nsqrp = list_options[1];
  }
  if (opt.isSet("--ninverses"))
    opt.get("--ninverses")->getInt(ninv);
  if (opt.isSet("--nbittriples"))
    opt.get("--nbittriples")->getInt(nbittrip);
  if (opt.isSet("--nbitgf2ntriples"))
    opt.get("--nbitgf2ntriples")->getInt(nbitgf2ntrip);

  bool zero = opt.isSet("--zero");
  if (zero)
      cout << "Set all values to zero" << endl;

  // check compatibility
  gf2n::init_field(lg2);

  PRNG G;
  G.ReSeed();
  prep_data_prefix = PREP_DIR;
  // Set up the fields
  T::clear::template generate_setup<T>(prep_data_prefix, nplayers, lgp);
  gfp::init_default(lgp);

  /* Find number players and MAC keys etc*/
  typename T::mac_type::Scalar keyp;
  gf2n key2;

  // create PREP_DIR if not there
  if (mkdir_p(PREP_DIR) == -1)
  {
    cerr << "mkdir_p(" PREP_DIR ") failed\n";
    throw file_error(PREP_DIR);
  }

  typedef Share<gf2n> sgf2n;

  generate_mac_keys<T>(keyp, nplayers, prep_data_prefix);
  generate_mac_keys<sgf2n>(key2, nplayers, prep_data_prefix);

  make_mult_triples<sgf2n>(key2,nplayers,ntrip2,zero,prep_data_prefix);
  make_mult_triples<T>(keyp,nplayers,ntripp,zero,prep_data_prefix);
  make_bits<Share<gf2n>>(key2,nplayers,nbits2,zero);
  make_bits<T>(keyp,nplayers,nbitsp,zero);
  make_square_tuples<sgf2n>(key2,nplayers,nsqr2,"2",zero);
  make_square_tuples<T>(keyp,nplayers,nsqrp,"p",zero);
  make_inputs<sgf2n>(key2,nplayers,ninp2,"2",zero);
  make_inputs<T>(keyp,nplayers,ninpp,"p",zero);
  make_inverse<sgf2n>(key2,nplayers,ninv,zero,prep_data_prefix);
  if (T::clear::invertible)
    make_inverse<T>(keyp,nplayers,ninv,zero,prep_data_prefix);
  make_bit_triples(key2,nplayers,nbittrip,DATA_BITTRIPLE,zero);
  make_bit_triples(key2,nplayers,nbitgf2ntrip,DATA_BITGF2NTRIPLE,zero);
  make_PreMulC<sgf2n>(key2,nplayers,ninv,zero);
  if (T::clear::invertible)
    make_PreMulC<T>(keyp,nplayers,ninv,zero);

  // replicated secret sharing only for three parties
  if (nplayers == 3)
  {
    make_bits<Rep3Share<Integer>>({}, nplayers, nbitsp, zero);
    make_basic<Rep3Share<gfp>>({}, nplayers, default_num, zero);
    make_basic<Rep3Share<gf2n>>({}, nplayers, default_num, zero);
    make_basic<BrainShare<64, 40>>({}, nplayers, default_num, zero);
    make_basic<MaliciousRep3Share<gf2n>>({}, nplayers, default_num, zero);
    make_basic<MaliciousRep3Share<gfp>>({}, nplayers, default_num, zero);
    make_bits<PostSacriRepRingShare<64, 40>>({}, nplayers, default_num, zero);
    make_bits<PostSacriRepFieldShare<gfp>>({}, nplayers, default_num, zero);
    make_bits<PostSacriRepFieldShare<gf2n>>({}, nplayers, default_num, zero);

    make_mult_triples<GC::MaliciousRepSecret>({}, nplayers, ntrip2, zero, prep_data_prefix);
    make_bits<GC::MaliciousRepSecret>({}, nplayers, nbits2, zero);
  }

  make_basic<SemiShare<gfp>>({}, nplayers, default_num, zero);
  make_basic<SemiShare<gf2n>>({}, nplayers, default_num, zero);

  make_mult_triples<GC::SemiSecret>({}, nplayers, default_num, zero, prep_data_prefix);
  make_bits<GC::SemiSecret>({}, nplayers, default_num, zero);

  Z2<41> keyt;
  generate_mac_keys<GC::TinySecret<40>>(keyt, nplayers, prep_data_prefix);

  make_minimal<GC::TinySecret<40>>(keyt, nplayers, default_num, zero);

  gf2n_short keytt;
  generate_mac_keys<GC::TinierSecret<gf2n_short>>(keytt, nplayers, prep_data_prefix);
  make_minimal<GC::TinierSecret<gf2n_short>>(keytt, nplayers, default_num, zero);

  make_basic<ShamirShare<gfp>>({}, nplayers, default_num, zero);
  make_basic<ShamirShare<gf2n>>({}, nplayers, default_num, zero);

  make_basic<MaliciousShamirShare<gfp>>({}, nplayers, default_num, zero);
  make_basic<MaliciousShamirShare<gf2n>>({}, nplayers, default_num, zero);

  return 0;
}
