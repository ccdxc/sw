// -*- C++ -*-
//************************************************************
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved.
//************************************************************
#ifndef _PEN_AXI4_SLAVE_H_
#define _PEN_AXI4_SLAVE_H_
// This file defines the class to implement an AXI4 slave that accepts
// axi transactions and produces AXI responses. It instantiates a
// memory to hold the data. It can optionally provide out of order
// following AXI ordering rules.
// It can have a list of downstream slaves and provides an address
// decoder to

// The interface for the slave is
//  create slave to accept a range of addresses.
//  add ds slave specify address range. this address range should be
//  fully contained in this slave.
//  R channel:
//     read(axi_ar_bundle)
//     read_resp(axi_r_bundle)
//  W channel
//     write(axi_aw_bundle)
//     write_data(axi_w_bundle)
//     write_rsp(axi_b_bundle)

#include "pen_axi_decoders.h"
#include "pen_mem.h"
#include <deque>
#include <vector>
#include <memory>
#include <pknobs.h>
#include <unordered_map>
#include "pen_axi_info.h"
#include "LogMsg.h"
#define AXI_BURST_FIXED 0
#define AXI_BURST_INCR  1
#define AXI_BURST_WRAP  2
#define AXI_BURST_RSVD  3

// Abstract base class for chaining.
typedef std::function<pen_mem_base *(uint64_t, uint64_t&, bool)>pen_addr_decoder_t;

class pen_addr_filter {
public:
  enum perms_e {
    NONE  = 0,
    READ  = 1 << 0,
    WRITE = 1 << 1,
    ALL   = READ|WRITE
  };
private:
  struct item {
    uint64_t start;
    uint64_t end;
    perms_e  perms;
    bool     assert_p;
    item (uint64_t s, uint64_t e, perms_e p, bool a) :
      start(s),
      end(e),
      perms(p),
      assert_p(a)
      {}
  };
  std::vector<item> filter_list;
  std::string prefix;
public:
  pen_addr_filter(const std::string & pref) : prefix(pref) {}
  void add_filter(uint64_t start, uint64_t end, perms_e perms, bool assert_p) {
    filter_list.emplace_back(start,end,perms,assert_p);
  }
  inline bool check_perm(uint64_t addr, uint32_t len, perms_e acc, bool & assert_p) {
    perms_e p = ALL;
    uint64_t last = addr+len-1;
    for (auto const &i : filter_list) {
      if ((i.start <= addr && addr <= i.end) || // Conservative.
	  (i.start <= last && last <= i.end)) {
	p = (perms_e) (p & i.perms);
        if ((p&acc) == 0) {
          assert_p = i.assert_p;
	  return false;
        }
      }
    }
    return (true);
  }
  void read_from_cfg();
};

class pen_axi4_slave_base {
public:

  bool save_write_xn = false;
  axi_xn_q axi_write_q;
  std::string name;
  pen_addr_filter addr_filter;
  axi_info_t * get_next_write_xn() {
    axi_info_t * p = nullptr;
    if (axi_write_q.size()) {
      p = axi_write_q.front();
      axi_write_q.pop_front();
    }
    return p;
  }

  pen_axi4_slave_base(const std::string&n) : name(n), addr_filter(n) {addr_filter.read_from_cfg();}
  virtual ~pen_axi4_slave_base() {}
  virtual void read      (pen_csr_base * axi_ar) = 0;
  virtual bool read_resp (std::vector<pen_csr_base *> & axi_ar) = 0;
  virtual void write     (pen_csr_base * axi_ar) = 0;
  virtual void write_data(pen_csr_base * axi_ar) = 0;
  virtual void write_resp(pen_csr_base *& axi_ar) = 0;
};

template <class AR = pen_axi_ar_bundle_t,
	  class  R = pen_axi_r_bundle_t,
	  class AW = pen_axi_aw_bundle_t,
	  class  W = pen_axi_w_bundle_t,
	  class  B = pen_axi_b_bundle_t>
class pen_axi4_slave_t : public pen_axi4_slave_base {
  pen_addr_decoder_t addr_decoder;
  uint64_t start_addr, end_addr;

  uint64_t get_xfer_size(uint64_t len, uint64_t burst, uint64_t size) {
    if (burst == AXI_BURST_FIXED) {
      return 1ull<<size;
    } else if (burst != AXI_BURST_RSVD) {
      return (len+1)*(1ull<<size);
    } else {
      return 0;
    }
  }

  uint64_t get_start_addr(uint64_t addr, uint64_t burst, uint64_t xfer_size) {
    if (burst == AXI_BURST_WRAP) {
      return addr - (addr % xfer_size);
    } else {
      return addr;
    }
  }

