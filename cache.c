/*
 * EECS 370, University of Michigan
 * Project 4: LC-2K Cache Simulator
 * Instructions are found in the project spec.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_CACHE_SIZE 256
#define MAX_BLOCK_SIZE 256

// **Note** this is a preprocessor macro. This is not the same as a function.
// Powers of 2 have exactly one 1 and the rest 0's, and 0 isn't a power of 2.
#define is_power_of_2(val) (val && !(val & (val - 1)))


/*
 * Accesses 1 word of memory.
 * addr is a 16-bit LC2K word address.
 * write_flag is 0 for reads and 1 for writes.
 * write_data is a word, and is only valid if write_flag is 1.
 * If write flag is 1, mem_access does: state.mem[addr] = write_data.
 * The return of mem_access is state.mem[addr].
 */
extern int mem_access(int addr, int write_flag, int write_data);

/*
 * Returns the number of times mem_access has been called.
 */
extern int get_num_mem_accesses(void);

//Use this when calling printAction. Do not modify the enumerated type below.
enum actionType
{
    cacheToProcessor,
    processorToCache,
    memoryToCache,
    cacheToMemory,
    cacheToNowhere
};

/* You may add or remove variables from these structs */
typedef struct blockStruct
{
    int data[MAX_BLOCK_SIZE];
    int dirty;      // 0 = clean, 1 = dirty
    int lruLabel;   // empty = 0, the larger the more recent
    int tag;
    int valid;      // 0 = invalid, 1 = valid
} blockStruct;

typedef struct cacheStruct
{
    blockStruct blocks[MAX_CACHE_SIZE];
    int blockSize;
    int numSets;
    int blocksPerSet;
    int blockOffsetBit;
    int setIndexBit;
    // add any variables for end-of-run stats
} cacheStruct;

/* Global Cache variable */
cacheStruct cache;

void printAction(int, int, enum actionType);
void printCache(void);




/* Helper functions */


int getPower(int n) {
    if(n <= 0 || !(is_power_of_2(n)))
        return -1;
    int power = 0;
    while(n > 1) {
        n >>= 1;
        power++;
    }
    return power;
}

void update_lru(int set_index, int recent_block_number) {
    for(int i = set_index * blocksPerSet; i < (set_index + 1) * blocksPerSet; i++) {
            cache.blocks[i].lruLabel--;
    }
    cache.blocks[recent_block_number].lruLabel = blocksPerSet;
}

void write_back_cache_to_memory(int block_number) {     // block_number = lru block's index in all the cache
    return;
}

void get_from_memory(int set_index, int dest_tag) {

    int lru = 0, min_label = blocksPerSet;
    for(int i = set_index * blocksPerSet; i < (set_index + 1) * blocksPerSet; i++) {    // find lru
        if(cache.blocks[i].lruLabel < min_label) {
            min_label = cache.blocks[i].lruLabel;
            lru = i;
            if(!cache.blocks[i].valid)  // if we find an empty slot
                break;
        }
    }
    if(cache.blocks[lru].valid)
        write_back_cache_to_memory(lru);    // write back

    cache.blocks[lru].dirty = 0;
    cache.blocks[lru].tag = dest_tag;
    cache.blocks[lru].valid = 1;
    for(int j = 0; j < cache.blockSize; j++)
        cache.blocks[lru].data[j] = mem_access((dest_tag << (cache.blockOffsetBit + cache.setIndexBit)) + j, 0, 0); // access the memory
    update_lru(set_index, lru);


}




/* Helper functions end */





/*
 * Set up the cache with given command line parameters. This is
 * called once in main(). You must implement this function.
 */
void cache_init(int blockSize, int numSets, int blocksPerSet)
{
    if (blockSize <= 0 || numSets <= 0 || blocksPerSet <= 0) {
        printf("error: input parameters must be positive numbers\n");
        exit(1);
    }
    if (blocksPerSet * numSets > MAX_CACHE_SIZE) {
        printf("error: cache must be no larger than %d blocks\n", MAX_CACHE_SIZE);
        exit(1);
    }
    if (blockSize > MAX_BLOCK_SIZE) {
        printf("error: blocks must be no larger than %d words\n", MAX_BLOCK_SIZE);
        exit(1);
    }
    if (!is_power_of_2(blockSize)) {
        printf("warning: blockSize %d is not a power of 2\n", blockSize);
    }
    if (!is_power_of_2(numSets)) {
        printf("warning: numSets %d is not a power of 2\n", numSets);
    }
    printf("Simulating a cache with %d total lines; each line has %d words\n",
        numSets * blocksPerSet, blockSize);
    printf("Each set in the cache contains %d lines; there are %d sets\n",
        blocksPerSet, numSets);

    /********************* Initialize Cache *********************/

    cache.blockSize = blockSize;
    cache.numSets = numSets;
    cache.blocksPerSet = blocksPerSet;
    cache.blockOffsetBit = getPower(blockSize);
    cache.setIndexBit = getPower(numSets);
    
    for(int i = 0; i < numSets; i++) {
        cache.blocks[i].dirty = 0;
        cache.blocks[i].lruLabel = 0;
        cache.blocks[i].valid = 0;
    }

    return;
}


