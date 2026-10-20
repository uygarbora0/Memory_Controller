#ifndef ROM_HPP
#define ROM_HPP

#include <systemc>
#include <vector>
using namespace sc_core;

extern "C" int run_simulation_rom( uint32_t romSize, uint32_t romLatency, uint32_t* romContent, Request* requests, uint32_t numRequests,uint32_t cycles);

SC_MODULE(ROM) {
    // Ports
    sc_in<bool> clk;  
    sc_in<bool> r;    
    sc_in<bool> w;    
    sc_in<uint32_t> addr;  
    sc_in<bool> wide;   
    sc_in<bool> rom_enable; //This port is set by the memory_controller , indicating that the module can work
    
    sc_out<bool> error;     
    sc_out<bool> ready;     
    sc_out<uint32_t> rdata;   

    std::vector<uint8_t> memory; //rom_memory, where data is stored
    uint32_t rom_size;
    uint32_t latency_rom;
    bool working=false; 
    bool idling=false;

    // Constructor
    SC_CTOR(ROM);

    ROM(sc_module_name name, uint32_t romSize, uint32_t romLatency, uint32_t* romContent)
        : sc_module(name), rom_size(romSize), latency_rom(romLatency)
    {

        

        memory.resize(rom_size);  // rom_size ist in Bytes
        uint32_t wordCount = rom_size / 4; // number of 4-byte words

        for (size_t i = 0; i < wordCount; ++i) {
            size_t base_address = i * 4; 
            uint32_t value = romContent[i]; // extract a 4-byte value   

            for (int j = 0; j < 4; ++j) {
                memory[base_address + j] = (value >> (j * 8)) & 0xFF; // 4 byte data is divided in 1 byte values and written into the corresponding adress (Little Endian)
            }
        }

        SC_THREAD(behaviour);
        sensitive << clk.pos();
    }

    void behaviour() {
        
        while (true) {
            wait();

             // In the beginning set ready and error to false
            ready.write(false);    
            error.write(false);   
            
            // an idling phase, so that rom doesn't process the "old" data before rom_enable is set to false or  new data is assigned by memory_controller 
            if(idling){             
                wait();
                wait();
                idling=false;
            }
            
            if(rom_enable.read() && (r.read() || w.read()) ) {  // if rom_enable is set true and 
                uint32_t address = addr.read();
                 

                if (address < rom_size) {   //checking if the address in rom or not
                    if (w.read()) {
                        doWrite();
                    }
                    else if (r.read()) {
                        doRead();
                    }
                }else{
                    // Eventhough this branch shouldnt be taken at all (because memory_controller shouldn't assign such an adress to rom), precaution is good
                    error.write(1);
                    ready.write(1);
                    rdata.write(0);
                }
            }    
        }    
    }


    void doWrite() {
        rdata.write(0); // set rdata to 0 so that the unauthorized user doesnt get any information about the last formevalid request to main_memory
        error.write(true);  
        ready.write(true);
        idling=true; // idling phase begins
    }

    void doRead() {

        uint32_t address = addr.read();
        for (uint32_t i = 0; i < latency_rom; ++i) {
            wait();
        }

        if (wide.read()) {  // 4-Byte reads
            uint32_t value = 0;

            if (address + 3 < rom_size) {
                for (int i = 0; i < 4; ++i) {
                    value |= (memory[address + i] << (i * 8));
                }
               
                rdata.write(value);
            }else{
                error.write(true);  
            }
            
        }else {  // 1-Byte read
            uint8_t value = 0; 
            value = memory[address];
            rdata.write(value);
        }
            
        ready.write(true);
        idling=true;        // idling phase begins
    }

    void setRomAt(uint32_t address, uint8_t data) {
        if (address < rom_size) {
            memory[address] = data;
        }
    }
};

#endif // ROM.hpp