  bool addr_overlap(cpp_int addr0, cpp_int len0, cpp_int burst0, cpp_int size0,
		    cpp_int addr1, cpp_int len1, cpp_int burst1, cpp_int size1)
  {
    auto uaddr0 = addr0.convert_to<uint64_t>();
    auto uaddr1 = addr1.convert_to<uint64_t>();
    auto ulen0  = len0.convert_to<uint64_t>();
    auto ulen1  = len1.convert_to<uint64_t>();
    auto uburst0 = burst0.convert_to<uint64_t>();
    auto uburst1 = burst1.convert_to<uint64_t>();
    auto usize0 = size0.convert_to<uint64_t>();
    auto usize1 = size1.convert_to<uint64_t>();

    auto xfer_size0 = get_xfer_size(ulen0, uburst0, usize0);
    auto xfer_size1 = get_xfer_size(ulen1, uburst1, usize1);
    auto start_addr0 = get_start_addr(uaddr0, uburst0, xfer_size0);
    auto start_addr1 = get_start_addr(uaddr1, uburst1, xfer_size1);
    auto end_addr0 = start_addr0 + xfer_size0 - 1;
    auto end_addr1 = start_addr1 + xfer_size1 - 1;
    if ((start_addr0 <= start_addr1 && end_addr0 >= start_addr1) ||
	(start_addr1 <= start_addr0 && end_addr1 >= start_addr0))
      return true;
    else
      return false;
  }

  template<class T, class U>
  bool pen_axi_can_pass(T *first,
			U *second) {
    if (first->id() == second->id())
      return false;
    else if (addr_overlap(first->addr(), first->len(), first->burst(), first->sz(),
			  second->addr(), second->len(), second->burst(), second->sz()))
      return false;
    else
      return true;
  }


  struct req_t {
    pen_csr_base * req;
    enum req_type_t { READ, WRITE } req_type;
    req_t(pen_csr_base *r, req_type_t t) :
      req(r), req_type(t) {}
    AR * get_read_req()  const {
      return dynamic_cast<AR *>(req);
    }
    AW * get_write_req() const {
      return dynamic_cast<AW *>(req);
    }
  };

  bool can_pass(const req_t & first, const req_t & second) {
    if (first.req_type == req_t::READ && second.req_type == req_t::READ) {
      return pen_axi_can_pass(first.get_read_req(), second.get_read_req());
    } else if (first.req_type == req_t::READ && second.req_type == req_t::WRITE) {
      return pen_axi_can_pass(first.get_read_req(), second.get_write_req());
    } else if (first.req_type == req_t::WRITE && second.req_type == req_t::READ) {
      return pen_axi_can_pass(first.get_write_req(), second.get_read_req());
    } else if (first.req_type == req_t::WRITE && second.req_type == req_t::WRITE) {
      return pen_axi_can_pass(first.get_write_req(), second.get_write_req());
    }
    return false;
  }
    pen_mem_base * get_mem(uint64_t addr_in, uint64_t& addr_out, bool is_read)
    {
	return addr_decoder(addr_in, addr_out, is_read);
    }


  std::vector<req_t> requests;
  struct write_id_len {
    uint32_t id;
    uint32_t len;
    write_id_len(uint32_t a, uint32_t b) : id(a), len(b) {}
  };
  std::deque<write_id_len> write_ids;
  std::vector<req_t> ready_requests;

  std::unordered_map<uint64_t, std::deque<W*> > write_data_map;

  pknobs::RKnob chooser;

public:
    pen_axi4_slave_t(const std::string& _name, pen_addr_decoder_t _addr_decoder, uint64_t start, uint64_t end, bool in_order = false )
      : pen_axi4_slave_base(_name),
	addr_decoder(_addr_decoder)
  {
    start_addr = start;
    end_addr   = end;
  }
  virtual ~pen_axi4_slave_t() {}
  virtual void read (pen_csr_base * axi_ar) {

    // Check if the read req can be converted.
    auto ar = dynamic_cast<AR *>(axi_ar);
    assert(ar);
    requests.emplace_back(ar, req_t::READ);
    update_ready_req();
  }

