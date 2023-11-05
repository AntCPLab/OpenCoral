// Author: Zhicong Huang
#ifndef SILENT_OT_H
#define SILENT_OT_H

#include <emp-ot/cot.h>
#include <emp-ot/ferret/ferret_cot.h>
#include <emp-tool/utils/mitccrh.h>
#include <math.h>

#include <algorithm>
#include <stdexcept>
#include <atomic>

#include "Tools/performance.h"

inline void pack_cot_messages(uint64_t *y, uint64_t *corr_data, int ysize,
                              int bsize, int bitsize) {
  assert(y != nullptr && corr_data != nullptr);
  uint64_t start_pos = 0;
  uint64_t end_pos = 0;
  uint64_t start_block = 0;
  uint64_t end_block = 0;
  uint64_t temp_bl = 0;
  uint64_t mask = (1ULL << bitsize) - 1;
  if (bitsize == 64)
    mask = -1;

  uint64_t carriersize = 64;
  for (int i = 0; i < ysize; i++) {
    y[i] = 0;
  }
  for (int i = 0; i < bsize; i++) {
    start_pos = i * bitsize; // inclusive
    end_pos = start_pos + bitsize;
    end_pos -= 1; // inclusive
    start_block = start_pos / carriersize;
    end_block = end_pos / carriersize;
    if (carriersize == 64) {
      if (start_block == end_block) {
        y[start_block] ^= (corr_data[i] & mask) << (start_pos % carriersize);
      } else {
        temp_bl = (corr_data[i] & mask);
        y[start_block] ^= (temp_bl) << (start_pos % carriersize);
        y[end_block] ^= (temp_bl) >> (carriersize - (start_pos % carriersize));
      }
    }
  }
}

inline void unpack_cot_messages(uint64_t *corr_data, uint64_t *recvd, int bsize,
                                int bitsize) {
  assert(corr_data != nullptr && recvd != nullptr);
  uint64_t start_pos = 0;
  uint64_t end_pos = 0;
  uint64_t start_block = 0;
  uint64_t end_block = 0;
  uint64_t mask = (1ULL << bitsize) - 1;
  if (bitsize == 64)
    mask = -1;
  uint64_t carriersize = 64;

  for (int i = 0; i < bsize; i++) {
    start_pos = i * bitsize;
    end_pos = start_pos + bitsize - 1; // inclusive
    start_block = start_pos / carriersize;
    end_block = end_pos / carriersize;
    if (carriersize == 64) {
      if (start_block == end_block) {
        corr_data[i] = (recvd[start_block] >> (start_pos % carriersize)) & mask;
      } else {
        corr_data[i] = 0;
        corr_data[i] ^= (recvd[start_block] >> (start_pos % carriersize));
        corr_data[i] ^=
            (recvd[end_block] << (carriersize - (start_pos % carriersize)));
      }
    }
  }
}

    
template <typename IO>
class SilentOT {
public:
  static atomic<int> n_instances;
  emp::FerretCOT<IO>* ferret;
  emp::MITCCRH<8> mitccrh;

  SilentOT(int party, int threads, IO** ios, bool malicious = false,
           bool run_setup = true, std::string pre_file = "", bool warm_up = true) {
    ferret = new emp::FerretCOT<IO>(party, threads, ios, malicious, run_setup, emp::ferret_b13, pre_file);
    if (warm_up) {
        emp::block tmp;
        ferret->rcot(&tmp, 1);
    }
    if (party == emp::ALICE) {
        emp::block s;
        ferret->prg.random_block(&s, 1);
        ferret->io->send_block(&s, 1);
        ferret->mitccrh.setS(s);
        ferret->io->flush();
    }
    else {
        emp::block s;
        ferret->io->recv_block(&s, 1);
        ferret->mitccrh.setS(s);
    }

    n_instances++;
  }

  ~SilentOT() { delete ferret; }

