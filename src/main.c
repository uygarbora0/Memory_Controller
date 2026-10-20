#include <stdint.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <stdbool.h>
#include "csv_parser.h"
#include "rom_parser.h"
#include "parser_utilities.h"
#include "structs.h"
#include <stdint.h>


extern Result run_simulation(uint32_t cycles ,const char* traceFile,uint32_t latencyRom ,uint32_t romSize ,uint32_t blockSize ,uint32_t* romContent ,uint32_t numRequests ,Request* requests);


// VALIDATION of access-rights for the files
// all methods in this section return 1 if path can not be written to and otherwise return 0
FILE* validate_and_open_read(char* input_path){
    FILE* file;
    if (!(file = fopen(input_path, "r"))) {
        return NULL;
    }
    struct stat statbuf;
    if (fstat(fileno(file), &statbuf)) {
        fclose(file);
        return NULL;
    }
    if (!S_ISREG(statbuf.st_mode) || statbuf.st_size <= 0) {
        fclose(file);
        return NULL;
    }
    return file;
}

int validate_and_open_write(char* trace_file){
    FILE* file;
    if (!(file = fopen(trace_file, "w"))) {
        return 1;
    }
    fclose(file);
    return 0;
}

//VALIDATION OF OPTIONAL ARGUMENTS
// return 0 if arguments are valid and else 1


// the provided argument should be a number , non-negative and not greater than 2^32-1
int validate_latency_rom(char* latency_rom,uint32_t* ptr){
    int valid_num=convert_string_to_uint_32(latency_rom,ptr);
    if(valid_num==1) return 1;
    return 0;
}

int validate_rom_size(char* rom_size,uint32_t* ptr){
    int valid_num=convert_string_to_uint_32(rom_size,ptr);
    if(valid_num==1){
        return 1;
    } 

    uint32_t romSize=*ptr;
    // power of 2
    if(romSize!=0 && (romSize&(romSize-1))!=0){
        fprintf(stderr,"Rom size should be a power of 2!\n");
        return 1;
    }
    return 0;

}

// Block-size should be at least 4, non-negative and not-greter than 2^32-1
int validate_block_size(char* block_size,uint32_t* ptr){
    int valid_num=convert_string_to_uint_32(block_size,ptr);
    if(valid_num==1) return 1;

    // if it isnt power of 2
    if (*ptr <4  || ((*ptr & (*ptr - 1)) != 0)) {
        fprintf(stderr, " Block-size must be at least 4 and a number of power of 2\n");
        return 1;
    }


    return 0;

}

// Cycle-number should be non-negative and not greater than 2^32-1
int validate_cycles(char* cycles,uint32_t* ptr){
    int valid_num=convert_string_to_uint_32(cycles,ptr);
    if(valid_num==1) return 1;
    return 0;
}




void print_help() {
    printf("Simulates a memory controller with configurable ROM and optional trace file output.\n\n");

    printf("Positional argument:\n");
    printf("  <input_file>           Path to the input file used for the simulation, which contains requests in csv-format.\n\n");

    printf("General options:\n");
    printf("  --help                 Show this help message and exit.\n");
    printf("  --cycles <number>      Number of clock cycles to simulate.\n");
    printf("                         Default: 5000\n");
    printf("  --tf <path>            Path to the trace file to be generated.\n");
    printf("                         If not provided, no trace file will be created.\n\n");

    printf("ROM and memory configuration:\n");
    printf("  --latency-rom <number>   ROM latency in clock cycles.\n");
    printf("                           Default: 2\n");
    printf("  --rom-size <number>      Size of the ROM in bytes (must be a power of two ≥ 0).\n");
    printf("                           Default: 0\n");
    printf("  --block-size <number>    Size of one memory block in bytes.\n");
    printf("                           Default: 16\n");
    printf("  --rom-content <path>     Path to a file containing ROM contents.\n");
    printf("                           The file must contain 32-bit values (decimal or hexadecimal, e.g., 0x1A2B3C4D),\n");
    printf("                           one per line.\n\n");

    printf("Notes:\n");
    printf("  - If the ROM file contains fewer values than specified by --rom-size, remaining values are set to 0.\n");
    printf("  - If the ROM file contains more values than --rom-size allows, the program will terminate with an error.\n\n");
    printf("  - If no ROM file and no rom-size is specified, then rom_memory will be initialized empty and 0 bytes.\n\n");
    printf("  - If you want to write 1 byte of data to main-memory, then the data in data-column of that particular request shouldn't be greater than 1 byte.\n\n");

    printf("Example:\n");
    printf("  ./project --cycles 200 --rom-size 2048 --rom-content rom.txt --tf trace.log input.dat\n");
}




