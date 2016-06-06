/** 
 * \file   hash.c
 * \author kirill
 *
 * Created on June 5, 2016, 4:29 PM
 */

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "hash.h"
#include "queue.h"

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) \
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)\
                       +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

// http://www.azillionmonkeys.com/qed/hash.html

uint32_t SuperFastHash(const char * data, int len) {
    uint32_t hash = len, tmp;
    int rem;

    if (len <= 0 || data == NULL) return 0;

    rem = len & 3;
    len >>= 2;

    /* Main loop */
    for (; len > 0; len--) {
        hash += get16bits(data);
        tmp = (get16bits(data + 2) << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        data += 2 * sizeof (uint16_t);
        hash += hash >> 11;
    }

    /* Handle end cases */
    switch (rem) {
        case 3: hash += get16bits(data);
            hash ^= hash << 16;
            hash ^= ((signed char) data[sizeof (uint16_t)]) << 18;
            hash += hash >> 11;
            break;
        case 2: hash += get16bits(data);
            hash ^= hash << 11;
            hash += hash >> 17;
            break;
        case 1: hash += (signed char) *data;
            hash ^= hash << 10;
            hash += hash >> 1;
    }

    /* Force "avalanching" of final 127 bits */
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

//typedef unsigned long int ub4; /* unsigned 4-byte quantities */
//typedef unsigned char ub1; /* unsigned 1-byte quantities */

#define hashsize(n) ((ub4)1<<(n))
#define hashmask(n) (hashsize(n)-1)

/*
--------------------------------------------------------------------
mix -- mix 3 32-bit values reversibly.
For every delta with one or two bits set, and the deltas of all three
  high bits or all three low bits, whether the original value of a,b,c
  is almost all zero or is uniformly distributed,
 * If mix() is run forward or backward, at least 32 bits in a,b,c
  have at least 1/4 probability of changing.
 * If mix() is run forward, every bit of c will change between 1/3 and
  2/3 of the time.  (Well, 22/100 and 78/100 for some 2-bit deltas.)
mix() was built out of 36 single-cycle latency instructions in a 
  structure that could supported 2x parallelism, like so:
      a -= b; 
      a -= c; x = (c>>13);
      b -= c; a ^= x;
      b -= a; x = (a<<8);
      c -= a; b ^= x;
      c -= b; x = (b>>13);
      ...
  Unfortunately, superscalar Pentiums and Sparcs can't take advantage 
  of that parallelism.  They've also turned some of those single-cycle
  latency instructions into multi-cycle latency instructions.  Still,
  this is the fastest good hash I could find.  There were about 2^^68
  to choose from.  I only looked at a billion or so.
--------------------------------------------------------------------
 */
#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

/*
--------------------------------------------------------------------
hash() -- hash a variable-length key into a 32-bit value
  k       : the key (the unaligned variable-length array of bytes)
  len     : the length of the key, counting by bytes
  initval : can be any 4-byte value
Returns a 32-bit value.  Every bit of the key affects every bit of
the return value.  Every 1-bit and 2-bit delta achieves avalanche.
About 6*len+35 instructions.

The best hash table sizes are powers of 2.  There is no need to do
mod a prime (mod is sooo slow!).  If you need less than 32 bits,
use a bitmask.  For example, if you need only 10 bits, do
  h = (h & hashmask(10));
In which case, the hash table should have hashsize(10) elements.

If you are hashing n strings (ub1 **)k, do it like this:
  for (i=0, h=0; i<n; ++i) h = hash( k[i], len[i], h);

By Bob Jenkins, 1996.  bob_jenkins@burtleburtle.net.  You may use this
code any way you wish, private, educational, or commercial.  It's free.

See http://burtleburtle.net/bob/hash/evahash.html
Use for hash table lookup, or anything where one collision in 2^^32 is
acceptable.  Do NOT use for cryptographic purposes.
--------------------------------------------------------------------
 */

ub4 hash_f(k, length, initval)
register ub1 *k; /* the key */
register ub4 length; /* the length of the key */
register ub4 initval; /* the previous hash, or an arbitrary value */
{
    register ub4 a, b, c, len;

    /* Set up the internal state */
    len = length;
    a = b = 0x9e3779b9; /* the golden ratio; an arbitrary value */
    c = initval; /* the previous hash value */

    /*---------------------------------------- handle most of the key */
    while (len >= 12) {
        a += (k[0] +((ub4) k[1] << 8) +((ub4) k[2] << 16) +((ub4) k[3] << 24));
        b += (k[4] +((ub4) k[5] << 8) +((ub4) k[6] << 16) +((ub4) k[7] << 24));
        c += (k[8] +((ub4) k[9] << 8) +((ub4) k[10] << 16)+((ub4) k[11] << 24));
        mix(a, b, c);
        k += 12;
        len -= 12;
    }

    /*------------------------------------- handle the last 11 bytes */
    c += length;
    switch (len) /* all the case statements fall through */ {
        case 11: c += ((ub4) k[10] << 24);
        case 10: c += ((ub4) k[9] << 16);
        case 9: c += ((ub4) k[8] << 8);
            /* the first byte of c is reserved for the length */
        case 8: b += ((ub4) k[7] << 24);
        case 7: b += ((ub4) k[6] << 16);
        case 6: b += ((ub4) k[5] << 8);
        case 5: b += k[4];
        case 4: a += ((ub4) k[3] << 24);
        case 3: a += ((ub4) k[2] << 16);
        case 2: a += ((ub4) k[1] << 8);
        case 1: a += k[0];
            /* case 0: nothing left to add */
    }
    mix(a, b, c);
    /*-------------------------------------------- report the result */
    return c;
}

// -----------------------------------------------------------------------------

trudpHashTdata *trudpHashTnew(size_t size) {

    int i;
    trudpHashTdata *ht = (trudpHashTdata *)malloc(sizeof(trudpHashTdata) + 
            size * sizeof(trudpQueue*));
    ht->size = size;
    
    for(i = 0; i < size; i++) {
        ht->q[i] = trudpQueueNew();
    }
    
    return ht;
}

void trudpHashTdestroy(trudpHashTdata *ht) {
    
    if(ht) {
        
        int i;
        for(i = 0; i < ht->size; i++) {
            trudpQueueDestroy(ht->q[i]);
        }
        free(ht);
    }
}

typedef struct trudpHashTvalueData {
    
    uint32_t hash;
    size_t key_length;
    size_t data_length;
    char data[];
    
} trudpHashTvalueData;


void *trudpHashTAdd(trudpHashTdata *ht, void *key, size_t key_length, void *data, 
        size_t data_length) {

    size_t htd_length = sizeof(trudpHashTvalueData) + key_length + data_length;    
    trudpHashTvalueData *htd = (trudpHashTvalueData *) malloc(htd_length);
    htd->hash = hash_f(key, key_length, HASH_TABLE_INITVAL);
    htd->key_length = key_length;
    htd->data_length = data_length;
    memcpy(htd->data, key, key_length);
    memcpy(htd->data + htd->key_length, data, data_length);
    
    int idx = htd->hash % ht->size;    
    trudpQueueData *tqd = trudpQueueAdd(ht->q[idx], (void*)htd, htd_length);
    free(htd);
    
    htd = (trudpHashTvalueData *)tqd->data;
            
    return htd->data + htd->key_length;
}

/**
 * Get key data from hash table
 * 
 * @param ht Pointer to trudpHashTdata
 * @param key Key
 * @param key_length Key length
 * @param data_length [out] Pointer to data length
 * 
 * @return Data of selected key or (void*)-1 if not found
 */
void *trudpHashTGet(trudpHashTdata *ht, void *key, size_t key_length, 
        size_t *data_length) {
    
    void *data = (void*)-1;
    *data_length = 0;
            
    uint32_t hash = hash_f(key, key_length, HASH_TABLE_INITVAL);
    int idx = hash % ht->size;
    trudpHashTvalueData *htd;
    trudpQueueData *tqd;
    trudpQueueIterator *it = trudpQueueIteratorNew(ht->q[idx]);
    if(it != NULL) {
      while((tqd = trudpQueueIteratorNext(it))) {
        
        htd = (trudpHashTvalueData *)tqd->data;  
        if(htd->hash == hash) {
            
            if(key_length == htd->key_length && 
               !memcmp(htd->data, key, key_length)) {
            
                data = htd->data + htd->key_length;
                *data_length = htd->data_length;
                break;
            }
        }
      }
      trudpQueueIteratorFree(it);
    }
    
    return data;
}
