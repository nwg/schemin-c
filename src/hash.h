#ifndef SCHEMIN_HASH_H
#define SCHEMIN_HASH_H
SCHEMIN_HASH_H

#include <stddef.h>
#include <stdint.h>

typedef struct hash_s hash_t;

hash_t *make_hash(uintmax_t num_buckets);
void destroy_hash(hash_t *hash);

void *hash_get(hash_t *hash, const char *key, size_t len);
void hash_set(hash_t *hash, const char *key, size_t len, void *data);

#endif
