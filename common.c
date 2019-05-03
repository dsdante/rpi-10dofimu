#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Read a file to a new char buffer
// If *length==0, the required size determined in runtime.
// Must be freed by the caller.
char* read_file(const char *filename, size_t *length)
{
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Cannot open '%s': %s\n", filename, strerror(errno));
        *length = 0;
        return NULL;
    }
    if (*length == 0) {
        fseek(file, 0, SEEK_END);
        *length = ftell(file);
        rewind(file);
    }
    char* buffer = (char*) malloc((*length + 1) * sizeof(char));
    *length = fread(buffer, 1, *length, file);
    fclose(file);
    buffer[*length] = '\0';
    return buffer;
}
