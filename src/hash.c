#include "hash.h"
#include <stdlib.h>
#include <string.h>

typedef struct bucket_s bucket_t;
struct bucket_s {
  const char *key;
  void *data;
  bucket_t *next;
};

struct hash_s {
  bucket_t **buckets;
  uintmax_t num_buckets;
};

hash_t *make_hash(uintmax_t num_buckets) {
  hash_t *hash = (hash_t*)malloc(sizeof(hash_t));
  hash->buckets = (bucket_t**)calloc(num_buckets, sizeof(bucket_t*));
  hash->num_buckets = num_buckets;

  return hash;
}

void destroy_hash(hash_t *hash) {
  for (uintmax_t i = 0; i < hash->num_buckets; i++) {
    bucket_t *bucket = hash->buckets[i];
    while (bucket != NULL) {
      free(bucket);
      bucket = bucket->next;
    }
  }

  free(hash->buckets);
  free(hash);
}

static uint64_t strhash64(const char *str, size_t len) {
  uint64_t g;
  uint64_t h = 0;

  for (size_t i = 0; i < len; i++) {
    char c = str[i];
    h = (h << 4) + (uint64_t)c;
    g = h & 0xF000000000000000LL;
    if (g != 0) {
      h = h ^ (g >> 56);
    }
    h = h & ~g;
  }
  return h;
}

static inline bucket_t *find_bucket_with_key(bucket_t *buckets, const char *key, size_t len) {
  for (bucket_t *bucket = buckets; bucket != NULL; bucket = bucket->next) {
    if (strncmp(key, bucket->key, len) == 0) {
      return bucket;
    }
  }

  return NULL;
}

void hash_set(hash_t *hash, const char *key, size_t len, void *data) {
  uint64_t code = strhash64(key, len);
  uintmax_t bucket_num = code % hash->num_buckets;
  bucket_t *buckets = hash->buckets[bucket_num];
  bucket_t *bucket = find_bucket_with_key(buckets, key, len);
  if (bucket != NULL) {
    bucket->data = data;
    return;
  }

  bucket_t *new_bucket = (bucket_t*)malloc(sizeof(bucket_t));
  new_bucket->next = buckets;
  new_bucket->data = data;
  new_bucket->key = strndup(key, len);
  hash->buckets[bucket_num] = new_bucket;
}

void *hash_get(hash_t *hash, const char *key, size_t len) {
  uint64_t code = strhash64(key, len);
  uintmax_t bucket_num = code % hash->num_buckets;
  bucket_t *buckets = hash->buckets[bucket_num];
  bucket_t *found = find_bucket_with_key(buckets, key, len);
  if (found != NULL) {
    return found->data;
  }

  return NULL;
}

