INTROSPECT(memory_layout) struct game_state_t
{
    b32 initialized = true;            // used by game
    platform_window_t* window;         // used by game, layer, resourcemgr
    Entity ents[MAX_ENTITIES] = {};    // used by entity, game, layer
};

typedef struct memory_element_t
{
    void*       address; // relative to beginning of state struct
    size_t      size;
    const char* type_name;
    const char* variable_name;
} memory_element_t;

typedef struct memory_layout_t {
    memory_element_t location_of_element_in_memory[100];
} memory_layout_t;


// scenarios:
// 1. member added to a struct
// 2. member removed from a struct
// 3. member variable renamed in a struct
// 4. member type changed in a struct
// 5. members reordered in a struct
// 6. member array shortened in a struct
// 7. member array lengthened in a struct

// 0x00 b32 4 bytes initialized  |  0x00 b32 4 bytes initialized
// 0x04 pfw 6 bytes window       |  0x04 pfw 6 bytes initialized
// 0x10 ent 4 bytes entity       |  0x10 ent 8 bytes entity

// idea:
// traverse all nodes of the old_memory_layout and new_memory_layout tree
// compare node for (address) size, type_name and variable_name


// problem:
// automatic padding
// -> turn off with a compiler switch?
