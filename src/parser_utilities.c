#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>


// convert the provided string to a uint32_t if possible
// negative numbers and numbers greater than 2^32-1 are considered invalid
// if the "number" is invalid 1 is returned, else 0
int convert_string_to_uint_32(char* str, uint32_t* result){
        if(str==NULL) return 1; 
        if (str[0] == '\0') return 1 ;
        if(str[0]=='-') {
            
            if(str[1]=='\0'){
                fprintf(stderr,"The  argument %s is  not a number!\n",str);
                return 1;
            }
            if (str[1] != '\0' ) {
                fprintf(stderr,"The  argument %s is  a negative number, which it shouldn't be !\n",str);
                return 1;
            }    
        }
        char* endptr=NULL;
        errno=0;

        uint64_t res;
        if (str[0] == '0' && str[1] == 'x') {
            res = strtoul(str, &endptr, 16);
        }else{
            res = strtoul(str, &endptr, 10);
        }

        if(*endptr!='\0'){
            fprintf(stderr,"The  argument %s is not a number!\n",str);
            return 1;
        }
        if(res>UINT32_MAX || errno==ERANGE){
            fprintf(stderr,"The  argument : %s was larger than 4 bytes which resulted in an overflow!\n",str);
            return 1;
        }
    

        *result=(uint32_t)res;
        return 0;

}
 
    // converts the give string to a uint8_t
    // neagtive numbers and numbers greater than 255 are considered invalid
    // if the "number" is invalid 1 is returned, else 0
    int convert_string_to_uint_8(char* str, uint8_t* result){
        if(str==NULL) return 1; 
        if (str[0] == '\0') return 1 ;
        if(str[0]=='-') {
            //manually check if the number might be potentially neagtive
            if(str[1]=='\0'){
                fprintf(stderr,"The  argument %s is  not a number!\n",str);
                return 1;
            }
            if (str[1] != '0' ) {
                fprintf(stderr,"The  argument %s is  a negative number, which it shouldnt be !\n",str);
                return 1;
            }    
        }
        char* endptr=NULL;
        errno=0;

        uint64_t res;
        if (str[0] == '0' && str[1] == 'x') {
            res = strtoul(str, &endptr, 16);
        }else{
            res = strtoul(str, &endptr, 10);
        }

        if(*endptr!='\0'){
            fprintf(stderr,"The  argument %s is not a number!\n",str);
            return 1;
        }
        if(res>UINT8_MAX || errno==ERANGE){
            fprintf(stderr,"The  argument : %s was larger than 1 byte which resulted in an overflow!\n",str);
            return 1;
        }
    

        *result=(uint32_t)res;
        return 0;

}



// trims the leading and trailing whitespace off a token
// if the token does contain multiple words(char chunks) than returns 1 because individual column headers should only consist of a single word
int helper_trim_leading_and_trailing(const char* token, char* result){
        while(*token==' '|| *token=='\t'){
            token++;
        }

        while(*token!=' ' && *token!='\t' && *token!='\n' && *token!='\0'){
            *result=*token;
            result++;
            token++;
        }

        *result='\0';

         while(*token==' '|| *token=='\t'){
            token++;
        }

        if(*token!='\0'){
            return 1;
        }
        return 0;

}