#include <stdint.h>
#include "structs.h"


int parse_single_line(char *adress_ptr, char *data_ptr, char *type_ptr, char *user_ptr, char *wide_ptr,  Request *result);

int assemble(char *input,  Request *requests);

int assemble_with_header(const char* input, Request* requests);