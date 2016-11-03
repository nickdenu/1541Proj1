
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#define WORDSIZE_BYTES 4
#define WORDSIZE_BITS 32

struct cache_blk_t {
    unsigned long tag;
    char valid;
    char dirty;
    unsigned long long ts;	//a timestamp that may be used to implement LRU replacement
    char inL1;
};

struct cache_t {
    // The cache is represented by a 2-D array of blocks. 
    // The first dimension of the 2D array is "nsets" which is the number of sets (entries)
    // The second dimension is "assoc", which is the number of blocks in each set.
    int nsets;				// # sets
    int blocksize;			// block size
    int assoc;				// associativity

    int block_index_offset;
    int block_index_mask;
    int tag_offset;

    int hit_latency;			// latency in case of a hit
    struct cache_blk_t **blocks;    // the array of cache blocks
};

/**
 * ilog2
 *
 * params: num, which is 2 raised to a positive integer
 * returns: log2 of the number
 */
int ilog2(int num) {
    int exponent = 0;

    // continue until 1 has been shifted to 0th position
    while ((num & 1) != 1) {
        num = num >> 1;
        exponent++;
    }
    return exponent;
}

/**
 * ones
 *
 * params: n number of consecutive ones
 * returns: an int containing the number of ones specified
 */
int ones(int n) {
    int res = 0;
    int i = 0;

    while (i < n) {
        res = res << 1;
        res |= 1;
        i++;
    }
    return res;
}


    struct cache_t *
cache_create(int size, int blocksize, int assoc, int latency)
{
    int i;
    int cachesize_bytes = size << 10; // convert KBytes to Bytes
    int nblocks = cachesize_bytes / blocksize;	// number of blocks in the cache
    int nsets = nblocks / assoc;			// number of sets (entries) in the cache

    int words_per_block = blocksize / WORDSIZE_BYTES; 

    int byte_offset = ilog2(WORDSIZE_BYTES);
    int block_offset = ilog2(words_per_block);
    int block_index_offset = byte_offset + block_offset;

    int block_index_size = ilog2(nsets);
    int block_index_mask = ones(block_index_size);

    int tag_offset = block_index_size + block_offset + byte_offset;

    struct cache_t *C = (struct cache_t *)calloc(1, sizeof(struct cache_t));

    C->nsets = nsets; 
    C->assoc = assoc;
    C->block_index_offset = block_index_offset;
    C->block_index_mask = block_index_mask;
    C->tag_offset = tag_offset;
    C->hit_latency = latency;

    C->blocks= (struct cache_blk_t **)calloc(nsets, sizeof(struct cache_blk_t));

    for(i = 0; i < nsets; i++) {
        C->blocks[i] = (struct cache_blk_t *)calloc(assoc, sizeof(struct cache_blk_t));
    }

    return C;
}

/**
 * update_not_in_L1
 *
 * Updates a cache entry in L2 to reflect that it was removed from L1
 */
void update_not_in_L1(struct cache_t *cp, unsigned long address) {
    int index, tag;
    int column;
    struct cache_blk_t *ablock;

    index = (address >> cp->block_index_offset) && cp->block_index_mask;
    tag = address >> cp->tag_offset;

    for (column = 0; column < cp->assoc; column++) {
        ablock = &(cp->blocks[index][column]); 

        // check for match
        if (ablock->valid && ablock->tag == tag) {
            ablock->inL1 = 0;
            return;
        }
    }
}


// TODO
int cache_access(struct cache_t *cp, unsigned long address, 
        char access_type, unsigned long long now, struct cache_t *next_cp,
        int level, int mem_latency, int eviction)
{
    //
    // Based on address, determine the set to access in cp and examine the blocks
    // in the set to check hit/miss and update the global hit/miss statistics
    // If a miss, determine the victim in the set to replace (LRU). Replacement for 
    // L2 blocks should observe the inclusion property.
    //
    // The function should return the hit_latency in case of a hit. In case
    // of a miss, you need to figure out a way to return the time it takes to service 
    // the request, which includes writing back the replaced block, if dirty, and bringing 
    // the requested block from the lower level (from L2 in case of L1, and from memory in case of L2).
    // This will require one or two calls to cache_access(L2, ...) for misses in L1.
    // It is recommended to start implementing and testing a system with only L1 (no L2). Then add the
    // complexity of an L2.
    // return(cp->hit_latency);
    
    int latency;
    int index, tag;
    int column;
    int lru;
    unsigned long long lru_ts = ULONG_MAX;
    struct cache_blk_t *lru_block;

    int evicted_address;
    int evicted_block_was_dirty;

    struct cache_blk_t *ablock;

    index = (address >> cp->block_index_offset) && cp->block_index_mask;
    tag = address >> cp->tag_offset;

    for (column = 0; column < cp->assoc; column++) {
        ablock = &(cp->blocks[index][column]); 

        // check for match
        if (ablock->valid && ablock->tag == tag) {
            // cache hit
            if (!eviction) {
                if (level == 1) L1hits++;
                if (level == 2) L2hits++;  
            }
            
            if (access_type == 'w') {
                ablock->dirty = 1;
            }
            ablock->ts = now;
            return cp->hit_latency;
        }

        // update LRU
        if (ablock->ts < lru_ts) {
            // observe inclusivity property for L2
            if (level != 2 || !ablock->inL1) {
                // this block is now the least recently used block
                lru = column;
                lru_ts = ablock->ts;
                lru_block = ablock;
            }
        }
    }

    // if you made it this far, there was a miss
    if (!eviction) {
        if (level == 1) L1misses++;
        if (level == 2) L2misses++;
    }

    // generate address for evicted block
    evicted_address = (lru_block->tag << cp->tag_offset) || (index << cp->block_index_offset);
    if (next_cp != NULL) {
        update_not_in_L1(next_cp, evicted_address);
    }
    evicted_block_was_dirty = lru_block->dirty;

    // replace LRU
    lru_block->tag = tag;
    lru_block->valid = 1;
    if (access_type == 'w') {
        lru_block->dirty = 1;
    } else if (access_type == 'r') {
        lru_block->dirty = 0;
    }
    lru_block->ts = now;
    lru_block->inL1 = 1;

    // getting block from next level, evicting block
    if (evicted_block_was_dirty) {
        if (next_cp == NULL) {
            // next level is memory
            // return cost to access this level 
            // + time to write back dirty block 
            // + time to get block from memory
            return cp->hit_latency
                + mem_latency
                + mem_latency;
        } else {
            // next level is L2 
            // return cost to access this level
            // + time to write back dirty block
            // + time to get block from next level
            return cp->hit_latency 
                + cache_access(next_cp, evicted_address, 'w', now, NULL, level+1, mem_latency, 1) 
                + cache_access(next_cp, address, access_type, now, NULL, level+1, mem_latency, 0);
        }
    } else {
        // block is not dirty
        if (next_cp == NULL) {
            // next level is memory
            // return cost to access this level
            // + time to get block from memory
            return cp->hit_latency + mem_latency;
        } else {
            // next level is L2
            // return cost to access this level
            // + time to get block from next level
            return cp->hit_latency + cache_access(next_cp, address, access_type, now, NULL, level+1, mem_latency, 0);
        }
    }
}
