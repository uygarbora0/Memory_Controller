#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

// parses a single line into a rom-content, in case of an invalid -1 is returned
// same as convert_string_to_uint32, but implemented here to provide better debug information for the user
int parse_single_line_rom_content(char* number_ptr, uint32_t* res_ptr) {
        if (number_ptr == NULL || *number_ptr == '\0') {
            fprintf(stderr, "Invalid ROM-line: '%s' tried to be parsed to an unsigned int\n", number_ptr);
            return -1;  
        }

        if(number_ptr[0]=='-') {
            if(number_ptr[1]=='\0'){
                fprintf(stderr,"The  argument %s is  not a number!\n",number_ptr);
                return -1;
            }
            if (number_ptr[1] != '\0' ) {
                fprintf(stderr,"The  argument %s is  a negative number, which it shouldnt be !\n",number_ptr);
                return -1;
            }    
        }
        char* endptr=NULL;
        errno=0;

        // explicitly chosen uint64_t to be able to check overflows
        uint64_t res;
        if (number_ptr[0] == '0' && number_ptr[1] == 'x') {
            res = strtoul(number_ptr, &endptr, 16);
        }else{
            res = strtoul(number_ptr, &endptr, 10);
        }

        if(*endptr!='\0'){
            fprintf(stderr,"The  argument %s is not a number!\n",number_ptr);
            return -1;
        }

        
        if(res>UINT32_MAX || errno==ERANGE){ //an owerflow/underflow has happened
            fprintf(stderr,"The  argument : %s was larger than 4 bytes which resulted in an overflow!\n",number_ptr);
            return -1;
        }
    

        *res_ptr=(uint32_t)res;
        return 0;
    }


    // parses every single line of the provided string and returns the number of lines
    // an empty file (this method returns 0) won't be considered an error
    // stores every single parsed rom-line in array which is passed as an argument
    int parse_rom_content(const char*input,uint32_t* array)
    {
        uint32_t count = 0;
        const char* last_new_line = input;


        while(1){

            //trim the leading whitespace
            while(*last_new_line==' '|| *last_new_line=='\n'|| *last_new_line=='\t'){
                last_new_line++;
            }

            //end of the file-content is reached
            if(*last_new_line=='\0'){
                break;
            }

            //next_new_line points to the "next" line (to be more specific "\n" before the next line)
            char* next_new_line=strchr(last_new_line,'\n');
            if(next_new_line==NULL){
                // currently at the last line
                next_new_line=strchr(last_new_line,'\0');
            }

            char* number_ptr=(char*)malloc(next_new_line-last_new_line+1);
            if(number_ptr==NULL){
                fprintf(stderr,"Failed to allocate memory\n");
                return -1;
            }

            // store the begin of the allocated memory, so that it can be freed later
            char* start_of_number_ptr=number_ptr;

            while(*last_new_line!=' ' && *last_new_line!='\n' && *last_new_line!='\t' && *last_new_line!='\0'){
                *number_ptr=*last_new_line;
                last_new_line++;
                number_ptr++;
            }
            // number_ptr now contains the "number" of the current line
            *number_ptr='\0';


            uint32_t current_number=0;
            int valid_number=parse_single_line_rom_content(start_of_number_ptr,&current_number);

           

            if(valid_number!=0){
                fprintf(stderr,"The current line being converted to a number was not a valid number\n");
                free(start_of_number_ptr);
                return -1;
            }

            //trim whitespace off the end
            while (*last_new_line == ' ' || *last_new_line == '\t') {
                last_new_line++;
            }

            if(array!=NULL){
                array[count]=current_number;
            }
            count++;
            free(start_of_number_ptr);

            while (last_new_line < next_new_line) {
                
                if (*last_new_line != '\n' && *last_new_line != '\0') { // there is a character after the "number"
                    return -1;
                }
                last_new_line++;
            }
            
            // end of the file-content
            if (*next_new_line == '\0') {
                break;
            }

            // last_new_line points to the next line and not to the '\n' before it
            last_new_line=next_new_line+1;
            
        }
        
        return count;
    }