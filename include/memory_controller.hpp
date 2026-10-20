#ifndef MEMORY_CONTROLLER_HPP
#define MEMORY_CONTROLLER_HPP

#include <systemc>
#include "main_memory.hpp"
#include "rom.hpp"
#include "memory_protection_unit.hpp"
#include "multiplexer.hpp"
using namespace sc_core;

extern "C" Result run_simulation(uint32_t cycles ,const char* traceFile,uint32_t latencyRom ,uint32_t romSize ,uint32_t blockSize ,uint32_t* romContent ,uint32_t numRequests ,Request* requests);

SC_MODULE(MemoryController) {

    ROM rom;
    MEMORY_PROTECTION_UNIT mpu;

    //output multiplexers:
    MULTIPLEXER_I32 muxRdata;
    MULTIPLEXER_BOOLEAN muxReady;
    MULTIPLEXER_BOOLEAN muxError;

    sc_signal<uint8_t> flag;
    sc_signal<bool> rom_enable;
    sc_signal<bool> mpu_enable;

    sc_signal<uint32_t> rom_rdata_muxRdata;
    sc_signal<uint32_t> mpu_rdata_muxRdata;
    sc_signal<bool> rom_ready_muxReady;
    sc_signal<bool> mpu_ready_muxReady;
    sc_signal<bool> rom_error_muxError;
    sc_signal<bool> mpu_error_muxError;

    sc_signal<uint32_t> rdata_out;
    sc_signal<bool> ready_out;
    sc_signal<bool> error_out;

    sc_signal<bool> wide_mpu_in;
    sc_signal<uint32_t> addr_mpu_in;
    sc_signal<bool> r_mpu_in;
    sc_signal<bool> w_mpu_in;

    sc_signal<bool> wide_rom_in;
    sc_signal<uint32_t> addr_rom_in;
    sc_signal<bool> r_rom_in;
    sc_signal<bool> w_rom_in;

    sc_signal<uint32_t> wdata_mpu_in;
    sc_signal<uint8_t> user_mpu_in;
    sc_signal<bool> mem_ready_mpu_in;
    sc_signal<uint32_t>mem_rdata_mpu_in;

    sc_signal<uint32_t> mem_addr_mpu_out;
    sc_signal<uint32_t> mem_wdata_mpu_out;
    sc_signal<bool> mem_r_mpu_out;
    sc_signal<bool> mem_w_mpu_out;

    // Ports
    sc_in<bool> clk;  // Clock input
    sc_in<bool> r;  
    sc_in<bool> w;  
    sc_in<bool> wide; 
    sc_in<uint32_t> addr;  
    sc_in<uint32_t> wdata;  
    sc_in<uint8_t> user; 
    sc_in<bool> mem_ready;
    sc_in<uint32_t> mem_rdata;

    sc_out<uint32_t> rdata;
    sc_out<bool> ready;
    sc_out<bool> error;

    sc_out<uint32_t> mem_addr;
    sc_out<uint32_t> mem_wdata;
    sc_out<bool> mem_r;
    sc_out<bool> mem_w;

    uint32_t latency_rom;
    uint32_t rom_size;
    uint32_t block_size;

    
    // Constructor
    SC_CTOR(MemoryController);

    MemoryController(sc_module_name name, uint32_t latencyRom, uint32_t romSize, uint32_t blockSize, uint32_t* romContent)
        : sc_module(name), latency_rom(latencyRom), rom_size(romSize), block_size(blockSize),
        rom("rom",romSize,latencyRom,romContent), mpu("mpu",blockSize),muxRdata("mux_rdata",2,1), muxReady("mux_ready",2,1), muxError("mux_error",2,1)
         
    {   
        

        //binding the clocks
        rom.clk(clk);
        mpu.clk(clk);
        
        //binding the enable signals
        //we used enable signals to prevent the system from working in the wrong place
        rom.rom_enable.bind(rom_enable);
        mpu.mpu_enable.bind(mpu_enable);

        //binding the common inputs for rom
        rom.addr.bind(addr_rom_in);

        rom.wide.bind(wide_rom_in);

        rom.w.bind(w_rom_in);

        rom.r.bind(r_rom_in);

        //binding the common inputs for mpu

        mpu.addr.bind(addr_mpu_in);

        mpu.wide.bind(wide_mpu_in);

        mpu.w.bind(w_mpu_in);

        mpu.r.bind(r_mpu_in);

        //binding inputs to mpu as inputs
        mpu.wdata.bind(wdata_mpu_in);

        mpu.mem_ready.bind(mem_ready_mpu_in);
        
        mpu.mem_rdata.bind(mem_rdata_mpu_in);
    
        mpu.user.bind(user_mpu_in);
        
        //binding outputs of mpu to main memory
        mpu.mem_addr.bind(mem_addr_mpu_out);
        
        mpu.mem_wdata.bind(mem_wdata_mpu_out);
        
        mpu.mem_r.bind(mem_r_mpu_out);
        
        mpu.mem_w.bind(mem_w_mpu_out);
        
        //binding the output values to output-multiplexers
        //multiplexers are used to get the corresponding values
      
        rom.rdata.bind(rom_rdata_muxRdata);
        muxRdata.in[0](rom_rdata_muxRdata);

        rom.ready.bind(rom_ready_muxReady);
        muxReady.in[0](rom_ready_muxReady);

        rom.error.bind(rom_error_muxError);
        muxError.in[0](rom_error_muxError);

        mpu.rdata.bind(mpu_rdata_muxRdata);
        muxRdata.in[1](mpu_rdata_muxRdata);

        mpu.ready.bind(mpu_ready_muxReady);
        muxReady.in[1](mpu_ready_muxReady);

        mpu.error.bind(mpu_error_muxError);
        muxError.in[1](mpu_error_muxError);

        //binding the outputs to output_multiplexers
        muxRdata.out[0](rdata_out);

        muxReady.out[0](ready_out);

        muxError.out[0](error_out);

        //binding the select signals to flag
        muxRdata.select.bind(flag);
        muxReady.select.bind(flag);
        muxError.select.bind(flag);
        
        SC_THREAD(behaviour);
        sensitive << clk.pos();
    }


    // in our behaviour method we "manually" write the incoming signals to the corresponding input-ports of the submodules
    // our memory_controller module also sets the enable-port of the submodules--> enable-port of Rom is set to true iff adress<romSize and enable-port of MPU is set to true iff adress>=romSize
    // our memory_controller also has an internal flag-signal, which is set accordingly to select the outputs from the submodules via a multiplexer
    void behaviour() {
        while (true) {
            wait(); 

            //writing to the inputs
            addr_mpu_in.write(addr.read());
            addr_rom_in.write(addr.read());
            wide_mpu_in.write(wide.read());
            wide_rom_in.write(wide.read());
            w_mpu_in.write(w.read());
            w_rom_in.write(w.read());
            r_mpu_in.write(r.read());
            r_rom_in.write(r.read());

            wdata_mpu_in.write(wdata.read());
            user_mpu_in.write(user.read());
            mem_ready_mpu_in.write(mem_ready.read());
            mem_rdata_mpu_in.write(mem_rdata.read());
    
           if(w.read()|| r.read()){
                ready.write(false);
                error.write(false);
            }
            
            //writing to mpu outputs
            mem_addr.write(mem_addr_mpu_out.read());
            mem_wdata.write(mem_wdata_mpu_out.read());
            mem_r.write(mem_r_mpu_out.read());
            mem_w.write(mem_w_mpu_out);

            //writing to outputs
            rdata.write(rdata_out.read());
            ready.write(ready_out.read());
            error.write(error_out.read());

            uint32_t address = addr.read();
            if (address<rom_size){       //checking which submodul is going to be activated --> then setting the enable signals and flags
                flag.write(0);          
                mpu_enable.write(false);
                rom_enable.write(true);
            } else {
                flag.write(1);
                rom_enable.write(false);
                mpu_enable.write(true);
            }

        }
    }


    uint8_t getOwner(uint32_t address){
        if(address>=rom_size){
            return mpu.getOwner(address);
        }
    }

     void setRomAt(uint32_t address, uint8_t data) {
        rom.setRomAt(address,data);
    }


};    

#endif // MEMORY_CONTROLLER_HPP    