
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
    // TODO
    // To guarantee that L2 is inclusive of L1, you may need an additional flag 
    // in L2 to indicate that the block is cached in L1
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
 * log2
 *
 * params: num, which is 2 raised to a positive integer
 * returns: log2 of the number
 */
int log2(int num) {
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

    int byte_offset = log2(WORDSIZE_BYTES);
    int block_offset = log2(words_per_block);
    int block_index_offset = byte_offset + block_offset;

    int block_index_size = log2(nsets);
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

// TODO
int cache_access(struct cache_t *cp, unsigned long address, 
        char access_type, unsigned long long now, struct cache_t *next_cp)
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
    int lru = -1;
    unsigned long long lru_ts = ULONG_MAX;
    struct cache_blk_t *lru_block;
    struct cache_blk_t *ablock;

    index = (address >> cp->block_index_offset) && cp->block_index_mask;
    tag = address >> tag_offset;

    for (column = 0; column < c->assoc; column++) {
        ablock = cp->blocks[index][column]; 

        // check for match
        if (ablock->valid && ablock->tag == tag) {
            // cache hit
            if (access_type == 'w') {
                ablock->dirty = 1;
            }
            ablock->ts = now;
            return cp->hit_latency;
        }

        // update LRU
        if (ablock->ts < lru_ts) {
            // this block is now the least recently used block
            lru = column;
            lru_ts = ablock->ts;
            lru_block = ablock;
        }
    }

    // getting block from next level, evicting dirty block
    if (lru_block->dirty) {



    return(cp->hit_latency);
}