  uint64_t get_axi_addr(uint64_t start_addr,
			uint64_t burst_num,
			uint64_t burst_type,
			uint64_t num_bytes,
			uint64_t axi_len) {
    auto burst_len = axi_len + 1;
    if (burst_type == AXI_BURST_FIXED || burst_num == 0)
      return start_addr;

    auto aligned_addr = start_addr & ~(num_bytes-1);
    auto burst_addr = aligned_addr + burst_num * num_bytes;
    if (burst_type == AXI_BURST_INCR) {
      return burst_addr;
    }
    if (burst_type == AXI_BURST_WRAP) {
      auto wrap_boundary = (start_addr / (burst_len * num_bytes)) * (burst_len * num_bytes);
      if (burst_addr >= wrap_boundary + (burst_len * num_bytes)) {
	burst_addr = burst_addr - (burst_len * num_bytes);
      }
      return burst_addr;
    }
    return 0;
  }

  uint64_t get_axi_start_byte(uint64_t addr,
			      uint64_t data_bus_bytes) {
    return addr - (addr & ~(data_bus_bytes -1));
  }

  uint64_t get_aligned_addr (uint64_t addr, uint64_t data_bus_bytes) {
    return (addr & ~(data_bus_bytes -1));
  }

  uint64_t get_axi_end_byte(uint64_t addr,
			    uint64_t num_bytes,
			    uint64_t data_bus_bytes) {
    return  get_aligned_addr(addr, num_bytes) + num_bytes - 1 - get_aligned_addr(addr, data_bus_bytes);

  }


  void update_ready_req() {
    for (auto i = requests.begin(); i < requests.end(); ) {
      bool can_pass_req = true;
      for (auto &req : ready_requests) {
	if (!can_pass(req, *i)) {
	  can_pass_req = false;
	  break;
	}
      }
      if (can_pass_req) {
	ready_requests.push_back(*i);
	i = requests.erase(i);
      } else {
	++i;
      }
    }
  }

  std::vector<req_t> get_ready_read() {
    std::vector<req_t> retVal;
    for (auto i = ready_requests.begin(); i != ready_requests.end(); ) {
      if (i->req_type == req_t::READ) {
	retVal.push_back(*i);
	i = ready_requests.erase(i);
      } else {
	++i;
      }
    }
    return retVal;
  }

  std::vector<req_t> get_ready_write() {
    std::vector<req_t> retVal;
    for (auto i = ready_requests.begin(); i != ready_requests.end(); ) {
      if (i->req_type == req_t::WRITE) {
	AW * wr = i->get_write_req();
	auto id  = wr->id().template convert_to<uint64_t>();
	auto len = wr->len().template convert_to<uint64_t>();
	if (write_data_map[id].size() > len) {
	  retVal.push_back(*i);
	  i = ready_requests.erase(i);
	} else {
	  ++i;
	}
      } else {
	++i;
      }
    }
    return retVal;
  }


  virtual bool read_resp (std::vector<pen_csr_base *> &resp_vec) {
    // Check the front of the request
    update_ready_req();
    auto ready_read = get_ready_read();

    if (ready_read.size() == 0) {
      return false;
    }
    uint64_t req_num = chooser.eval() % ready_read.size();
    AR * xn = ready_read[req_num].get_read_req();
    ready_read.erase(ready_read.begin()+req_num);
    // Put back all other ready requests.
    ready_requests.insert(ready_requests.end(),
			  ready_read.begin(),
			  ready_read.end());

    uint64_t xfer_sz = get_xfer_size(xn->len().template convert_to<uint64_t>(),
				     xn->burst().template convert_to<uint64_t>(),
				     xn->sz().template convert_to<uint64_t>());

    auto ptr = std::unique_ptr<uint8_t[]>(new uint8_t[xfer_sz]);
    uint64_t axi_len = xn->len().template convert_to<uint64_t>();
    uint64_t addr_out;
    auto p_mem = get_mem(xn->addr().template convert_to<uint64_t>(), addr_out, true);
    bool assert_p = false;
    bool acc_ok = addr_filter.check_perm(xn->addr().template convert_to<uint64_t>(), xfer_sz, pen_addr_filter::READ, assert_p);
    if (p_mem == nullptr || !acc_ok) {
      if (assert_p) {
	PLOG_ERR("Address 0x"<< hex << xn->addr() << dec << "not permitted to be accessed by " << name << endl);
        PEN_ASSERT(0);
      }
      for (unsigned i = 0; i < axi_len+1; i++) {
	auto resp = new R;
	resp->id(xn->id());
	resp->valid(1);
	resp->resp(3); // DECERR
	resp->last(i == axi_len);
	resp_vec.push_back(resp);
      }
      delete xn;
      return true;
    }
	
    if (!p_mem->can_block_write()) {
      auto acc_sz = p_mem->get_block_size();
      uint32_t i = 0;
      while (i < xfer_sz) {
	auto read_sz = i+acc_sz <= xfer_sz  ? acc_sz : xfer_sz - i;
	p_mem->burst_read(addr_out+i, ptr.get()+i, read_sz);
	i+= read_sz;
      } 
    } else {      
      p_mem->burst_read(addr_out, ptr.get(), xfer_sz);
    }
    PLOG("axi_trace", "AXI_TRACE: " << name << " AR ADDR " << hex << xn->addr() << " LEN " <<  xn->len() << " SZ " << xn->sz() << dec << "\n");
    // Now create responses
    uint64_t addr = xn->addr().template convert_to<uint64_t>();
    uint64_t num_bytes = 1<<xn->sz().template convert_to<uint64_t>();
    uint64_t burst_type = xn->burst().template convert_to<uint64_t>();
    uint64_t offset = 0;
    for (unsigned i = 0; i < axi_len+1; i++) {
      auto resp = new R;
      resp->id(xn->id());
      resp->valid(1);
      resp->last(i == axi_len);
      auto start_byte = get_axi_start_byte(addr, 64);
      auto end_byte   = get_axi_end_byte(addr, num_bytes, 64);
      cpp_int_helper::s_cpp_int_from_array(resp->int_var__data, start_byte, end_byte, ptr.get()+offset);
      offset += (end_byte-start_byte+1);
      addr = get_axi_addr(addr_out,i+1,burst_type,num_bytes,axi_len);
      resp_vec.push_back(resp);
    }
    delete xn;
    return true;
  }