  // chosen xor correlated message, chosen choice
  // Sender chooses one message 'corr'. A correlation is defined by the xor
  // function: f(x) = x XOR corr Sender receives a random message 'x' as output
  // ('data0').
  void send_ot_cxm_cc(emp::block* data0, const emp::block* corr, int64_t length) {
    emp::block* rcm_data = new emp::block[length];
    send_ot_rcm_cc(rcm_data, length);

    emp::block pad[2 * emp::ot_bsize];
    uint32_t corrected_bsize;
    emp::block corr_data[emp::ot_bsize];

    for (int64_t i = 0; i < length; i += emp::ot_bsize) {
      for (int64_t j = i; j < std::min(i + emp::ot_bsize, length); ++j) {
        pad[2 * (j - i)] = rcm_data[j];
        pad[2 * (j - i) + 1] = rcm_data[j] ^ ferret->Delta;
      }

      ferret->mitccrh.template hash<emp::ot_bsize, 2>(pad);

      for (int j = i; j < i + emp::ot_bsize and j < length; ++j) {
        data0[j] = pad[2 * (j - i)];
        corr_data[j - i] = corr[j] ^ data0[j] ^ pad[2 * (j - i) + 1];
      }
      corrected_bsize = std::min(emp::ot_bsize, length - i);
      ferret->io->send_data(corr_data, sizeof(emp::block) * corrected_bsize);
    }

    delete[] rcm_data;
  }

  // chosen xor correlated message, chosen choice
  // Receiver chooses a choice bit 'b', and
  // receives 'x' if b = 0, and 'x XOR corr' if b = 1
  void recv_ot_cxm_cc(emp::block* data, const bool* b, int64_t length) {
    emp::block* rcm_data = new emp::block[length];
    recv_ot_rcm_cc(rcm_data, b, length);

    emp::block pad[emp::ot_bsize];

    uint32_t corrected_bsize;
    emp::block corr_data[emp::ot_bsize];

    for (int64_t i = 0; i < length; i += emp::ot_bsize) {
      corrected_bsize = std::min(emp::ot_bsize, length - i);

      memcpy(pad, rcm_data + i, corrected_bsize * sizeof(emp::block));
      ferret->mitccrh.template hash<emp::ot_bsize, 1>(pad);

      ferret->io->recv_data(corr_data, sizeof(emp::block) * corrected_bsize);

      for (int j = i; j < i + emp::ot_bsize and j < length; ++j) {
        if (b[j])
          data[j] = corr_data[j - i] ^ pad[j - i];
        else
          data[j] = pad[j - i];
      }
    }
    delete[] rcm_data;
  }

  // chosen message, chosen choice
  void send_ot_cm_cc(const emp::block* data0, const emp::block* data1, int64_t length) {
    emp::block* data = new emp::block[length];
    send_ot_rcm_cc(data, length);

    emp::block pad[2 * emp::ot_bsize];
    for (int64_t i = 0; i < length; i += emp::ot_bsize) {
      for (int64_t j = i; j < std::min(i + emp::ot_bsize, length); ++j) {
        pad[2 * (j - i)] = data[j];
        pad[2 * (j - i) + 1] = data[j] ^ ferret->Delta;
      }
      // here, ferret depends on the template parameter "IO", making mitccrh
      // also dependent, hence we have to explicitly tell the compiler that
      // "hash" is a template function. See:
      // https://stackoverflow.com/questions/7397934/calling-template-function-within-template-class
      ferret->mitccrh.template hash<emp::ot_bsize, 2>(pad);
      for (int64_t j = i; j < std::min(i + emp::ot_bsize, length); ++j) {
        pad[2 * (j - i)] = pad[2 * (j - i)] ^ data0[j];
        pad[2 * (j - i) + 1] = pad[2 * (j - i) + 1] ^ data1[j];
      }
      ferret->io->send_data(pad, 2 * sizeof(emp::block) * std::min(emp::ot_bsize, length - i));
    }
    delete[] data;
  }

  // chosen message, chosen choice
  void recv_ot_cm_cc(emp::block* data, const bool* r, int64_t length) {
    recv_ot_rcm_cc(data, r, length);

    emp::block res[2 * emp::ot_bsize];
    emp::block pad[emp::ot_bsize];
    for (int64_t i = 0; i < length; i += emp::ot_bsize) {
      memcpy(pad, data + i, std::min(emp::ot_bsize, length - i) * sizeof(emp::block));
      ferret->mitccrh.template hash<emp::ot_bsize, 1>(pad);
      ferret->io->recv_data(res, 2 * sizeof(emp::block) * std::min(emp::ot_bsize, length - i));
      for (int64_t j = 0; j < emp::ot_bsize and j < length - i; ++j) {
        data[i + j] = res[2 * j + r[i + j]] ^ pad[j];
      }
    }
  }

