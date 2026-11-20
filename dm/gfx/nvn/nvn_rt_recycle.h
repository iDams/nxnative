// Pure-C policy helpers for recycled render-target placement heaps (nvn_mtl.m).
#ifndef NVN_RT_RECYCLE_H
#define NVN_RT_RECYCLE_H

#include <stdint.h>

enum { RH_CAP = 256u << 20, RH_SLACK = 4u << 20 };
#define RH_IDLE_NS 3000000000ull

typedef struct {
    uint64_t cap_bytes;
    uint64_t max_entry_bytes;
    uint64_t idle_ns;
    int exact_alloc;
} rh_policy;

static inline rh_policy rh_policy_for_platform(int is_ios, uint64_t avail_bytes, int pressure) {
    if (!is_ios) {
        return (rh_policy){(uint64_t)RH_CAP, (uint64_t)RH_CAP, RH_IDLE_NS, 0};
    }
    if (pressure != 0 || avail_bytes < (768ull << 20)) {
        return (rh_policy){0, 0, 0, 1};
    }
    if (avail_bytes < (1024ull << 20)) {
        return (rh_policy){16ull << 20, 8ull << 20, 500000000ull, 1};
    }
    if (avail_bytes < (1536ull << 20)) {
        return (rh_policy){48ull << 20, 16ull << 20, 1500000000ull, 1};
    }
    return (rh_policy){96ull << 20, 32ull << 20, 2000000000ull, 1};
}

static inline int rh_entry_expired(uint64_t now, uint64_t entry_t, uint64_t idle_ns) {
    return idle_ns == 0 || (now >= entry_t && (now - entry_t) >= idle_ns);
}

static inline uint64_t rh_alloc_size(uint64_t size, int exact_alloc) {
    if (exact_alloc) return size;
    uint64_t extra = size / 8;
    if (extra > (uint64_t)RH_SLACK) extra = (uint64_t)RH_SLACK;
    return size > UINT64_MAX - extra ? UINT64_MAX : size + extra;
}

static inline uint64_t rh_reuse_max(uint64_t size) {
    uint64_t half = size / 2;
    return size > UINT64_MAX - half ? UINT64_MAX : size + half;
}

static inline int rh_should_pool_entry(const rh_policy *p, uint64_t size) {
    if (!p || p->cap_bytes == 0 || size == 0 || size > p->cap_bytes) return 0;
    if (p->max_entry_bytes > 0 && size > p->max_entry_bytes) return 0;
    return 1;
}

static inline int rh_needs_evict(uint64_t total_bytes, uint64_t now, uint64_t oldest_t, const rh_policy *p) {
    if (!p) return 0;
    return total_bytes > p->cap_bytes || rh_entry_expired(now, oldest_t, p->idle_ns);
}

#endif
