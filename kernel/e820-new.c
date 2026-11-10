/**
 * @file e820.c
 * @author verner002
 * @date 19/07/2026
*/

#include <stddef.h>
#include <errno.h>

#include <kernel/e820-new.h>

#define NDESC       128
#define NNODE       (2 * NDESC)

struct e820_node {
    uint32_t address;
    struct e820_entry *entry;
};

static struct e820_node nodes[NNODE];
static struct e820_node *buffer[NNODE];
static size_t sbuffer;

int e820_sanitize(struct e820_entry **e820_map, int entries_count) {
    if (entries_count > NDESC)
        return -EINVAL; // too many entries

    for (int i = 0, j = 0; i < entries_count; ++i) {
        struct e820_entry *entry = e820_map[i];

        nodes[j++] = (struct e820_node){
            .address = entry->base,
            .entry = entry
        };

        nodes[j++] = (struct e820_node){
            .address = entry->base + entry->length - 1,
            .entry = entry
        };
    }

    int nodes_count = 2 * entries_count;

    int swap;

    do {
        swap = 0;

        // TODO: start from swap index (optimization)
        for (size_t i = 1; i < nodes_count; ++i) {
            // sort by address[asc], type[asc]
            if (nodes[i - 1].address > nodes[i].address || (nodes[i - 1].address == nodes[i].address && nodes[i - 1].entry->type > nodes[i].entry->type)) {
                struct e820_node temp = nodes[i - 1];
                nodes[i - 1] = nodes[i];
                nodes[i] = temp;
                swap = 1;
            }
        }
    } while (swap);

    sbuffer = 0;

    size_t index = 0;
    struct e820_node *node = &nodes[index++];
    struct e820_node *temp;
    uint32_t region_start;
    uint32_t region_type;
    uint32_t region_end;

    for (; index < nodes_count; ++index) {
        temp = &nodes[index];
        
        if (temp->entry == node->entry) {
            // we've reached end of region
            region_start = node->address;
            region_type = node->entry->type;
            region_end = temp->address;
            
            // load node with node with highest priority from buffer
            if (sbuffer) {
                size_t prior_i = 0;
                struct e820_node *prior = &buffer[prior_i];
                
                for (size_t i = 1; i < sbuffer; ++i) {
                    struct e820_node *tmp = &buffer[i];

                    if (tmp->entry->type > prior->entry->type) {
                        prior_i = i;
                        prior = tmp;
                    }
                }

                // move prior to end and decrement sbuffer
            } else // TODO: check if there are nodes to process
                node = &nodes[++index];

            ++index;
            break;
        } else if (temp->entry->type > node->entry->type) {
            // there is region with higher
            // type in our way
            region_end = temp->address - 1;
            buffer[sbuffer++] = node;
            node = temp;
            break;
        }
            
        buffer[sbuffer++] = temp;
    }

    push_region(region_start, region_type, region_end);


    while (1) {

        struct e820_node *prior = &buffer[0];

        for (size_t i = 1; i < sbuffer; ++i) {
            struct e820_node *temp = &buffer[i];

            if (temp->address < prior->address || (temp->address == prior->address && temp->entry->type > prior->entry->type))
                prior = temp;
        }

        
    }
}