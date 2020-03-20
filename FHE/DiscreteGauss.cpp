
#include "DiscreteGauss.h"
#include "math.h"

void DiscreteGauss::set(double RR)
{
  if (RR > 0 or NewHopeB < 1)
    NewHopeB = max(1, int(round(2 * RR * RR)));
  assert(NewHopeB > 0);
}



/* This uses the approximation to a Gaussian via
 * binomial distribution
 *
 * This procedure consumes 2*NewHopeB bits
 *
 */
int DiscreteGauss::sample(PRNG &G, int stretch) const
{
  int s= 0;
  // stretch refers to the standard deviation
  int B = NewHopeB * stretch * stretch;
  for (int i = 0; i < B; i++)
    {
      s += G.get_bit();
      s -= G.get_bit();
    }
  return s;
}



void RandomVectors::set(int nn,int hh,double R)
{
  n=nn;
  if (h > 0)
    h=hh;
  DG.set(R);
}


 
vector<bigint> RandomVectors::sample_Gauss(PRNG& G, int stretch) const
{
  vector<bigint> ans(n);
  for (int i=0; i<n; i++)
    { ans[i]=DG.sample(G, stretch); }
  return ans;
}


vector<bigint> RandomVectors::sample_Hwt(PRNG& G) const
{
  if (h>n/2) { return sample_Gauss(G); }
  vector<bigint> ans(n);
  for (int i=0; i<n; i++) { ans[i]=0; }
  int cnt=0,j=0;
  unsigned char ch=0;
  while (cnt<h)
    { unsigned int i=G.get_uint()%n;
      if (ans[i]==0)
	{ cnt++;
          if (j==0)
            { j=8; 
              ch=G.get_uchar();
            }
          int v=ch&1; j--;
          if (v==0) { ans[i]=-1; }
          else      { ans[i]=1;  }
        }
    }
  return ans;
}




int sample_half(PRNG& G)
{
  int v=G.get_uchar()&3;
  if (v==0 || v==1)
    return 0;
  else if (v==2)
    return 1;
  else
    return -1;
}


vector<bigint> RandomVectors::sample_Half(PRNG& G) const
{
  vector<bigint> ans(n);
  for (int i=0; i<n; i++)
    ans[i] = sample_half(G);
  return ans;
}


vector<bigint> RandomVectors::sample_Uniform(PRNG& G,const bigint& B) const
{
  vector<bigint> ans(n);
  bigint v;
  for (int i=0; i<n; i++)
    { G.get_bigint(v, numBits(B));
      int bit=G.get_uint()&1;
      if (bit==0) { ans[i]=v; }
      else        { ans[i]=-v; }
    }
  return ans;
}

bool RandomVectors::operator!=(const RandomVectors& other) const
{
  if (n != other.n or h != other.h or DG != other.DG)
    return true;
  else
    return false;
}

bool DiscreteGauss::operator!=(const DiscreteGauss& other) const
{
  if (other.NewHopeB != NewHopeB)
    return true;
  else
    return false;
}