int parse_cli(int argc, char** argv,char**input_file,char** trace_file,char** cycles,char**latency_rom,char**rom_size,char**block_size,char**rom_content,bool* rom_size_was_set){
    static struct option long_options[] = {
        {"cycles", required_argument, 0, 'c'},
        {"tf", required_argument, 0, 't'},
        {"latency-rom",required_argument,0,'l'},
        {"rom-size",required_argument,0,'r'},
        {"block-size",required_argument,0,'b'},
        {"rom-content",required_argument,0,'p'},
        {"help",no_argument,0,'h'},
        {0, 0, 0, 0}
    };

    int opt;

    while((opt=getopt_long(argc,argv,"h",long_options,NULL))!=-1){
        switch(opt){
            case 'h':
            print_help();
            return 0;
            break;
            case 't':
            *trace_file=optarg;
            break;
            case 'c':
            *cycles=optarg;
            break;
            case 'l':
            *latency_rom=optarg;
            break;
            case 'r':
            *rom_size=optarg;
            *rom_size_was_set=true;
            break;
            case 'b':
            *block_size=optarg;
            break;
            case 'p':
            *rom_content=optarg;
            break;
            case '?':
            fprintf(stderr, "Unknown option: -%c\n", optopt);
            return 1;
            default:
            fprintf(stderr, "Unexpected error while parsing options.\n");
            return 1;
            //print error message

        }
    }

    if(optind>=argc){
        fprintf(stderr, "No input file specified.\n");
        return 1;
    }

    while(optind<argc){
         if (*input_file != NULL) {
            fprintf(stderr, "Too many input files specified.\n");
            return 1;
        }
        *input_file = argv[optind];
        optind++;
    }

    return 0;
    // otherwise there is an positional argument which has been passed, which COULD be the input-file
}