  virtual void write     (pen_csr_base * axi_aw) {
    // Check if the read req can be converted.
    auto aw = dynamic_cast<AW *>(axi_aw);
    assert(aw);
    requests.emplace_back(aw, req_t::WRITE);
    update_ready_req();
    write_ids.emplace_back(aw->id().template convert_to<uint32_t>(),
			   aw->len().template convert_to<uint32_t>());
  }


  virtual void write_data(pen_csr_base * axi_w) {
    auto w = dynamic_cast<W *>(axi_w);
    assert(w);
    auto id = write_ids.front().id;
    auto len = write_ids.front().len;
    write_data_map[id].push_back(w);
    if (len == 0) {
      write_ids.pop_front();
    } else {
      write_ids.front().len = len-1;
    }
  }

  virtual void write_resp(pen_csr_base *& axi_b) {
    // Check the front of the request
    update_ready_req();
    auto ready_write = get_ready_write();
    axi_b = nullptr;
    if (ready_write.size() == 0) {
      return;
    }
    uint64_t req_num = chooser.eval() % ready_write.size();
    AW * xn = ready_write[req_num].get_write_req();
    ready_write.erase(ready_write.begin()+req_num);
    // Put back all other ready requests.
    ready_requests.insert(ready_requests.end(),
			  ready_write.begin(),
			  ready_write.end());

    uint64_t xfer_sz = get_xfer_size(xn->len().template convert_to<uint64_t>(),
				     xn->burst().template convert_to<uint64_t>(),
				     xn->sz().template convert_to<uint64_t>());

    uint64_t axi_len = xn->len().template convert_to<uint64_t>();
    auto & w_queue = write_data_map[xn->id().template convert_to<uint64_t>()];
    auto ptr = std::unique_ptr<uint8_t[]>(new uint8_t[xfer_sz]);
    uint64_t addr_out;
    auto p_mem = get_mem(xn->addr().template convert_to<uint64_t>(), addr_out, false);
    bool assert_p = false;
    bool acc_ok = addr_filter.check_perm(xn->addr().template convert_to<uint64_t>(), xfer_sz, pen_addr_filter::WRITE, assert_p);
    if (p_mem == nullptr || !acc_ok) {
      if (assert_p) {
	PLOG_ERR("Address 0x"<< hex << xn->addr() << dec << "not permitted to be accessed by " << name << endl);
        PEN_ASSERT(0);
      }	
      for (unsigned i = 0; i < axi_len+1; i++) {
	auto w_data = w_queue.front();
	w_queue.pop_front();
	delete w_data;
      }
      auto resp = new B;
      resp->id(xn->id());
      resp->valid(1);
      resp->resp(3); // DECERR
      axi_b = resp;      
      delete xn;
      return;
    }

    auto block_write_enable = p_mem->can_block_write();
    auto block_size = p_mem->get_block_size();
    // Now create responses
    uint64_t addr = addr_out;
    uint64_t num_bytes = 1<<xn->sz().template convert_to<uint64_t>();
    uint64_t start_addr = get_aligned_addr(addr_out,num_bytes);
    uint64_t xfer_start_off = start_addr - get_aligned_addr(addr_out,64);
    auto first_start_byte = addr_out - start_addr;

    uint64_t burst_type = xn->burst().template convert_to<uint64_t>();
    uint64_t offset = 0;
    auto resp = new B;
    resp->id(xn->id());
    resp->valid(1);
    assert (w_queue.size() > axi_len);
    PLOG("axi_trace", "AXI_TRACE: " << name << " AW ADDR " << hex << xn->addr() << " LEN " <<  xn->len() << " SZ " << xn->sz() << dec << "\n");
    for (unsigned i = 0; i < axi_len+1; i++) {
      uint8_t buf[64];
      auto w_data = w_queue.front();
      w_queue.pop_front();
      PLOG("axi_trace", "AXI_TRACE: " << name << " W STRB " << hex << w_data->strb() << dec << "\n");
      if (i == 0 && save_write_xn) { // The AXI UVM monitor seems to only inform us about the first flit.
        auto info_p = new axi_info_t {1, addr, w_data->strb().template convert_to<uint64_t>(),
                                      w_data->data()};
        axi_write_q.push_back(info_p);
      }

      auto start_byte = get_axi_start_byte(addr, 64);
      auto end_byte   = get_axi_end_byte(addr, num_bytes, 64);
      cpp_int mask(1);
      cpp_int strb = w_data->strb();
      mask <<= (end_byte+1);
      mask  -= 1;
      mask >>= start_byte;
      mask <<= start_byte;
      // Block write enabled, do read modify write
      if (block_write_enable && (~strb != 0)) {
          p_mem->burst_read(get_aligned_addr(addr,64), buf, 64);
      }
      cpp_int_helper::s_array_from_cpp_int(w_data->int_var__data, start_byte, end_byte, buf+start_byte, w_data->int_var__strb);
      memcpy(ptr.get()+offset, buf+xfer_start_off, end_byte-xfer_start_off+1);

      // Block write not enabled, do 8/4/2/1 byte writes.
      if ( !block_write_enable ) {
          uint8_t * p = ptr.get()+offset+first_start_byte;
          strb >>= start_byte;
          auto b = start_byte;
	  auto a = get_aligned_addr(addr, 64);
          // No need to add offset here are address is already incremented.
          while (b <= end_byte) {
            if (block_size >= 64 && (strb & 0xffffffffffffffffull) == 0xffffffffffffffffull && ((b & 0x3f) == 0)) {
                  p_mem->burst_write(a+b, p, 64);
                  b += 64;
                  strb >>= 64;
                  p += 64;
            } else if (block_size >= 32 && (strb & 0xffffffff) == 0xffffffff && ((b & 0x1f) == 0)) {
                  p_mem->burst_write(a+b, p, 32);
                  b += 32;
                  strb >>= 32;
                  p += 32;
            } else if (block_size >= 16 && (strb & 0xffff) == 0xffff && ((b & 0xf) == 0)) {
                  p_mem->burst_write(a+b, p, 16);
                  b += 16;
                  strb >>= 16;
                  p += 16;
            } else if (block_size >= 8 && (strb & 0xff) == 0xff && ((b & 0x7) == 0)) {
                  p_mem->burst_write(a+b, p, 8);
                  b += 8;
                  strb >>= 8;
                  p += 8;
              } else if (block_size >= 4 && (strb & 0xf) == 0xf && ((b & 0x3) == 0)) {
                  p_mem->burst_write(a+b, p, 4);
                  b += 4;
                  strb >>= 4;
                  p += 4;
              } else if (block_size >= 2 && (strb & 0x3) == 0x3 && ((b & 0x1) == 0)) {
                  p_mem->burst_write(a+b, p, 2);
                  b += 2;
                  strb >>= 2;
                  p += 2;
              } else if (strb & 0x1) {
                  p_mem->burst_write(a+b, p, 1);
                  b += 1;
                  strb >>= 1;
                  p += 1;
              } else {
                  b += 1;
                  strb >>= 1;
                  p += 1;
              }
          }
      }

      offset += (end_byte-xfer_start_off+1);
      xfer_start_off = 0;
      first_start_byte = 0;
      addr = get_axi_addr(addr_out,i+1,burst_type,num_bytes,axi_len);
      delete w_data;
    }
    // If block writes were not enabled, the data would be written in the
    // innter loop. Write here if enabled.
    if ( block_write_enable ) {
        p_mem->burst_write(start_addr, ptr.get(), xfer_sz);
    }
    axi_b = resp;
    delete xn;
    return;
  }
};

#endif