  // random correlated message, chosen choice
  void send_ot_rcm_cc(emp::block* data0, int64_t length) {
    ferret->send_cot(data0, length);
  }

  // random correlated message, chosen choice
  void recv_ot_rcm_cc(emp::block* data, const bool* b, int64_t length) {
    ferret->recv_cot(data, b, length);
  }

  // random message, chosen choice
  void send_ot_rm_cc(emp::block* data0, emp::block* data1, int64_t length) {
    send_ot_rcm_cc(data0, length);

    emp::block pad[emp::ot_bsize * 2];
    for (int64_t i = 0; i < length; i += emp::ot_bsize) {
      for (int64_t j = i; j < std::min(i + emp::ot_bsize, length); ++j) {
        pad[2 * (j - i)] = data0[j];
        pad[2 * (j - i) + 1] = data0[j] ^ ferret->Delta;
      }
      ferret->mitccrh.template hash<emp::ot_bsize, 2>(pad);
      for (int64_t j = i; j < std::min(i + emp::ot_bsize, length); ++j) {
        data0[j] = pad[2 * (j - i)];
        data1[j] = pad[2 * (j - i) + 1];
      }
    }
  }

  // random message, chosen choice
  void recv_ot_rm_cc(emp::block* data, const bool* r, int64_t length) {
    recv_ot_rcm_cc(data, r, length);

    emp::block pad[emp::ot_bsize];
    for (int64_t i = 0; i < length; i += emp::ot_bsize) {
	  std::memcpy(pad, data + i, std::min(emp::ot_bsize, length - i) * sizeof(emp::block));
      ferret->mitccrh.template hash<emp::ot_bsize, 1>(pad);
	  std::memcpy(data + i, pad, std::min(emp::ot_bsize, length - i) * sizeof(emp::block));
    }
  }

  // random message, random choice
  void send_ot_rm_rc(emp::block* data0, emp::block* data1, int64_t length) {
    ferret->rcot(data0, length);

    emp::block pad[emp::ot_bsize * 2];
    for (int64_t i = 0; i < length; i += emp::ot_bsize) {
      for (int64_t j = i; j < std::min(i + emp::ot_bsize, length); ++j) {
        pad[2 * (j - i)] = data0[j];
        pad[2 * (j - i) + 1] = data0[j] ^ ferret->Delta;
      }
      ferret->mitccrh.template hash<emp::ot_bsize, 2>(pad);
      for (int64_t j = i; j < std::min(i + emp::ot_bsize, length); ++j) {
        data0[j] = pad[2 * (j - i)];
        data1[j] = pad[2 * (j - i) + 1];
      }
    }
  }

  // random message, random choice
  void recv_ot_rm_rc(emp::block* data, bool* r, int64_t length) {
    ferret->rcot(data, length);
    for (int64_t i = 0; i < length; i++) {
      r[i] = emp::getLSB(data[i]);
    }

    emp::block pad[emp::ot_bsize];
    for (int64_t i = 0; i < length; i += emp::ot_bsize) {
	  std::memcpy(pad, data + i, std::min(emp::ot_bsize, length - i) * sizeof(emp::block));
      ferret->mitccrh.template hash<emp::ot_bsize, 1>(pad);
	  std::memcpy(data + i, pad, std::min(emp::ot_bsize, length - i) * sizeof(emp::block));
    }
  }

  // random correlated message, random choice
  void send_ot_rcm_rc(emp::block* data, int64_t length) {
    ferret->rcot(data, length);
  }

  // random correlated message, random choice
  void recv_ot_rcm_rc(emp::block* data, bool* r, int64_t length) {
    ferret->rcot(data, length);
    for (int64_t i = 0; i < length; i++) {
      r[i] = emp::getLSB(data[i]);
    }
  }

  // EMP's COT API for compatibility
  void send_cot(emp::block * data, int64_t length) {
    ferret->send_cot(data, length);
  }

