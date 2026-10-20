#include <list>

#ifndef MEMORY_PROTECTION_UNIT_HPP
#define MEMORY_PROTECTION_UNIT_HPP

#include <systemc>
#include <systemc.h>
using namespace sc_core;


SC_MODULE(MEMORY_PROTECTION_UNIT){

    sc_in<bool> clk;
    sc_in<uint32_t> addr;
    sc_in<uint32_t> wdata;
    sc_in<bool> r;
    sc_in<bool> w;
    sc_in<bool> wide;
    sc_in<uint8_t> user;
    sc_in<bool> mpu_enable;

    sc_in<uint32_t> mem_rdata;
    sc_in<bool> mem_ready;

    sc_out<bool> ready;
    sc_out<bool> error;
    sc_out<uint32_t> rdata;

    sc_out<uint32_t> mem_addr;
    sc_out<uint32_t> mem_wdata;
    sc_out<bool> mem_r;
    sc_out<bool> mem_w;

    uint32_t rom_size;
    uint32_t block_size;
    bool idling=false;

    

    std::list<uint32_t> array_list[256];


    SC_CTOR(MEMORY_PROTECTION_UNIT);

    MEMORY_PROTECTION_UNIT(sc_module_name name,uint32_t block_size)
    :sc_module(name),block_size(block_size)
    {

        SC_THREAD(behaviour);
        sensitive << clk.pos();
    }


    uint8_t getOwner(uint32_t address){
        uint32_t block_index=address/block_size;
        for(uint32_t i=0;i<256;i++){
            for(uint32_t b: array_list[i]){
                if(block_index==b) {
                    return i;
                }
            }
        }
        return 255;
    }


    bool tryAccess(uint32_t address, uint8_t user, bool w,bool width)  //to see if the user can access this address and if so, then also update the data_structure which stores users and their blocks
    {
        if(user==0) return true;
        if(!width){ 
            uint8_t owner=getOwner(address);
            if(owner==user) return true;
            else if(owner==255){
                if(w){
                    uint32_t block_index=address/block_size;
                    array_list[user].push_back(block_index);
                }
                return true;
            }else if(user==255){
                uint32_t block_index=address/block_size;
                array_list[owner].remove(block_index);
                return true;
            }else{
                return false;
            }
        }else{
            uint8_t owner_of_begin=getOwner(address);
            uint8_t owner_of_end=getOwner(address+3);

            if(owner_of_begin==user && owner_of_end==user) return true;
            else if(user==255){
                if(owner_of_begin!=255){
                    uint32_t begin_index=address/block_size;
                    array_list[owner_of_begin].remove(begin_index);
                }
                if(owner_of_end!=255){
                    uint32_t end_index=(address+3)/block_size;
                    array_list[owner_of_end].remove(end_index);
                }
                return true;
            }else if(owner_of_begin==255 || owner_of_end==255){

                if(w){
                    if(owner_of_begin==255 && owner_of_end==user){
                        uint32_t block_index=address/block_size;
                        array_list[user].push_back(block_index);
                        return true;
                    }else if(owner_of_begin==user && owner_of_end==255){
                        uint32_t block_index=(address+3)/block_size;
                        array_list[user].push_back(block_index);
                        return true;
                    }else if(owner_of_begin==255 && owner_of_end==255){
                        uint32_t block_index=address/block_size;
                        array_list[user].push_back(block_index);
                        block_index=(address+3)/block_size;
                        array_list[user].push_back(block_index);
                        return true;
                    }else{
                        return false;
                    }
                }
                else{
                    if((owner_of_begin==255 && owner_of_end==user) || (owner_of_begin==user && owner_of_end==255) || (owner_of_begin==255 && owner_of_end==255)){
                        return true;
                    }else{
                        return false;
                    }
                }
            }else{
                return false;
            }
        }
    }

    
    void behaviour(){
        while(true){  
        wait();


        ready.write(0); //resetting ready before starting

        if(idling){     //waiting before next request so old values can be written
            wait();
            wait();
        }
        
        
        if(mpu_enable.read()){  

            if(tryAccess(addr.read(),user.read(),w.read(), wide.read())){ // if the user is authorized, then proceed to process the request

                if(r.read() == true && w.read() == false){
                    doRead();
                    idling =true;
                } else if(r.read() == false && w.read() == true){
                    doWrite();
                    idling=true;
                }
            } else {
                ready.write(1);
                error.write(1);
                rdata.write(0);
                idling=true;
            }
        }   
      }
    }

    void doRead(){      
        if(wide) {
            ifWideRead();
        } else {
            notWideRead();
        }
    }

    void doWrite(){    
        if(wide){
            ifWideWrite();   
        } else {
            notWideWrite();          
        }
    }
    
    void ifWideRead(){
        
        mem_addr.write(addr.read());
        mem_r.write(true);
        mem_w.write(false);
                
        while (!mem_ready.read()){
            wait();
        }
        
        rdata.write(mem_rdata.read());

        mem_r.write(false);     //setting the values before next request
        mem_w.write(false);
        ready.write(1);
        error.write(0);
    }

    void notWideRead(){

        mem_addr.write(addr.read() & ~0x3); //aligns the address down to the nearest multiple of 4
        mem_r.write(true);
        mem_w.write(false);


        while (!mem_ready.read()) wait();

        uint32_t memVal = mem_rdata.read();
        mem_r.write(false);

        memVal = mem_rdata.read();          //getting the 1 byte out of 4 byte addr
        uint32_t offset = addr.read() % 4;
        uint8_t byte = (memVal >> (offset * 8)) & 0xFF;

        rdata.write(static_cast<uint32_t> (byte) );

        mem_r.write(false);         //setting the values before next request
        mem_w.write(false);
        ready.write(1);
        error.write(0);
    }

    void ifWideWrite(){
        mem_addr.write(addr.read());
        mem_wdata.write(wdata.read());
        mem_w.write(true);
        mem_r.write(false);
        

        while (!mem_ready.read()) {
            wait();
        }

        mem_w.write(false);

        mem_r.write(false);     //setting the values before next request
        mem_w.write(false);
        ready.write(1);
        error.write(0);
    }

    void notWideWrite(){
        // 1. Step: reading old Word

        mem_addr.write(addr.read() & ~0x3);  // 4-Byte-aligned Adress
        mem_r.write(true);
        mem_w.write(false);

        while (!mem_ready.read()) wait();

        uint32_t oldVal = mem_rdata.read();  // readed 4-Byte-Wert
        mem_r.write(false);

        //2. Step: adding new Byte and  writing the new Word 

        uint32_t address = addr.read();
        uint8_t offset = address % 4;
        uint32_t mask = 0xFF << (offset * 8);        
        uint32_t byteVal = (wdata.read() & 0xFF) << (offset * 8);

        uint32_t newVal = (oldVal & ~mask) | byteVal; 

        //now writing the new Word

        mem_addr.write(address & ~0x3);
        mem_wdata.write(newVal);
        mem_w.write(true);
        wait(); 

        while (!mem_ready.read()) wait();

       
        mem_r.write(false);   //setting values before next  request
        mem_w.write(false);
        ready.write(1);
        error.write(0);
    }

};

#endif //MAIN_PROTECTION_UNIT_HPP