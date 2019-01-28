/// @cond PRIVATE
/// @file hashtable.c
/// @copyright BSD 2-clause. See LICENSE.txt for the complete license text
/// @author Dane Larsen

#include "hashtable.h"
#include "hashfunc.h"

#include "../lexer/block.h"

#include "murmur.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

uint32_t global_seed = 2976579735;


/// The hash entry struct. Acts as a node in a linked list.
struct hash_entry {
    /// A pointer to the key.
    char *key;

    /// A pointer to the value.
    JeruBlock *value;

    /// The size of the key in bytes.
    size_t key_size;

    /// The size of the value in bytes.
    size_t value_size;

    /// A pointer to the next hash entry in the chain (or NULL if none).
    /// This is used for collision resolution.
    struct hash_entry *next;
};

hash_entry he_copy(hash_entry *source) {
    hash_entry new_entry = *source;
    new_entry.value = malloc(sizeof(JeruBlock));
    JeruBlock tmp = copy_jeru_block(source->value);
    memcpy(new_entry.value, &tmp, sizeof(JeruBlock));
    if (new_entry.next) {
        new_entry.next = malloc(sizeof(JeruBlock));
        hash_entry tmp = he_copy(source->next);
        memcpy(new_entry.next, &tmp, sizeof(hash_entry));
    }
    return new_entry;
}

hash_table ht_copy(hash_table *source) {
    hash_table new = *source;
    new.array = malloc(source->array_size * sizeof(JeruBlock));
    for (unsigned int index = 0; index < source->array_size; ++index) {
        if (source->array[index] == NULL) {
            new.array[index] = NULL;
            continue;
        }
        new.array[index] = malloc(sizeof(hash_entry));
        hash_entry tmp = he_copy(source->array[index]);
        memcpy(new.array[index], &tmp, sizeof(hash_entry));
    }
    return new;
}

//----------------------------------
// HashEntry functions
//----------------------------------

/// @brief Creates a new hash entry.
/// @param flags Hash table flags.
/// @param key A pointer to the key.
/// @param key_size The size of the key in bytes.
/// @param value A pointer to the value.
/// @param value_size The size of the value in bytes.
/// @returns A pointer to the hash entry.
hash_entry *he_create(void *key, size_t key_size, void *value,
        size_t value_size);

/// @brief Destroys the hash entry and frees all associated memory.
/// @param flags The hash table flags.
/// @param hash_entry A pointer to the hash entry.
void he_destroy(hash_entry *entry);

/// @brief Compare two hash entries.
/// @param e1 A pointer to the first entry.
/// @param e2 A pointer to the second entry.
/// @returns 1 if both the keys and the values of e1 and e2 match, 0 otherwise.
///          This is a "deep" compare, rather than just comparing pointers.
int he_key_compare(hash_entry *e1, hash_entry *e2);

/// @brief Sets the value on an existing hash entry.
/// @param flags The hashtable flags.
/// @param entry A pointer to the hash entry.
/// @param value A pointer to the new value.
/// @param value_size The size of the new value in bytes.
void he_set_value(hash_entry *entry, void *value, size_t value_size);

//-----------------------------------
// HashTable functions
//-----------------------------------

void ht_init(hash_table *table, double max_load_factor)
{
    table->hashfunc_x86_32  = MurmurHash3_x86_32;
    table->hashfunc_x86_128 = MurmurHash3_x86_128;
    table->hashfunc_x64_128 = MurmurHash3_x64_128;

    table->array_size   = HT_INITIAL_SIZE;
    table->array        = malloc(table->array_size * sizeof(*(table->array)));

    table->key_count            = 0;
    table->collisions           = 0;
    table->max_load_factor      = max_load_factor;
    table->current_load_factor  = 0.0;

    unsigned int i;
    for(i = 0; i < table->array_size; i++)
    {
        table->array[i] = NULL;
    }

    return;
}

