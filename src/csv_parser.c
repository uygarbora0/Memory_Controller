#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "field_parsers.h"
#include "parser_utilities.h"
#include "structs.h"
#include <stdint.h>
 
 
// return 0 if everything was parsed correctly else 1
 int parse_single_line(char* adress_ptr,char*data_ptr, char*type_ptr, char* user_ptr, char* wide_ptr,  Request* request)
    {
        uint32_t adress;
        uint32_t data;
        uint8_t w;
        uint8_t user;
        uint8_t wide;

        int valid_type=parseType(type_ptr,&w);
        int valid_adress=parseAdress(adress_ptr,&adress);
        int valid_user=parseUser(user_ptr,&user);
        int valid_wide=parseWidth(wide_ptr,&wide);


        if(valid_adress==0 && valid_type==0 && valid_user==0 && valid_wide==0){
            int valid_data=parseData(data_ptr,&data,w,wide);
            if(valid_data==0){
                // All parsed parts of the line are valid--> A corresponding request is created and written to request 
                Request req={adress,data,w,user,wide};
                *request=req;
                return 0;
            }
            return 1;
        }
        return 1;
    }

  

    // takes the content of csv-file without the header and returns the number of requests if every single line was valid and else 0 
    int assemble(const char* input,  Request* requests){
        uint32_t count=0;
       

        char* lines=(char*)malloc(strlen(input)+1);
        if(lines==NULL){
            fprintf(stderr, "Memory allocation failed\n");
            return 0;
        }
        strcpy(lines,input);
        char* last_new_line=lines;
        
        while(1){
            
            while(*last_new_line==' ' || *last_new_line=='\n' || *last_new_line=='\t'){
                //trim any whitespace before the current line and also skip empty lines
                last_new_line++;
            }
           

            if(*last_new_line=='\0'){
                // End of the file has been reached--> break out of the loop
                break;
            }

            char* new_next_line=strchr(last_new_line,'\n');
            if(new_next_line==NULL){
                // if new_next_line is NULL then last_new_line is currently at the last line
                new_next_line=strchr(last_new_line,'\0');
                //new_next_line points to '\0'
            }

            
            // Allocate space in heap of size : (new_next_line-last_new_line= number of chars in that line) (+1 also considering the \0)
            char* adress_ptr=(char*)malloc(new_next_line-last_new_line+1);
            char* data_ptr=(char*)malloc(new_next_line-last_new_line+1);
            char* type_ptr=(char*)malloc(new_next_line-last_new_line+1);
            char* user_ptr=(char*)malloc(new_next_line-last_new_line+1);
            char* wide_ptr=(char*)malloc(new_next_line-last_new_line+1);

            if(adress_ptr==NULL || data_ptr==NULL || type_ptr==NULL || user_ptr==NULL || wide_ptr==NULL){
                free(adress_ptr);
                free(data_ptr);
                free(type_ptr);
                free(user_ptr);
                free(wide_ptr);
                free(lines);
                fprintf(stderr, "Memory allocation failed\n");
                return 0;
            }

            // initialization of empty strings-->if something goes wrong and these buffers somehow are passed to helper-functions no buffer-overflow happens
            adress_ptr[0]='\0';
            data_ptr[0]='\0';
            type_ptr[0]='\0';
            user_ptr[0]='\0';
            wide_ptr[0]='\0';

            char* buffers[]={type_ptr,adress_ptr,data_ptr,user_ptr,wide_ptr};

            for(int i=0;i<5;i++){
                int comma_was_found=0;
                char *write_ptr = buffers[i];

                if(i==0&&*last_new_line==','){
                    // special error handling for a line starting with "," to provide detailed error message for the user
                    fprintf(stderr,"The %d'th line of the csv-file starts with a \",\" (comma) which shouldnt! Remove the comma and check if the your input-file has the right format!\n",count+1);
                    free(adress_ptr);
                    free(data_ptr);
                    free(type_ptr);
                    free(user_ptr);
                    free(wide_ptr);
                    free(lines);
                    return 0;
                }

                while (*last_new_line != ' ' && *last_new_line != '\n' && *last_new_line != '\t' && *last_new_line != '\0' && *last_new_line != ',') {
                    // write the corresponding chars into the corresponding buffer
                    *write_ptr = *last_new_line;
                    write_ptr++;
                    last_new_line++;
                }

                comma_was_found=(*last_new_line==',');
                *write_ptr='\0';

                
                if(i==4){
                    // special error handling for chars after the 4th collumn to provide detailed error-message for the user
                    while ( *last_new_line == ' ' || *last_new_line == '\t') {
                        //trim all the whitespace after 4th collumn
                        last_new_line++;
                    }
                    if(*last_new_line!='\0' && *last_new_line!='\n'){
                        // if there are other characters after the 4th collumn, thats invalid format
                        fprintf(stderr,"There are some characters after the last (5th) argument of the %d'th line! Nothing is expected after the 5th argument.\n",count+1);
                        free(adress_ptr);
                        free(data_ptr);
                        free(type_ptr);
                        free(user_ptr);
                        free(wide_ptr);
                        free(lines);
                        return 0;
                    }
                    
                }
                else
                {
                  // after reading the current column , get rid of all the whitespace until the next comma and also get rid of that first comma that was encountered
                  while (*last_new_line == ',' || *last_new_line == ' ' || *last_new_line == '\t') {
                    
                    if (*last_new_line == ',') {
                        comma_was_found = 1;
                    }

                    
                    if(i!=1 && comma_was_found==1 && *(last_new_line+1)==','){
                        // there are 2 subsequent commas in a column except 2.column(2 subsequent commas  in column 3 are okay possibly indicating a read request)
                        fprintf(stderr, "2 subsequent commas without any argument in between was found. Possibly the %d'th argument was ommited in %d'th line!\n", i==0?i+1:i+2,count+1);
                        free(adress_ptr);
                        free(data_ptr);
                        free(type_ptr);
                        free(user_ptr);
                        free(wide_ptr);
                        free(lines);
                        return 0; 
                    }
                    
                    //if a comma was found after reading the adress column and the data column doesnt immediately begin with a char, then there is the possibility of an empyt data column (,,)
                    // checking for i==1 instead of i==2 because , the data column will  be directly parsed in the same "iteration" as the adress column
                    if(i==1 && *last_new_line==',' && (*(last_new_line+1) == ',' || *(last_new_line+1) == ' ' || *(last_new_line+1) == '\t' || *(last_new_line+1) == '\n' || *(last_new_line+1) == '\0')){
                        //in case of an empty data-column the data buffer will be set to " "
                        
                        // increment the pointer to point to the very next char after the currrent comma
                        last_new_line++;
                        while(*last_new_line==' '|| *last_new_line=='\t'){
                            //trim white space until a char except ' ' or '\t' is reached
                            last_new_line++;
                        }
                        
                        if(*last_new_line=='\n'||*last_new_line=='\0'){
                            fprintf(stderr,"Data column doesn't end with a comma and didnt conatin any value and was infinitely the last argument of the %d'th line!\n",count+1);
                            free(adress_ptr);
                            free(data_ptr);
                            free(type_ptr);
                            free(user_ptr);
                            free(wide_ptr);
                            free(lines);
                            return 0;
                        }
                        else if(*last_new_line==','){
                            char* ptr=last_new_line+1;
                            while(*ptr==' '|| *ptr=='\t'){
                                ptr++;
                            }
                            if(*ptr==','){
                                // third consecutive comma was found, indicating invalid format
                                fprintf(stderr,"Data column  possibly contains a comma in itself! Look at the %d'th line!\n",count+1);
                                free(adress_ptr);
                                free(data_ptr);
                                free(type_ptr);
                                free(user_ptr);
                                free(wide_ptr);
                                free(lines);
                                return 0;
                            }

                            // data column is empty
                            data_ptr[0]=' ';
                            data_ptr[1]='\0';
                            i=2;
                        }else{
                            // decrement the pointer if nothing extraordinary happened so that the last_new_line can be incremented as usual(look at line 219)
                            last_new_line--;
                        }
                    }
                    last_new_line++;
                }
                }

                if (*last_new_line == '\0' || *last_new_line == '\n') {
                    break;
                }

                if (!comma_was_found) {
                    // if a column didnt end with a comma, then this is invalid format
                    free(adress_ptr);
                    free(data_ptr);
                    free(type_ptr);
                    free(user_ptr);
                    free(wide_ptr);
                    free(lines);
                    fprintf(stderr, "Missing comma at %d'th line!\n",count+1);
                    return 0;
                }    

            }
            Request request;

            // current line has at least one invalid column
            if(parse_single_line(adress_ptr,data_ptr,type_ptr,user_ptr,wide_ptr,&request)==1){
                free(adress_ptr);
                free(data_ptr);
                free(user_ptr);
                free(type_ptr);
                free(wide_ptr);
                free(lines);
                fprintf(stderr, "Invalid line tried to be converted to a request! Look at the %d'th line of your file! (Empty lines are not considered) \n",count+1);
                return 0;
            }

            if(requests!=NULL){
                requests[count]=request;
            }
            count++;
            free(adress_ptr);
            free(data_ptr);
            free(type_ptr);
            free(user_ptr);
            free(wide_ptr);

            // set the last_new_line (pointer to the line which is being parsed) to the next line while checking if there are some chars after the 4th column
            while (last_new_line < new_next_line) {
                if (*last_new_line != '\n' && *last_new_line != '\0') {
                    free(lines);
                    return 0;
                }
                last_new_line++;
            }
            
            //end of the "file" is reached
            if (*new_next_line == '\0') {
                break;
            }

            //+1 to avoid pointing at '\n' or '\0'
            last_new_line=new_next_line+1;
        }
        free(lines);
        return count;

    }


    
  
     int assemble_with_header(const char* input, Request* requests){
        
        while(*input=='\t' || *input==' ' || *input=='\n'){
            input++;
        }
        const char* ptr_to_next_line=strchr(input,'\n');
        if(ptr_to_next_line==NULL){
            fprintf(stderr,"Our program only accepts csv-files with a header! The provided file possibly doesn't contain any line after the header or even no lines at all!\n");
            return 0;
        }

        
        char* header_line=(char*) malloc(ptr_to_next_line-input+1);
        if(header_line==NULL){
            fprintf(stderr,"Failed to allocate memory!\n");
            return 0;
        }

        char* current_column=(char*) malloc(ptr_to_next_line-input+1);
        if(current_column==NULL){
            free(header_line);
            fprintf(stderr,"Failed to allocate memory!\n");
            return 0;
        }

        strncpy(header_line, input,ptr_to_next_line-input);
        header_line[ptr_to_next_line-input]='\0';

        int number_of_columns=0;
        char *token = strtok(header_line, ",");
        while (1) {
            
            int i=helper_trim_leading_and_trailing(token,current_column);
            token = strtok(NULL, ",");

            if(token==NULL) break;

            if(i==1){
                fprintf(stderr,"Invalid column name:%s\n",token);
                free(header_line);
                free(current_column);
                return 0;
            }

            if(number_of_columns==0){
                if(strcmp("Type",current_column)!=0){
                    fprintf(stderr,"First column of the csv-header should be \"Type\" and not: \"%s\"",current_column);
                    free(header_line);
                    free(current_column);
                    return 0;
                }
            }else if(number_of_columns==1){
                if(strcmp("Address",current_column)!=0){
                    fprintf(stderr,"Second column of the csv-header should be \"Address\" and not: \"%s\"",current_column);
                    free(header_line);
                    free(current_column);
                    return 0;
                }
            }else if(number_of_columns==2){
                if(strcmp("Data",current_column)!=0){
                    fprintf(stderr,"Third column of the csv-header should be \"Data\" and not: \"%s\"",current_column);
                    free(header_line);
                    free(current_column);
                    return 0;
                }
            }else if(number_of_columns==3){
                if(strcmp("User",current_column)!=0){
                    fprintf(stderr,"Fourth column of the csv-header should be \"User\" and not: \"%s\"",current_column);
                    free(header_line);
                    free(current_column);
                    return 0;
                }
            }else if(number_of_columns==4){
                if(strcmp("Wide",current_column)!=0){
                    fprintf(stderr,"Fifth column of the csv-header should be \"Wide\" and not: \"%s\"",current_column);
                    free(header_line);
                    free(current_column);
                    return 0;
                }
            }else{
                break;
            }

             number_of_columns++;
            
        }

        free(header_line);
        free(current_column);

        if(number_of_columns!=4){
            fprintf(stderr,"The column number in header deviates from 5!\n");    
            return 0;
        }
        
        return assemble(ptr_to_next_line+1,requests);
    }
