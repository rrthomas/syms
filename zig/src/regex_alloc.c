// Allocator function for regex_t
// Works around Zig's inability to cope with bitfields in structs
// See: https://stackoverflow.com/questions/73086494/how-to-allocate-a-struct-of-incomplete-type-in-zig

#include "config.h"

#include <regex.h>

#include "xalloc.h"

regex_t *regex_t_alloc(void) {
    return xzalloc(sizeof(regex_t));
}