void ht_destroy(hash_table *table)
{
    unsigned int i;
    hash_entry *entry;
    hash_entry *tmp;

    // crawl the entries and delete them
    for(i = 0; i < table->array_size; i++) {
        entry = table->array[i];

        while(entry != NULL) {
            tmp = entry->next;
            he_destroy(entry);
            entry = tmp;
        }
    }

    table->hashfunc_x86_32 = NULL;
    table->hashfunc_x86_128 = NULL;
    table->hashfunc_x64_128 = NULL;
    table->array_size = 0;
    table->key_count = 0;
    table->collisions = 0;
    free(table->array);
    table->array = NULL;
}

void ht_insert(hash_table *table, char *key, size_t key_size, JeruBlock *block)
{
    hash_entry *entry = he_create(key, key_size, block, sizeof(JeruBlock));

    ht_insert_he(table, entry);
}

// this was separated out of the regular ht_insert
// for ease of copying hash entries around
void ht_insert_he(hash_table *table, hash_entry *entry){
    hash_entry *tmp;
    unsigned int index;

    entry->next = NULL;
    index = ht_index(table, entry->key, entry->key_size);
    tmp = table->array[index];

    // if true, no collision
    if(tmp == NULL)
    {
        table->array[index] = entry;
        table->key_count++;
        return;
    }

    // walk down the chain until we either hit the end
    // or find an identical key (in which case we replace
    // the value)
    while(tmp->next != NULL)
    {
        if(he_key_compare(tmp, entry))
            break;
        else
            tmp = tmp->next;
    }

    if(he_key_compare(tmp, entry))
    {
        // if the keys are identical, throw away the old entry
        // and stick the new one into the table
        he_set_value(tmp, entry->value, entry->value_size);
        he_destroy(entry);
    }
    else
    {
        // else tack the new entry onto the end of the chain
        tmp->next = entry;
        table->collisions += 1;
        table->key_count ++;
        table->current_load_factor = (double)table->collisions / table->array_size;

        // double the size of the table if autoresize is on and the
        // load factor has gone too high
        if(table->current_load_factor > table->max_load_factor) {
            ht_resize(table, table->array_size * 2);
            table->current_load_factor =
                (double)table->collisions / table->array_size;
        }
    }
}

JeruBlock *ht_get(hash_table *table, char *key, size_t key_size)
{
    unsigned int index  = ht_index(table, key, key_size);
    hash_entry *entry   = table->array[index];
    hash_entry tmp;
    tmp.key             = key;
    tmp.key_size        = key_size;

    // once we have the right index, walk down the chain (if any)
    // until we find the right key or hit the end
    while(entry != NULL)
    {
        if(he_key_compare(entry, &tmp))
        {
            return entry->value;
        }
        else
        {
            entry = entry->next;
        }
    }

    return NULL;
}

void ht_remove(hash_table *table, char *key, size_t key_size)
{
    unsigned int index  = ht_index(table, key, key_size);
    hash_entry *entry   = table->array[index];
    hash_entry *prev    = NULL;
    hash_entry tmp;
    tmp.key             = key;
    tmp.key_size        = key_size;

    // walk down the chain
    while(entry != NULL)
    {
        // if the key matches, take it out and connect its
        // parent and child in its place
        if(he_key_compare(entry, &tmp))
        {
            if(prev == NULL)
                table->array[index] = entry->next;
            else
                prev->next = entry->next;

            table->key_count--;

            if(prev != NULL)
              table->collisions--;

            he_destroy(entry);
            return;
        }
        else
        {
            prev = entry;
            entry = entry->next;
        }
    }
}

int ht_contains(hash_table *table, char *key, size_t key_size)
{
    unsigned int index  = ht_index(table, key, key_size);
    hash_entry *entry   = table->array[index];
    hash_entry tmp;
    tmp.key             = key;
    tmp.key_size        = key_size;

    // walk down the chain, compare keys
    while(entry != NULL)
    {
        if(he_key_compare(entry, &tmp))
            return 1;
        else
            entry = entry->next;
    }

    return 0;
}

unsigned int ht_size(hash_table *table)
{
    return table->key_count;
}

