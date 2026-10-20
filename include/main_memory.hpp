#ifndef MAIN_MEMORY_HPP
#define MAIN_MEMORY_HPP

#include <systemc>
#include <map>
using namespace sc_core;

#define LATENCY 100

SC_MODULE(MAIN_MEMORY) {
  sc_in<bool> clk;

  sc_in<uint32_t> addr;
  sc_in<uint32_t> wdata;
  sc_in<bool> r;
  sc_in<bool> w;

  sc_out<uint32_t> rdata;
  sc_out<bool> ready;

  std::map<uint32_t, uint8_t> memory;
  bool idling=false;

  SC_CTOR(MAIN_MEMORY) {
    SC_THREAD(behaviour);
    sensitive << clk.pos();
  }


  // additional to the default implemention which was provided by the university, our main_memory moduel intoduces an idling phase, in which the module doesnt process any request!
  // after any request has been processed, the idling flag is set to true
  // if idling-flag is set to true, main-memory doesnt process any signal/request for 3 cycles
  void behaviour() {
    while(true) {
      wait();
      ready.write(false);
      if(idling){
        wait();
        wait();
        wait();
        idling=false;
      }

      if (r.read()) {
        doRead(w.read());
        idling=true;
      }
      if (w.read()) {
        doWrite();
        idling=true;
      } 
    }
  }

  void doRead(bool dontSetReady) {
    ready.write(false);
    
    for(int i = 0; i < LATENCY; i++) { 
      wait();
    }
    uint32_t result = get(addr.read());
    
    rdata.write(result);
    if(!dontSetReady) {
      ready.write(true);
    }
  }

  void doWrite() {
    
    ready.write(false);
    for(int i = 0; i < LATENCY; i++) {
      wait();
    }
    set(addr.read(), wdata.read());
    ready.write(true);
  }

  uint32_t get(uint32_t address) {
    uint32_t result = 0;

    for (int i = 0; i < 4; i++) {
      uint8_t value = 0;
      if(memory.find(address + i) != memory.end()) {
        value = memory[address + i];
      }
      result |= value << (i * 8);
    }
    return result;
  }

  void set(uint32_t address, uint32_t value) {
    for (int i = 0; i < 4; i++) {
      memory[address + i] = (value >> (i * 8)) & 0xFF;
      if(address + i == UINT32_MAX) {
        break;
      }
    }
  }


 void load_initial_memory(const std::map<uint32_t, uint32_t>& initMem) {
    memory.clear();  // Optional, to reset memory state

    for (const auto& [addr, word] : initMem) {
        for (int i = 0; i < 4; ++i) {
            memory[addr + i] = (word >> (i * 8)) & 0xFF;
        }
    }
}

};



#endif // MAIN_MEMORY_HPP
