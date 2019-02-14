
#include <fcntl.h>
#include <sodium.h>

#include "octetStream.h"
#include <string.h>
#include "Networking/sockets.h"
#include "Networking/ssl_sockets.h"
#include "Tools/sha1.h"
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
  for (size_t i=0; i<len; i++)
    { if (data[i]!=a.data[i]) { return false; } }
  return true;
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
  length=decode_length(data+ptr,4); ptr+=4;
  memcpy(ans,data+ptr,length*sizeof(octet));
  ptr+=length;
}

void octetStream::store(int l)
{
  resize(len+4);
  encode_length(data+len,l,4);
  len+=4;
}


void octetStream::get(int& l)
{
  l=decode_length(data+ptr,4);
  ptr+=4;
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
  int sign=(data+ptr)[0];
  if (sign!=0 && sign!=1) { throw bad_value(); }
  ptr++;

  long length=decode_length(data+ptr,4); ptr+=4;

  if (length!=0)
    { bigintFromBytes(ans, data+ptr, length);
      ptr+=length;
      if (sign)
        mpz_neg(ans.get_mpz_t(), ans.get_mpz_t());
    }
  else
    ans=0;
}


template<class T>
void octetStream::exchange(T send_socket, T receive_socket, octetStream& receive_stream)
{
  send(send_socket, len, LENGTH_SIZE);
  const size_t buffer_size = 100000;
  size_t sent = 0, received = 0;
  bool length_received = false;
  size_t new_len = 0;
#ifdef TIME_ROUNDS
  Timer recv_timer;
#endif
  while (received < new_len or sent < len or not length_received)
    {
      if (sent < len)
        {
          size_t to_send = min(buffer_size, len - sent);
          sent += send_non_blocking(send_socket, data + sent, to_send);
        }

      // avoid extra branching, false before length received
      if (received < new_len)
        {
          // same buffer for sending and receiving
          // only receive up to already sent data
          // or when all is sent
          size_t to_receive = 0;
          if (sent == len)
            to_receive = new_len - received;
          else if (sent > received)
            to_receive = sent - received;
          if (to_receive > 0)
            {
#ifdef TIME_ROUNDS
              TimeScope ts(recv_timer);
#endif
              received += receive_non_blocking(receive_socket,
                  receive_stream.data + received, to_receive);
            }
        }
      else if (not length_received)
        {
#ifdef TIME_ROUNDS
          TimeScope ts(recv_timer);
#endif
          octet blen[LENGTH_SIZE];
          if (receive_all_or_nothing(receive_socket,blen,LENGTH_SIZE) == LENGTH_SIZE)
            {
              new_len=decode_length(blen,sizeof(blen));
              receive_stream.resize(max(new_len, len));
              length_received = true;
            }
        }
    }

#ifdef TIME_ROUNDS
  cout << "Exchange time: " << recv_timer.elapsed() << " seconds to receive "
      << 1e-3 * new_len << " KB" << endl;
#endif
  receive_stream.len = new_len;
  receive_stream.reset_read_head();
}


void octetStream::store(const vector<int>& v)
{
  store(v.size());
  for (int x : v)
    store(x);
}


void octetStream::get(vector<int>& v)
{
  size_t size;
  get(size);
  v.resize(size);
  for (int& x : v)
    get(x);
}


// Construct the ciphertext as `crypto_secretbox(pt, counter||random)`
void octetStream::encrypt_sequence(const octet* key, uint64_t counter)
{
  octet nonce[crypto_secretbox_NONCEBYTES];
  int i;
  int message_len_bytes = len;
  randombytes_buf(nonce, sizeof nonce);
  if(counter == UINT64_MAX) {
      throw Processor_Error("Encryption would overflow counter. Too many messages.");
  } else {
      counter++;
  }
  for(i=0; i<8; i++) {
      nonce[i] = uint8_t ((counter >> (8*i)) & 0xFF);
  }
  int ciphertext_len = message_len_bytes + crypto_secretbox_MACBYTES;
  octet ciphertext[ciphertext_len];

  crypto_secretbox_easy(ciphertext, data, message_len_bytes, nonce, key);
  // append the ciphertext to an empty octet stream
  reset_read_head();
  reset_write_head();
  append(ciphertext, ciphertext_len*sizeof(octet));
  // append the nonce
  append(nonce, crypto_secretbox_NONCEBYTES * sizeof(octet));
}

void octetStream::decrypt_sequence(const octet* key, uint64_t counter)
{
  int ciphertext_len = len - crypto_box_NONCEBYTES;
  const octet *nonce = data + ciphertext_len;
  int i;
  uint64_t recvCounter=0;
  // Numbers are typically 24U + 16U so cast to int is safe.
  if (len < (int)(crypto_box_NONCEBYTES + crypto_secretbox_MACBYTES))
  {
    throw Processor_Error("Cannot decrypt octetStream: ciphertext too short");
  }
  for(i=7; i>=0; i--) {
      recvCounter |= ((uint64_t) *(nonce + i)) << (i*8);
  }
  if(recvCounter != counter + 1) {
      throw Processor_Error("Incorrect counter on stream.  Possible MITM.");
  }
  if (crypto_secretbox_open_easy(data, data, ciphertext_len, nonce, key) != 0)
  {
    throw Processor_Error("octetStream decryption failed!");
  }
  rewind_write_head(crypto_box_NONCEBYTES + crypto_secretbox_MACBYTES);
  //prepare for unpack after decryption by resetting the read head
  reset_read_head();
}

void octetStream::encrypt(const octet* key)
{
  octet nonce[crypto_secretbox_NONCEBYTES];
  randombytes_buf(nonce, sizeof nonce);
  int message_len_bytes = len;
  resize(len + crypto_secretbox_MACBYTES + crypto_secretbox_NONCEBYTES);

  // Encrypt data in-place
  crypto_secretbox_easy(data, data, message_len_bytes, nonce, key);
  // Adjust length to account for MAC, then append nonce
  len += crypto_secretbox_MACBYTES;
  append(nonce, sizeof nonce);
}

void octetStream::decrypt(const octet* key)
{
  int ciphertext_len = len - crypto_box_NONCEBYTES;
  // Numbers are typically 24U + 16U so cast to int is safe.
  if (len < (int)(crypto_box_NONCEBYTES + crypto_secretbox_MACBYTES))
  {
    throw Processor_Error("Cannot decrypt octetStream: ciphertext too short");
  }
  if (crypto_secretbox_open_easy(data, data, ciphertext_len, data + ciphertext_len, key) != 0)
  {
    throw Processor_Error("octetStream decryption failed!");
  }
  rewind_write_head(crypto_box_NONCEBYTES + crypto_secretbox_MACBYTES);
}

void octetStream::input(istream& s)
{
  size_t size;
  s.read((char*)&size, sizeof(size));
  resize_min(size);
  s.read((char*)data, size);
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