void** ht_keys(hash_table *table, unsigned int *key_count)
{
    void **ret;

    if(table->key_count == 0){
      *key_count = 0;
      return NULL;
    }

    // array of pointers to keys
    ret = malloc(table->key_count * sizeof(void *));
    *key_count = 0;

    unsigned int i;
    hash_entry *tmp;

    // loop over all of the chains, walk the chains,
    // add each entry to the array of keys
    for(i = 0; i < table->array_size; i++)
    {
        tmp = table->array[i];

        while(tmp != NULL)
        {
            ret[*key_count]=tmp->key;
            *key_count += 1;
            tmp = tmp->next;
        }
    }

    return ret;
}

void ht_clear(hash_table *table)
{
    ht_destroy(table);

    ht_init(table, table->max_load_factor);
}

unsigned int ht_index(hash_table *table, char *key, size_t key_size)
{
    uint32_t index;
    // 32 bits of murmur seems to fare pretty well
    table->hashfunc_x86_32(key, key_size, global_seed, &index);
    index %= table->array_size;
    return index;
}

// new_size can be smaller than current size (downsizing allowed)
void ht_resize(hash_table *table, unsigned int new_size)
{
    hash_table new_table;

    new_table.hashfunc_x86_32 = table->hashfunc_x86_32;
    new_table.hashfunc_x86_128 = table->hashfunc_x86_128;
    new_table.hashfunc_x64_128 = table->hashfunc_x64_128;
    new_table.array_size = new_size;
    new_table.array = malloc(new_size * sizeof(hash_entry*));
    new_table.key_count = 0;
    new_table.collisions = 0;
    new_table.max_load_factor = table->max_load_factor;

    unsigned int i;
    for(i = 0; i < new_table.array_size; i++)
    {
        new_table.array[i] = NULL;
    }

    hash_entry *entry;
    hash_entry *next;
    for(i = 0; i < table->array_size; i++)
    {
        entry = table->array[i];
        while(entry != NULL)
        {
            next = entry->next;
            ht_insert_he(&new_table, entry);
            entry = next;
        }
        table->array[i]=NULL;
    }

    ht_destroy(table);

    table->hashfunc_x86_32 = new_table.hashfunc_x86_32;
    table->hashfunc_x86_128 = new_table.hashfunc_x86_128;
    table->hashfunc_x64_128 = new_table.hashfunc_x64_128;
    table->array_size = new_table.array_size;
    table->array = new_table.array;
    table->key_count = new_table.key_count;
    table->collisions = new_table.collisions;

}

void ht_set_seed(uint32_t seed){
    global_seed = seed;
}

//---------------------------------
// hash_entry functions
//---------------------------------

hash_entry *he_create(void *key, size_t key_size, void *value,
        size_t value_size)
{
    hash_entry *entry = malloc(sizeof(*entry));
    if(entry == NULL) {
        return NULL;
    }

    entry->key_size = key_size;
    {
        entry->key = malloc(key_size);
        if(entry->key == NULL) {
            free(entry);
            return NULL;
        }
        memcpy(entry->key, key, key_size);
    }

    entry->value_size = value_size;
    {
        entry->value = malloc(value_size);
        if(entry->value == NULL) {
            free(entry->key);
            free(entry);
            return NULL;
        }
        memcpy(entry->value, value, value_size);
    }

    entry->next = NULL;

    return entry;
}

void he_destroy(hash_entry *entry)
{
    free(entry->key);
    free(entry->value);
    free(entry);
}

int he_key_compare(hash_entry *e1, hash_entry *e2)
{
    char *k1 = e1->key;
    char *k2 = e2->key;

    if(e1->key_size != e2->key_size)
        return 0;

    return (memcmp(k1,k2,e1->key_size) == 0);
}

void he_set_value(hash_entry *entry, void *value, size_t value_size)
{
    {
        if(entry->value)
            free(entry->value);

        entry->value = malloc(value_size);
        if(entry->value == NULL) {
            return;
        }
        memcpy(entry->value, value, value_size);
    }
    entry->value_size = value_size;

    return;
}



