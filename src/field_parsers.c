#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "parser_utilities.h"
#include <stdint.h>


//Type should be either W or R, anything except "W" or "R" is invalid
    int parseType(char*type, uint8_t* w )
    {
        if(strcmp(type,"W")==0){
            *w=1;
            return 0;
        }else if(strcmp(type,"R")==0){
            *w=0;
            return 0;
        }else{
            fprintf(stderr,"The type of a request should be \"R\" or \"W\" but not: \"%s\" \n",type);
            return 1;
        }
    }
    // Adress should be non-negative and not greater than 2^32-1
    int parseAdress(char* adress, uint32_t* addr){
        int valid_adress=convert_string_to_uint_32(adress,addr);
        if(valid_adress==1){
            fprintf(stderr,"Invalid adress: \"%s\"\n",adress);
            return 1;
        }
        return 0;
    }

    // added an adition conditional, which indicates that no data has been passed as argument indicating a potential read
    // when parsing the input-file empty data columns are not interpreted as errors--> an empty data column is " "
    int parseData(char* data, uint32_t* d, const uint8_t w, const uint8_t width){
        if(data==NULL) return 1;
        if(data[0]=='\0'){
            fprintf(stderr,"No data was specified! \n");
            return 1;
        }
        if(w==0){
            // the request is a read request, however data column wasnt empty
            if(data[0]!=' '){
                fprintf(stderr,"Read requests should have empty data columns!! \n");
                return 1;
            }else{
                 *d=0;
                return 0;
            }
        }else{
            // check if the number is non-negative and not greater than 2^32-1
            int valid_data=convert_string_to_uint_32(data,d);
            if(valid_data==1){
                fprintf(stderr,"Invalid data: \"%s\"\n",data);
                return 1;
            }

            //specific requirement of the task
            if(width==1 && *d>UINT8_MAX) {
                fprintf(stderr,"You are trying to write a byte value but the value \"%d\" you specified is larger than a byte! \n",*d);
                return 1;
            }
            return 0;
        }
    }

    // user should be non-negative and in range 0 to 255
    int parseUser(char* user,uint8_t* usr){
        int valid_user=convert_string_to_uint_8(user,usr);
        if(valid_user==1){
            fprintf(stderr,"Invalid user: \"%s\"\n",user);
            return 1;
        }
        return 0;
    }

    int parseWidth(char* wide, uint8_t* width){
        if(strcmp(wide,"T")==0){
            *width=4;
            return 0;
        }else if(strcmp(wide,"F")==0){
            *width=1;
            return 0;
        }else{
            fprintf(stderr,"The width of a request should be \"T\" ( 4 byte access) or \"F\" (1 byte access) but not: \"%s\" \n",wide);
            return 1;
        }
    }
