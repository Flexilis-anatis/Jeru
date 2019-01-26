#include "src/runner.h"
#include "src/repl.h"
#include <stdio.h>
#include <stdlib.h>

char *read_file(char *filename)
{
   char *buffer = NULL;
   int string_size, read_size;
   FILE *handler = fopen(filename, "rb");

   if (handler)
   {
       fseek(handler, 0, SEEK_END);
       string_size = ftell(handler);
       rewind(handler);
       buffer = (char*) malloc(sizeof(char) * (string_size + 1) );
       read_size = fread(buffer, sizeof(char), string_size, handler);
       buffer[string_size] = '\0';
       if (string_size != read_size) {
           free(buffer);
           buffer = NULL;
       }

       fclose(handler);
    }

    return buffer;
}

int main(int argc, char **argv) {
    if (argc == 1) {
        run_repl();
    } else {
        JeruVM *vm = init_vm();
        for (int file_index = 1; file_index < argc; ++file_index) {
            char *file = read_file(argv[file_index]);
            set_source(file);
            if (!run_next_token(vm, NULL)) {
                free(file);
                free_vm(vm);
                return -1;
            }
            free(file);
        }
        free_vm(vm);
    }

    return 0;
}