/*
 * Access the cache. This is the main part of the project,
 * and should call printAction as is appropriate.
 * It should only call mem_access when absolutely necessary.
 * addr is a 16-bit LC2K word address.
 * write_flag is 0 for reads (fetch/lw) and 1 for writes (sw).
 * write_data is a word, and is only valid if write_flag is 1.
 * The return of mem_access is undefined if write_flag is 1.
 * Thus the return of cache_access is undefined if write_flag is 1.
 */
int cache_access(int addr, int write_flag, int write_data)
{
    int block_offset = addr & ((1 << cache.blockOffsetBit) - 1);
    int set_index = (addr >> cache.blockOffsetBit) & ((1 << cache.setIndexBit) - 1);
    int tag = addr >> (cache.blockOffsetBit + cache.setIndexBit);

    if(!write_flag) {   // read
        
        for(int i = set_index * cache.blocksPerSet; i < (set_index+1) * cache.blocksPerSet; i++) {
            if(tag == cache.blocks[i].tag) {    // hit
                return cache.blocks[i].data[block_offset];
            }
        }


    }





    /* The next line is a placeholder to connect the simulator to
    memory with no cache. You will remove this line and implement
    a cache which interfaces between the simulator and memory. */
    return mem_access(addr, write_flag, write_data);


}


/*
 * print end of run statistics like in the spec. **This is not required**,
 * but is very helpful in debugging.
 * This should be called once a halt is reached.
 * DO NOT delete this function, or else it won't compile.
 * DO NOT print $$$ in this function
 */
void printStats(void)
{
    printf("End of run statistics:\n");
    return;
}

/*
 * Log the specifics of each cache action.
 *
 *DO NOT modify the content below.
 * address is the starting word address of the range of data being transferred.
 * size is the size of the range of data being transferred.
 * type specifies the source and destination of the data being transferred.
 *  -    cacheToProcessor: reading data from the cache to the processor
 *  -    processorToCache: writing data from the processor to the cache
 *  -    memoryToCache: reading data from the memory to the cache
 *  -    cacheToMemory: evicting cache data and writing it to the memory
 *  -    cacheToNowhere: evicting cache data and throwing it away
 */
void printAction(int address, int size, enum actionType type)
{
    printf("$$$ transferring word [%d-%d] ", address, address + size - 1);

    if (type == cacheToProcessor) {
        printf("from the cache to the processor\n");
    }
    else if (type == processorToCache) {
        printf("from the processor to the cache\n");
    }
    else if (type == memoryToCache) {
        printf("from the memory to the cache\n");
    }
    else if (type == cacheToMemory) {
        printf("from the cache to the memory\n");
    }
    else if (type == cacheToNowhere) {
        printf("from the cache to nowhere\n");
    }
    else {
        printf("Error: unrecognized action\n");
        exit(1);
    }

}

/*
 * Prints the cache based on the configurations of the struct
 * This is for debugging only and is not graded, so you may
 * modify it, but that is not recommended.
 */
void printCache(void)
{
    int blockIdx;
    int decimalDigitsForWaysInSet = (cache.blocksPerSet == 1) ? 1 : (int)ceil(log10((double)cache.blocksPerSet));
    printf("\ncache:\n");
    for (int set = 0; set < cache.numSets; ++set) {
        printf("\tset %i:\n", set);
        for (int block = 0; block < cache.blocksPerSet; ++block) {
            blockIdx = set * cache.blocksPerSet + block;
            if(cache.blocks[set * cache.blocksPerSet + block].valid) {
                printf("\t\t[ %0*i ] : ( V:T | D:%c | LRU:%-*i | T:%i )\n\t\t%*s{",
                    decimalDigitsForWaysInSet, block,
                    (cache.blocks[blockIdx].dirty) ? 'T' : 'F',
                    decimalDigitsForWaysInSet, cache.blocks[blockIdx].lruLabel,
                    cache.blocks[blockIdx].tag,
                    7+decimalDigitsForWaysInSet, "");
                for (int index = 0; index < cache.blockSize; ++index) {
                    printf(" 0x%08X", cache.blocks[blockIdx].data[index]);
                }
                printf(" }\n");
            }
            else {
                printf("\t\t[ %0*i ] : ( V:F )\n\t\t%*s{     }\n", decimalDigitsForWaysInSet, block, 7+decimalDigitsForWaysInSet, "");
            }
        }
    }
    printf("end cache\n");
}
