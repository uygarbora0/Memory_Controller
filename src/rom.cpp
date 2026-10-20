#include <systemc>
#include "systemc.h"
#include "structs.h"
#include "rom.hpp"

using namespace sc_core;

//was used for rom-testing, isnt important for the project!

int run_simulation_rom( uint32_t romSize, uint32_t romLatency, uint32_t* romContent, Request* requests, uint32_t numRequests,uint32_t cycles){
        
    ROM rom("rom",romSize,romLatency,romContent);
    
    //for input ports
    sc_signal<uint32_t> address;
    sc_signal<bool> wide,r,w;
    
    //for output ports
    sc_signal<uint32_t> rdata;
    sc_signal<bool> ready,error,rom_enable;

    sc_time clock_period(10, SC_NS);  // z. B. 100 MHz
    sc_clock main_clk("main_clk", clock_period);
    
    rom.addr(address);
    rom.wide(wide);
    rom.r(r);
    rom.w(w);
    rom.clk(main_clk);
    rom.rom_enable(rom_enable);

    rom.rdata(rdata);
    rom.ready(ready);
    rom.error(error);
   
    uint32_t current_request_number=0;
    uint32_t number_of_cycles=0;
    uint32_t number_of_errors=0;
    for(uint32_t i=0; i<cycles&&current_request_number<numRequests;i++){
        Request current_request=requests[current_request_number];

        if(i==0){
            r.write(current_request.w==0);
            w.write(current_request.w==1);
            wide.write(current_request.wide==4);
            address.write(current_request.addr);
            rom_enable.write(true);

            //printf("First request is being executed: \n");

        }
        if(ready.read()==true){
           if(rom.error.read()) number_of_errors++;
           printf("-------> \n",current_request_number);
           printf("Request %d has been passed processed \n",current_request_number);
           printf("rData of %d request is:%u \n",current_request_number,rdata.read());
           printf("Error of %d request is:%u \n",current_request_number,error.read());
           printf("\n");
           current_request_number++;
            

           if(current_request_number<numRequests){
             current_request=requests[current_request_number];
             r.write(current_request.w==0);
             w.write(current_request.w==1);
             wide.write(current_request.wide==4);
             address.write(current_request.addr);
             rom_enable.write(true);
              printf("---->Next request (%d) has been loaded \n",current_request_number);
              printf("Adress: %u\n",current_request.addr);
              printf("Write: %d\n",current_request.w);
              printf("Wide: %d bytes\n",current_request.wide);
           }
           

           
        }

        //increase the clock
        printf("Cycle :%d\n",number_of_cycles);
        number_of_cycles++;
        sc_start(clock_period);
        
    }

    return 0;
}

