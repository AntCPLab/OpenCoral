// Author: Zhicong Huang
#ifndef SILENT_OT_H
#define SILENT_OT_H

#include <emp-ot/cot.h>
#include <emp-ot/ferret/ferret_cot.h>
#include <emp-tool/utils/mitccrh.h>
#include <math.h>

#include <algorithm>
#include <stdexcept>

    
template <typename IO>
class SilentOT {
 public:
  emp::FerretCOT<IO>* ferret;
  emp::MITCCRH<8> mitccrh;

  SilentOT(int party, int threads, IO** ios, bool malicious = false,
           bool run_setup = true, std::string pre_file = "", bool warm_up = true) {
    ferret = new emp::FerretCOT<IO>(party, threads, ios, malicious, run_setup, emp::ferret_b13, pre_file);
    if (warm_up) {
      emp::block tmp;
      ferret->rcot(&tmp, 1);
    }
  }

  ~SilentOT() { delete ferret; }

  // chosen xor correlated message, chosen choice
  // Sender chooses one message 'corr'. A correlation is defined by the xor
  // function: f(x) = x XOR corr Sender receives a random message 'x' as output
  // ('data0').
  void send_ot_cxm_cc(emp::block* data0, const emp::block* corr, int64_t length) {
    emp::block* rcm_data = new emp::block[length];
    send_ot_rcm_cc(rcm_data, length);

    emp::block s;
    ferret->prg.random_block(&s, 1);
    ferret->io->send_block(&s, 1);
    ferret->mitccrh.setS(s);
    ferret->io->flush();

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
    emp::block s;
    ferret->io->recv_block(&s, 1);
    ferret->mitccrh.setS(s);

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

    emp::block s;
    ferret->prg.random_block(&s, 1);
    ferret->io->send_block(&s, 1);
    ferret->mitccrh.setS(s);
    ferret->io->flush();

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

    emp::block s;
    ferret->io->recv_block(&s, 1);
    ferret->mitccrh.setS(s);
    // ferret->io->flush();

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
    emp::block s;
    ferret->prg.random_block(&s, 1);
    ferret->io->send_block(&s, 1);
    ferret->mitccrh.setS(s);
    ferret->io->flush();

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
    emp::block s;
    ferret->io->recv_block(&s, 1);
    ferret->mitccrh.setS(s);
    // ferret->io->flush();
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

    emp::block s;
    ferret->prg.random_block(&s, 1);
    ferret->io->send_block(&s, 1);
    ferret->mitccrh.setS(s);
    ferret->io->flush();

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

    emp::block s;
    ferret->io->recv_block(&s, 1);
    ferret->mitccrh.setS(s);
    // ferret->io->flush();
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
};

#endif // SILENT_OT_H