int main(int argc, char** argv){


    char* input_file=NULL;
    char* trace_file=NULL;
    char* rom_content=NULL;
    char* cycles="5000";
    char* latency_rom="2";
    char* rom_size="0";
    char* block_size="16";
    bool rom_size_was_set=false; // to distinguish if possibly rom_size was set to 0 by the user or not
   
    uint32_t numOfCycles;
    uint32_t latencyRom;
    uint32_t romSize;
    uint32_t blockSize;

    


    int result=parse_cli(argc,argv,&input_file,&trace_file,&cycles,&latency_rom,&rom_size,&block_size,&rom_content,&rom_size_was_set);

    if(result!=0 || input_file==NULL){
        // something went wrong while reading arguments from the user
        return EXIT_FAILURE;
    }
     
    // the user has passed the optional arguments-positional arguments successfuly
   

     
     // Validation of numbers
    if(validate_cycles(cycles,&numOfCycles)==1){
        fprintf(stderr, "Number of cycles '%s' is invalid.\n", cycles);
        return EXIT_FAILURE;
    }

    if(validate_latency_rom(latency_rom,&latencyRom)==1){
        fprintf(stderr, "Rom latency '%s' is invalid.\n", latency_rom);
        return EXIT_FAILURE;
    }

    if(validate_block_size(block_size,&blockSize)==1){
        fprintf(stderr, "Block-size '%s' is invalid.\n", block_size);
        return EXIT_FAILURE;
    }


    if(validate_rom_size(rom_size,&romSize)==1){
        fprintf(stderr, "Rom size '%s' is invalid.\n", rom_size);
        return EXIT_FAILURE;
    }

   


    //Validation of files
    if(trace_file!=NULL){
        if(validate_and_open_write(trace_file)==1){
            fprintf(stderr, "Trace file '%s' was not accessible.\n", trace_file);
            return EXIT_FAILURE;
        }

    }

    FILE* file=validate_and_open_read(input_file);
    if(file==NULL){
         fprintf(stderr, "Failed to open input file '%s'.\n", input_file);
         return EXIT_FAILURE;
    }

    FILE* romContent=NULL;
    if(rom_content!=NULL){
        romContent=validate_and_open_read(rom_content);
        if(romContent==NULL){
         fprintf(stderr, "Failed to open rom-content file '%s'.\n", rom_content);
         return EXIT_FAILURE;
        }
    }


    //All arguments are valid
    // Parsing of the input - file
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);


    char* input = (char*)malloc(file_size + 1);

    if (input == NULL) {
        fprintf(stderr, "Failed to allocate memory for input file.\n");
        fclose(file);
        return EXIT_FAILURE;
    }

    if (fread(input, 1, file_size, file) != (size_t)file_size) {
        fprintf(stderr, "Error reading file.\n");
        free(input);
        fclose(file);
        return EXIT_FAILURE;
    }
    fclose(file);
    input[file_size] = '\0'; // Null-terminate the string

    // to determine how much memory we need to store all requests in case all requests are valid
    int number_of_requests=assemble_with_header(input,NULL);
    if(number_of_requests==0){
        fprintf(stderr, "Failed to convert the input of the file to a list of requests.\n");
        free(input);
        return EXIT_FAILURE;
    }

    Request* output = (Request*)malloc(number_of_requests * sizeof(Request));
    if(output==NULL){
            fprintf(stderr,"Failed to allocate memory for rom_content_parsing");
            free(input);
            return EXIT_FAILURE;
        }
    number_of_requests=assemble_with_header(input,output);

    
    free(input);

   

    uint32_t* array=NULL;
    int number_of_entries=0;

    // rom-content was specified explicitly by the user
    if(romContent!=NULL){
        fseek(romContent,0,SEEK_END);
        long file_size=ftell(romContent);
        fseek(romContent,0,SEEK_SET);


        char* input= (char*) malloc(file_size+1);
        if(input==NULL){
            fprintf(stderr,"Failed to allocate memory for rom_content_parsing");
            free(output);
            return EXIT_FAILURE;
        }

        if(fread(input, 1, file_size, romContent) != (size_t) file_size) {
            fprintf(stderr, "Error reading file. \n");
            free(input);
            free(output);
            fclose(romContent);
            return EXIT_FAILURE;
        }
        fclose(romContent);
        input[file_size]='\0';


        number_of_entries=parse_rom_content(input,NULL);
        

        
        if(number_of_entries==-1){
            fprintf(stderr,"An invalid entry in your file: %s was tried to be converted to rom-content",rom_content);
            free(input);
            free(output);
            return EXIT_FAILURE;
        }

        // romSize was explicitly set by the user and romContent is larger
        if(rom_size_was_set==true && romSize < (uint32_t) (number_of_entries*4)){
            fprintf(stderr,"The content (list of integers) of the rom-file: %s is larger than the specified rom-size!! ",rom_content);
            free(input);
            free(output);
            return EXIT_FAILURE;
        }else if(rom_size_was_set==false){ // romSize was not set explicitly, so the default value should be defined correctly 
            romSize=4*number_of_entries;
            // romSize!=0 , so that the if-statement doesnt yield true if rom-file is empty
            //(romSize&(romSize-1))!=0 to check that the "implicit" romSize is of power 2
            if(romSize!=0 && (romSize&(romSize-1))!=0){ 
                fprintf(stderr,"Rom-size should be of power 2 , because you didnt pass any rom-size explicitly , romSize is implicitly extracted from your rom-file, which at the end wasn't of power 2!!\n");
                free(input);
                free(output);
                return 1;
            }

        }else{
            // romSize>=numberOfEntries*4
            // no check if romSize is of power 2 because this check is already performed
        }



        array=(uint32_t*) malloc(romSize);
        if(array==NULL){
            fprintf(stderr,"Failed to allocate memory for rom_content_parsing");
            free(input);
            free(output);
            return EXIT_FAILURE;
        }

        parse_rom_content(input,array);

        // initialize the remaining slots with 0!!
        if(romSize >= (uint32_t) number_of_entries*4){
            
            for(uint32_t i=number_of_entries; i<romSize/4;i++){
               
                array[i]=0;
            }
        }
        free(input);
       
    }else{ // rom-content wasnt specified explicitly but might have been implicitly specified through romSize
        
        if(romSize>0){
            array=(uint32_t*) malloc(romSize);
            if (array==NULL) {
                fprintf(stderr,"Failed to allocate memory for rom_content_parsing");
                free(output);
                return EXIT_FAILURE;
            }  
            for(uint32_t i=0;i<romSize/4;i++){
                array[i]=0;
            }

        }

    }

    Result final_result=run_simulation(numOfCycles,trace_file,latencyRom,romSize,blockSize,array,number_of_requests,output);
    printf("Error flags:%d\n ",final_result.errors);
    printf("Cycles :%d\n ",final_result.cycles);

    free(output);
    if(array!=NULL){
        free(array);
    }
   

    return EXIT_SUCCESS;
}
