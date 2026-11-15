// Pure-C CPU-only tests for recycled render-target heap policy (nvn_rt_recycle.h).
#include "../nvn/nvn_rt_recycle.h"
#include <inttypes.h>
#include <stdio.h>

static int checks, failed;
#define CHECK(c, label) do { checks++; if (!(c)) { failed++; fprintf(stderr, "FAIL: %s (line %d)\n", label, __LINE__); } } while (0)

typedef struct {
    uint64_t size;
    uint64_t t;
} sim_ent;

static unsigned sim_drain(sim_ent *pool, unsigned *n, uint64_t *bytes, uint64_t now, const rh_policy *p, unsigned *batches) {
    unsigned evicted = 0;
    *batches = 0;
    for (;;) {
        unsigned nd = 0;
        while (*n && nd < 64) {
            unsigned old = 0;
            for (unsigned i = 1; i < *n; i++) {
                if (pool[i].t < pool[old].t) old = i;
            }
            if (!rh_needs_evict(*bytes, now, pool[old].t, p)) break;
            *bytes -= pool[old].size;
            pool[old] = pool[--(*n)];
            nd++;
            evicted++;
        }
        if (!nd) break;
        (*batches)++;
        if (nd < 64 || *n == 0) break;
    }
    return evicted;
}

int main(void) {
    // macOS defaults remain unchanged regardless of available memory or pressure
    for (int pressure = 0; pressure <= 1; pressure++) {
        uint64_t avails[3] = {0, 512ull << 20, 4096ull << 20};
        for (int i = 0; i < 3; i++) {
            rh_policy p = rh_policy_for_platform(0, avails[i], pressure);
            CHECK(p.cap_bytes == (256ull << 20), "macOS cap_bytes is 256 MiB");
            CHECK(p.max_entry_bytes == (256ull << 20), "macOS max_entry_bytes is 256 MiB");
            CHECK(p.idle_ns == 3000000000ull, "macOS idle_ns is 3s");
            CHECK(p.exact_alloc == 0, "macOS keeps +12.5 percent headroom");
        }
    }

    // iOS tier 0: pressure != 0 or avail < 768 MiB -> immediate bypass and full drain
    {
        rh_policy p0 = rh_policy_for_platform(1, 0, 0);
        rh_policy p1 = rh_policy_for_platform(1, (768ull << 20) - 1, 0);
        rh_policy p2 = rh_policy_for_platform(1, 4096ull << 20, 1);
        CHECK(p0.cap_bytes == 0 && p0.max_entry_bytes == 0 && p0.idle_ns == 0 && p0.exact_alloc == 1, "iOS 0 avail enters critical bypass tier");
        CHECK(p1.cap_bytes == 0 && p1.max_entry_bytes == 0 && p1.idle_ns == 0 && p1.exact_alloc == 1, "iOS <768 MiB enters critical bypass tier");
        CHECK(p2.cap_bytes == 0 && p2.max_entry_bytes == 0 && p2.idle_ns == 0 && p2.exact_alloc == 1, "iOS pressure overrides high avail into critical tier");
    }

    // iOS tier 1: [768 MiB, 1024 MiB) -> 16 MiB cap, 8 MiB entry, 500ms idle
    {
        rh_policy lo = rh_policy_for_platform(1, 768ull << 20, 0);
        rh_policy hi = rh_policy_for_platform(1, (1024ull << 20) - 1, 0);
        CHECK(lo.cap_bytes == (16ull << 20) && lo.max_entry_bytes == (8ull << 20) && lo.idle_ns == 500000000ull && lo.exact_alloc == 1, "iOS 768 MiB lower bound");
        CHECK(hi.cap_bytes == (16ull << 20) && hi.max_entry_bytes == (8ull << 20) && hi.idle_ns == 500000000ull && hi.exact_alloc == 1, "iOS <1024 MiB upper bound");
    }

    // iOS tier 2: [1024 MiB, 1536 MiB) -> 48 MiB cap, 16 MiB entry, 1.5s idle
    {
        rh_policy lo = rh_policy_for_platform(1, 1024ull << 20, 0);
        rh_policy hi = rh_policy_for_platform(1, (1536ull << 20) - 1, 0);
        CHECK(lo.cap_bytes == (48ull << 20) && lo.max_entry_bytes == (16ull << 20) && lo.idle_ns == 1500000000ull && lo.exact_alloc == 1, "iOS 1024 MiB lower bound");
        CHECK(hi.cap_bytes == (48ull << 20) && hi.max_entry_bytes == (16ull << 20) && hi.idle_ns == 1500000000ull && hi.exact_alloc == 1, "iOS <1536 MiB upper bound");
    }

    // iOS tier 3: >= 1536 MiB -> 96 MiB cap, 32 MiB entry, 2s idle
    {
        rh_policy lo = rh_policy_for_platform(1, 1536ull << 20, 0);
        rh_policy hi = rh_policy_for_platform(1, 4096ull << 20, 0);
        CHECK(lo.cap_bytes == (96ull << 20) && lo.max_entry_bytes == (32ull << 20) && lo.idle_ns == 2000000000ull && lo.exact_alloc == 1, "iOS 1536 MiB lower bound");
        CHECK(hi.cap_bytes == (96ull << 20) && hi.max_entry_bytes == (32ull << 20) && hi.idle_ns == 2000000000ull && hi.exact_alloc == 1, "iOS high memory tier");
    }

    // Single-entry cap rejection
    {
        rh_policy crit = rh_policy_for_platform(1, 512ull << 20, 0);
        rh_policy low = rh_policy_for_platform(1, 800ull << 20, 0);
        rh_policy cap_only = {10ull << 20, 0, 1000ull, 1};
        CHECK(!rh_should_pool_entry(NULL, 4096), "null policy rejects entry");
        CHECK(!rh_should_pool_entry(&low, 0), "zero-size entry is rejected");
        CHECK(!rh_should_pool_entry(&crit, 4096), "zero-cap policy rejects any entry");
        CHECK(rh_should_pool_entry(&low, 8ull << 20), "entry equal to max_entry_bytes is accepted");
        CHECK(!rh_should_pool_entry(&low, (8ull << 20) + 1), "entry above max_entry_bytes is rejected");
        CHECK(rh_should_pool_entry(&cap_only, 10ull << 20), "entry equal to cap_bytes when max_entry_bytes is 0 is accepted");
        CHECK(!rh_should_pool_entry(&cap_only, (10ull << 20) + 1), "entry above cap_bytes is rejected");
    }

    // Underflow protection when now < entry_t
    {
        CHECK(!rh_entry_expired(100, 200, 500000000ull), "now < entry_t does not underflow or expire when idle_ns > 0");
        CHECK(rh_entry_expired(100, 200, 0), "idle_ns == 0 expires immediately even when now < entry_t");
        CHECK(!rh_entry_expired(1000, 500, 501), "entry before idle_ns boundary is not expired");
        CHECK(rh_entry_expired(1000, 500, 500), "entry at idle_ns boundary is expired");
    }

    // Overflow-safe headroom and reuse sizing
    {
        CHECK(rh_alloc_size(800, 0) == 900, "macOS alloc adds +12.5 percent headroom");
        CHECK(rh_alloc_size(800, 1) == 800, "iOS alloc keeps exact size");
        CHECK(rh_alloc_size(UINT64_MAX, 0) == UINT64_MAX, "alloc headroom clamps UINT64_MAX safely");
        CHECK(rh_alloc_size(UINT64_MAX - 10, 0) == UINT64_MAX, "alloc headroom clamps near UINT64_MAX without wrap");
        CHECK(rh_alloc_size(UINT64_MAX, 1) == UINT64_MAX, "exact alloc preserves UINT64_MAX");
        CHECK(rh_reuse_max(800) == 1200, "reuse max adds +50 percent");
        CHECK(rh_reuse_max(UINT64_MAX) == UINT64_MAX, "reuse max clamps UINT64_MAX safely");
        CHECK(rh_reuse_max(UINT64_MAX - 10) == UINT64_MAX, "reuse max clamps near UINT64_MAX without wrap");
    }

    // Batch-drain (>64 entries) simulation: full drain on critical drop
    {
        sim_ent pool[150];
        unsigned n = 150;
        uint64_t bytes = 0;
        for (unsigned i = 0; i < n; i++) {
            pool[i].size = 512ull << 10;
            pool[i].t = 1000000000ull + (uint64_t)i * 1000ull;
            bytes += pool[i].size;
        }
        rh_policy crit = rh_policy_for_platform(1, 500ull << 20, 0);
        unsigned batches = 0;
        unsigned evicted = sim_drain(pool, &n, &bytes, 1000000000ull, &crit, &batches);
        CHECK(evicted == 150 && n == 0 && bytes == 0 && batches == 3, "critical tier drains >64 heaps completely across 3 batches");
    }

    // Batch-drain (>64 entries) simulation: partial convergence to 16 MiB cap
    {
        sim_ent pool[150];
        unsigned n = 150;
        uint64_t bytes = 0;
        for (unsigned i = 0; i < n; i++) {
            pool[i].size = 512ull << 10;
            pool[i].t = 2000000000ull + (uint64_t)i * 1000ull;
            bytes += pool[i].size;
        }
        rh_policy low = rh_policy_for_platform(1, 800ull << 20, 0);
        unsigned batches = 0;
        unsigned evicted = sim_drain(pool, &n, &bytes, 2000200000ull, &low, &batches);
        CHECK(evicted == 118 && n == 32 && bytes == (16ull << 20) && batches == 2, "low tier trims oldest >64 entries down to exact 16 MiB cap");
    }

    // Batch-drain (>64 entries) simulation: mixed expired entries and future timestamps (underflow guard)
    {
        sim_ent pool[150];
        unsigned n = 150;
        uint64_t bytes = 0;
        uint64_t now = 5000000000ull;
        for (unsigned i = 0; i < n; i++) {
            pool[i].size = 64ull << 10;
            pool[i].t = i < 80 ? (now - 3000000000ull + i) : (now + 1000000ull + i);
            bytes += pool[i].size;
        }
        rh_policy high = rh_policy_for_platform(1, 2048ull << 20, 0);
        unsigned batches = 0;
        unsigned evicted = sim_drain(pool, &n, &bytes, now, &high, &batches);
        CHECK(evicted == 80 && n == 70 && batches == 2, "batch drain evicts 80 expired heaps and keeps 70 newer heaps without underflow");
    }

    printf("t_rt_recycle_policy: %d checks, %d failed\n", checks, failed);
    return failed != 0;
}
