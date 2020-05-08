
#include <fcntl.h>
#include <sodium.h>

#include "octetStream.h"
#include <string.h>
#include "Networking/sockets.h"
#include "Networking/ssl_sockets.h"
#include "Exceptions/Exceptions.h"
#include "Networking/data.h"
#include "Math/bigint.h"
#include "Tools/time-func.h"
#include "Tools/FlexBuffer.h"


void octetStream::reset()
{
    data = 0;
    len = mxlen = ptr = 0;
}

void octetStream::clear()
{
    if (data)
        delete[] data;
    data = 0;
    len = mxlen = ptr = 0;
}

void octetStream::assign(const octetStream& os)
{
  if (os.len>=mxlen)
    {
      if (data)
        delete[] data;
      mxlen=os.mxlen;  
      data=new octet[mxlen];
    }
  len=os.len;
  memcpy(data,os.data,len*sizeof(octet));
  ptr=os.ptr;
}


octetStream::octetStream(size_t maxlen)
{
  mxlen=maxlen; len=0; ptr=0;
  data=new octet[mxlen];
}

octetStream::octetStream(size_t len, const octet* source) :
    octetStream(len)
{
  append(source, len);
}

octetStream::octetStream(const octetStream& os)
{
  mxlen=os.mxlen;
  len=os.len;
  data=new octet[mxlen];
  memcpy(data,os.data,len*sizeof(octet));
  ptr=os.ptr;
}

octetStream::octetStream(FlexBuffer& buffer)
{
  mxlen = buffer.capacity();
  len = buffer.size();
  data = (octet*)buffer.data();
  ptr = buffer.ptr - buffer.data();
  buffer.reset();
}


void octetStream::hash(octetStream& output) const
{
  assert(output.mxlen >= crypto_generichash_blake2b_BYTES_MIN);
  crypto_generichash(output.data, crypto_generichash_BYTES_MIN, data, len, NULL, 0);
  output.len=crypto_generichash_BYTES_MIN;
}


octetStream octetStream::hash() const
{
  octetStream h(crypto_generichash_BYTES_MIN);
  hash(h);
  return h;
}


bigint octetStream::check_sum(int req_bytes) const
{
  unsigned char hash[req_bytes];
  crypto_generichash(hash, req_bytes, data, len, NULL, 0);

  bigint ans;
  bigintFromBytes(ans,hash,req_bytes);
  // cout << ans << "\n";
  return ans;
}


bool octetStream::equals(const octetStream& a) const
{
  if (len!=a.len) { return false; }
  return memcmp(data, a.data, len) == 0;
}


void octetStream::append_random(size_t num)
{
  resize(len+num);
  randombytes_buf(data+len, num);
  len+=num;
}


void octetStream::concat(const octetStream& os)
{
  resize(len+os.len);
  memcpy(data+len,os.data,os.len*sizeof(octet));
  len+=os.len;
}


void octetStream::store_bytes(octet* x, const size_t l)
{
  resize(len+4+l); 
  encode_length(data+len,l,4); len+=4;
  memcpy(data+len,x,l*sizeof(octet));
  len+=l;
}

void octetStream::get_bytes(octet* ans, size_t& length)
{
  length = get_int(4);
  memcpy(ans, consume(length), length * sizeof(octet));
}

void octetStream::store(int l)
{
  resize(len+4);
  encode_length(data+len,l,4);
  len+=4;
}


void octetStream::get(int& l)
{
  l = get_int(4);
}


void octetStream::store(const bigint& x)
{
  size_t num=numBytes(x);
  resize(len+num+5);

  (data+len)[0]=0;
  if (x<0) { (data+len)[0]=1; }
  len++;

  encode_length(data+len,num,4); len+=4;
  bytesFromBigint(data+len,x,num);
  len+=num;
}


void octetStream::get(bigint& ans)
{
  int sign = *consume(1);
  if (sign!=0 && sign!=1) { throw bad_value(); }

  long length = get_int(4);

  if (length!=0)
    {
      bigintFromBytes(ans, consume(length), length);
      if (sign)
        mpz_neg(ans.get_mpz_t(), ans.get_mpz_t());
    }
  else
    ans=0;
}


template<class T>
void octetStream::exchange(T send_socket, T receive_socket, octetStream& receive_stream) const
{
  send(send_socket, len, LENGTH_SIZE);
  size_t sent = 0, received = 0;
  bool length_received = false;
  size_t new_len = 0;
#ifdef TIME_ROUNDS
  Timer recv_timer;
#endif
  size_t n_iter = 0, n_send = 0;
  while (received < new_len or sent < len or not length_received)
    {
      n_iter++;
      if (sent < len)
        {
          n_send++;
          size_t to_send = len - sent;
          sent += send_non_blocking(send_socket, data + sent, to_send);
        }

      // avoid extra branching, false before length received
      if (received < new_len)
        {
          // if same buffer for sending and receiving
          // only receive up to already sent data
          // or when all is sent
          size_t to_receive = 0;
          if (sent == len or this != &receive_stream)
            to_receive = new_len - received;
          else if (sent > received)
            to_receive = sent - received;
          if (to_receive > 0)
            {
#ifdef TIME_ROUNDS
              TimeScope ts(recv_timer);
#endif
              if (sent < len)
                received += receive_non_blocking(receive_socket,
                    receive_stream.data + received, to_receive);
              else
                {
                  receive(receive_socket,
                      receive_stream.data + received, to_receive);
                  received += to_receive;
                }
            }
        }
      else if (not length_received)
        {
#ifdef TIME_ROUNDS
          TimeScope ts(recv_timer);
#endif
          octet blen[LENGTH_SIZE];
          size_t tmp = LENGTH_SIZE;
          if (sent < len)
            tmp = receive_all_or_nothing(receive_socket, blen, LENGTH_SIZE);
          else
            receive(receive_socket, blen, LENGTH_SIZE);
          if (tmp == LENGTH_SIZE)
            {
              new_len=decode_length(blen,sizeof(blen));
              receive_stream.resize(max(new_len, len));
              length_received = true;
            }
        }
    }

#ifdef TIME_ROUNDS
  cout << "Exchange time: " << recv_timer.elapsed() << " seconds and " << n_iter <<
          " iterations to receive "
      << 1e-3 * new_len << " KB, " << n_send
      << " iterations to send " << 1e-3 * len << " KB" << endl;
#endif
  receive_stream.len = new_len;
  receive_stream.reset_read_head();
}


void octetStream::input(istream& s)
{
  size_t size;
  s.read((char*)&size, sizeof(size));
  if (not s.good())
    throw IO_Error("not enough data");
  resize_min(size);
  s.read((char*)data, size);
  len = size;
  if (not s.good())
    throw IO_Error("not enough data");
}

void octetStream::output(ostream& s)
{
  s.write((char*)&len, sizeof(len));
  s.write((char*)data, len);
}

ostream& operator<<(ostream& s,const octetStream& o)
{
  for (size_t i=0; i<o.len; i++)
    { int t0=o.data[i]&15;
      int t1=o.data[i]>>4;
      s << hex << t1 << t0 << dec;
    }
  return s;
}


template void octetStream::exchange(int, int);
template void octetStream::exchange(ssl_socket*, ssl_socket*);
