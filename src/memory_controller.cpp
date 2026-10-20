#include "structs.h"
#include <systemc>
#include "systemc.h"
#include "memory_controller.hpp"
#include "main_memory.hpp"
#include <stdint.h>

using namespace sc_core;

Result run_simulation(uint32_t cycles ,const char* traceFile,uint32_t latencyRom ,uint32_t romSize ,uint32_t blockSize ,uint32_t* romContent ,uint32_t numRequests ,Request* requests){

   
    MemoryController memory_controller("mc", latencyRom, romSize, blockSize, romContent); // memory_controller instanzieren
    // main_memory
    MAIN_MEMORY main_memory("main_memory");

    sc_signal<bool> r, w, ready, error;
    sc_signal<bool> wide;
    sc_signal<uint32_t> addr, wdata, rdata;
    sc_signal<uint8_t> user;

// Intermediate signals between MemoryController and MAIN_MEMORY
    sc_signal<bool> mem_ready, mem_r, mem_w;
    sc_signal<uint32_t> mem_addr, mem_wdata, mem_rdata;

// Create clock
    sc_time clock_period(10, SC_NS);  // 100 MHz
    sc_clock main_clk("main_clk", clock_period);

// === MemoryController bindings ===
    memory_controller.clk(main_clk);
    memory_controller.addr(addr);
    memory_controller.user(user);
    memory_controller.wide(wide);
    memory_controller.w(w);
    memory_controller.r(r);
    memory_controller.wdata(wdata);

    memory_controller.ready(ready);
    memory_controller.error(error);
    memory_controller.rdata(rdata);

// Memory-side ports of MemoryController — use signals!
    memory_controller.mem_ready(mem_ready);
    memory_controller.mem_rdata(mem_rdata);
    memory_controller.mem_addr(mem_addr);       
    memory_controller.mem_wdata(mem_wdata);     
    memory_controller.mem_r(mem_r);             
    memory_controller.mem_w(mem_w);             

// === MAIN_MEMORY bindings ===
    main_memory.clk(main_clk);
    main_memory.addr(mem_addr);                 
    main_memory.wdata(mem_wdata);              
    main_memory.r(mem_r);                       
    main_memory.w(mem_w);                       
    main_memory.ready(mem_ready);               
    main_memory.rdata(mem_rdata);               


//  ===TRACE-FILE==== if needed
    sc_trace_file* file = nullptr;

    if(traceFile != nullptr){
        file = sc_create_vcd_trace_file(traceFile);

        sc_trace(file, main_clk, "clk");
        sc_trace(file, addr, "addr");
        sc_trace(file, wdata, "wdata");
        sc_trace(file, r, "r");
        sc_trace(file, w, "w");
        sc_trace(file, wide, "wide");
        sc_trace(file, user, "user");
        sc_trace(file, rdata, "rdata");
        sc_trace(file, ready, "ready");
        sc_trace(file, error, "error");

        sc_trace(file, mem_addr, "mem_addr");
        sc_trace(file, mem_wdata, "mem_wdata");
        sc_trace(file, mem_r, "mem_r");
        sc_trace(file, mem_w, "mem_w");
        sc_trace(file, mem_ready, "mem_ready");
        sc_trace(file, mem_rdata, "mem_rdata");
    }
    
    uint32_t current_request_number=0;
    uint32_t number_of_cycles=0;
    uint32_t number_of_errors=0;
    for(uint32_t i=0; i<cycles&&current_request_number<numRequests;i++){
        Request current_request=requests[current_request_number];



        if(i==0){
            r.write(current_request.w==0);
            w.write(current_request.w==1);
            wide.write(current_request.wide==4);
            addr.write(current_request.addr);
            wdata.write(current_request.data);
            user.write(current_request.user);
 
            
        }
        if(ready.read()==true){
           if(memory_controller.error.read()) number_of_errors++;
           current_request_number++;


           if(current_request_number>=numRequests) break;

            current_request=requests[current_request_number];
            r.write(current_request.w==0);
            w.write(current_request.w==1);
            wide.write(current_request.wide==4);
            addr.write(current_request.addr);
            wdata.write(current_request.data);
            user.write(current_request.user);

        }

        //increase the clock
        number_of_cycles++;
        sc_start(clock_period);
    }

    if(file){
        sc_close_vcd_trace_file(file);
    }

    return Result{number_of_cycles,number_of_errors};

}


int sc_main(int argc, char* argv[]) {
    // Unused parameters
    (void)argc;
    (void)argv;
    
    
    return 0;
}

