typedef union {
    unsigned long long AO_whole;
    struct {AO_t AO_v1; AO_t AO_v2;} AO_parts;
} AO_double_t;
#define AO_HAVE_double_t
#define AO_val1 AO_parts.AO_v1
#define AO_val2 AO_parts.AO_v2


// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
