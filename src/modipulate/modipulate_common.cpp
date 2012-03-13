#include "modipulate_common.h"
#include <stdarg.h>

extern char* last_error;

// From the man page for snprintf
char* modipulate_make_message(const char *fmt, ...) {
    int n;
    int size = 100;     /* Guess we need no more than 100 bytes. */
    char *p;
    va_list ap;
    
    if ((p = new char[size]) == NULL)
        return NULL;
    
    while (1) {
        /* Try to print in the allocated space. */
        va_start(ap, fmt);
        n = vsnprintf(p, size, fmt, ap);
        va_end(ap);
        
       /* If that worked, return the string. */
       if (n > -1 && n < size)
            return p;
        
       /* Else try again with more space. */
       if (n > -1)    /* glibc 2.1 */
            size = n+1; /* precisely what is needed */
        else           /* glibc 2.0 */
            size *= 2;  /* twice the old size */
        
       delete p;
       p = new char[size];
    }
}

void modipulate_set_error_string(const char* fmt, ...) {
    va_list ap;
    if (last_error != NULL)
        delete last_error;
    
    last_error = modipulate_make_message(fmt, ap);
}

void modipulate_set_error_string_cpp(std::string err) {
    if (last_error != NULL)
        delete last_error;
    
    last_error = modipulate_make_message("%s", err.c_str());
}

// Checks for a PortAudio error, returns a Modipulate error and sets
// the last error string.
ModipulateErr modipulate_handle_pa_error(PaError err) {
    if (err != paNoError) {
        modipulate_set_error_string("PortAudio error: %d %s", err, Pa_GetErrorText(err));
        DPRINT("%s", last_error);
        
        return MODIPULATE_ERROR_GENERAL;
    }
    
    return MODIPULATE_ERROR_NONE;
}
