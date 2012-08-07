// Include for declaring various types of hashtables.
// Used by table mods and also for creating local hash table types.

#include <unordered_map>
#include "heap_allocator.h"

#define DECLARE_MAP(name, _keyt, _valuet) \
typedef _keyt key_type; \
typedef _valuet value_type; \
typedef std::pair<key_type, value_type> pair_type; \
typedef std::heap_allocator<pair_type> name##_heap_allocator; \
typedef std::unordered_map<key_type, value_type, std::hash<key_type>, std::equal_to<key_type>, name##_heap_allocator> name##_t

// Usage: DECLARE_MAP(card64_table, card64_t, address_t);