  // EMP's COT API for compatibility
	void recv_cot(emp::block* data, const bool * b, int64_t length) {
    ferret->recv_cot(data, b, length);
  }


  // Just for benchmark, not necessarily correct
  // chosen additive message, chosen choice
  // Sender chooses one message 'corr'. A correlation is defined by the addition
  // function: f(x) = x + corr Sender receives a random message 'x' as output
  // ('data0').
  void send_ot_cam_cc(uint64_t* data0, const uint64_t* corr, int64_t length,
                      int l) {
    uint64_t modulo_mask = (1ULL << l) - 1;
    if (l == 64) modulo_mask = -1;
    emp::block* rcm_data = new emp::block[length];
    send_ot_rcm_cc(rcm_data, length);

    emp::block pad[2 * emp::ot_bsize];
    uint32_t y_size = (uint32_t)ceil((emp::ot_bsize * l) / (float(64)));
    uint32_t corrected_y_size, corrected_bsize;
    uint64_t y[y_size];
    uint64_t corr_data[emp::ot_bsize];

    for (int64_t i = 0; i < length; i += emp::ot_bsize) {
      for (int64_t j = i; j < std::min(i + emp::ot_bsize, length); ++j) {
        pad[2 * (j - i)] = rcm_data[j];
        pad[2 * (j - i) + 1] = rcm_data[j] ^ ferret->Delta;
      }

      ferret->mitccrh.template hash<emp::ot_bsize, 2>(pad);

      for (int j = i; j < i + emp::ot_bsize and j < length; ++j) {
        data0[j] = _mm_extract_epi64(pad[2 * (j - i)], 0) & modulo_mask;
        corr_data[j - i] =
            (corr[j] + data0[j] + _mm_extract_epi64(pad[2 * (j - i) + 1], 0)) &
            modulo_mask;
      }
      corrected_y_size = (uint32_t)ceil((std::min(emp::ot_bsize, length - i) * l) /
                                        ((float)sizeof(uint64_t) * 8));
      corrected_bsize = std::min(emp::ot_bsize, length - i);

      pack_cot_messages(y, corr_data, corrected_y_size, corrected_bsize,
                             l);
      ferret->io->send_data(y, sizeof(uint64_t) * (corrected_y_size));
    }

    delete[] rcm_data;
  }

  // Just for benchmark, not necessarily correct
  // chosen additive message, chosen choice
  // Receiver chooses a choice bit 'b', and
  // receives 'x' if b = 0, and 'x + corr' if b = 1
  void recv_ot_cam_cc(uint64_t* data, const bool* b, int64_t length, int l) {
    uint64_t modulo_mask = (1ULL << l) - 1;
    if (l == 64) modulo_mask = -1;

    emp::block* rcm_data = new emp::block[length];
    recv_ot_rcm_cc(rcm_data, b, length);

    emp::block pad[emp::ot_bsize];

    uint32_t recvd_size = (uint32_t)ceil((emp::ot_bsize * l) / (float(64)));
    uint32_t corrected_recvd_size, corrected_bsize;
    uint64_t corr_data[emp::ot_bsize];
    uint64_t recvd[recvd_size];

    for (int64_t i = 0; i < length; i += emp::ot_bsize) {
      corrected_recvd_size =
          (uint32_t)ceil((std::min(emp::ot_bsize, length - i) * l) / (float(64)));
      corrected_bsize = std::min(emp::ot_bsize, length - i);

      memcpy(pad, rcm_data + i, std::min(emp::ot_bsize, length - i) * sizeof(emp::block));
      ferret->mitccrh.template hash<emp::ot_bsize, 1>(pad);

      ferret->io->recv_data(recvd, sizeof(uint64_t) * corrected_recvd_size);

      unpack_cot_messages(corr_data, recvd, corrected_bsize, l);

      for (int j = i; j < i + emp::ot_bsize and j < length; ++j) {
        if (b[j])
          data[j] = (corr_data[j - i] - _mm_extract_epi64(pad[j - i], 0)) &
                    modulo_mask;
        else
          data[j] = _mm_extract_epi64(pad[j - i], 0) & modulo_mask;
      }
    }

    delete[] rcm_data;
  }

};

template <typename IO>
atomic<int> SilentOT<IO>::n_instances = 0;

#endif // SILENT_OT_H