// Recycled texture memory (nvn_mtl.m rh_texture, rh_place, GitHub issues #1831, #1823): a shared texture in a GPU-only pool is made in a placement heap of its own, an eighth larger
// than the texture; once its texture is freed the heap stays resident, and the smallest waiting heap that fits, up to half as large again, takes the next texture, cleared as a new
// allocation is: one a step of Metal's sizes larger included, COMPRESSIBLE or not. Same-pass feedback snapshots draw from the same heaps. Heaps that wait too long leave the
// residency set when a submission begins. CPU-visible pools, block-compressed textures and DM_NVN_RT_RECYCLE=0 keep allocations of their own.
#include "../nvn/nvn_mtl.m"
void *front_metal_layer(void) { return NULL; }
module_t *module_of(const void *p) { (void)p; return NULL; }
module_t *g_mod; int g_nmod;
void svc_thread_dump(void) {}
uint64_t now_ns(void) { return bp_now(); }
static int checks, failed;
#define CHECK(c, label) do { checks++; if (!(c)) { failed++; fprintf(stderr, "FAIL: %s (line %d)\n", label, __LINE__); } } while (0)

static tex_t target(pool_t *pool, uint64_t serial, uint64_t off, unsigned w, unsigned h) {
    tex_t t = {.magic = MAGIC('T','E','X',0), .serial = serial, .pool = pool, .mem = pool->mem + off, .off = (int64_t)off};
    t.d = (nvn_texdesc){.target = NVN_TEXTURE_TARGET_2D, .format = NVN_FORMAT_RGBA16F, .flags = NVN_TEXTURE_FLAGS_COMPRESSIBLE, .width = w, .height = h, .depth = 1, .levels = 1, .samples = 1};
    nvn_tex_layout(&t.d, &t.L); return t;
}
static int resident(id o) { pthread_mutex_lock(&g_res_mu); int r = [g_res_objects containsObject:o]; pthread_mutex_unlock(&g_res_mu); return r; }
static unsigned waiting(void) { os_unfair_lock_lock(&g_rh_lk); unsigned n = g_rh_n; os_unfair_lock_unlock(&g_rh_lk); return n; }
static void settle(void) {   // the released textures' fences pass and the deferred releases run (as in t_image_alias)
    mtl_sub_commit(); mtl_drain(); mtl_sub_begin(); mtl_sub_commit(); mtl_drain();
    dispatch_sync(g_feedback_q, ^{}); defer_drain(); mtl_sub_begin();
}
static void fill(id<MTLTexture> t, uint8_t v) {
    NSUInteger row = t.width * 8, bytes = row * t.height;
    id<MTLBuffer> b = [g_dev newBufferWithLength:bytes options:MTLResourceStorageModeShared]; memset(b.contents, v, bytes); res_add(b);
    enc_b_copy_buffer_texture(blit(), b, 0, row, bytes, MTLSizeMake(t.width, t.height, 1), t, 0, 0, MTLOriginMake(0, 0, 0));
    mtl_sub_commit(); mtl_drain(); CHECK(!mtl_last_commit_error(), "the fill completes without GPU error"); res_remove(b); mtl_sub_begin();
}
static int all_zero(id<MTLTexture> t) {
    NSUInteger row = t.width * 8, bytes = row * t.height;
    id<MTLBuffer> b = [g_dev newBufferWithLength:bytes options:MTLResourceStorageModeShared]; res_add(b);
    enc_b_copy_texture_buffer(blit(), t, 0, 0, MTLOriginMake(0, 0, 0), MTLSizeMake(t.width, t.height, 1), b, 0, row, bytes);
    mtl_sub_commit(); mtl_drain(); CHECK(!mtl_last_commit_error(), "the read completes without GPU error"); res_remove(b); mtl_sub_begin();
    const uint8_t *p = b.contents; for (NSUInteger i = 0; i < bytes; i++) if (p[i]) return 0;
    return 1;
}

int main(void) { @autoreleasepool {
    mtl_init(); void *mem = NULL; posix_memalign(&mem, 16384, 64 << 20); memset(mem, 0, 64 << 20);
    pool_t pool = {.magic = MAGIC('P','O','O','L'), .flags = NVN_MEMORY_POOL_FLAGS_CPU_NO_ACCESS | NVN_MEMORY_POOL_FLAGS_GPU_CACHED | NVN_MEMORY_POOL_FLAGS_COMPRESSIBLE, .mem = mem, .size = 64 << 20};
    mtl_sub_begin();
    if (!sw_on(rh_mode())) {   // DM_NVN_RT_RECYCLE=0
        tex_t a = target(&pool, 95001, 0, 640, 360); mtl_tex_init(&a);
        CHECK(tex_mtl(&a) && !tex_mtl(&a).heap && g_rh_made == 0, "with the switch off a target is its own allocation");
        mtl_tex_release(&a); mtl_alias_drop_pool(&pool); settle();
        printf("t_rt_recycle (off): %d checks, %d failed\n", checks, failed); return failed != 0;
    }
    __weak id<MTLHeap> first_heap;
    {
        tex_t a = target(&pool, 95001, 0, 640, 360); mtl_tex_init(&a);
        id<MTLTexture> ta = tex_mtl(&a);
        CHECK(ta && ta.heap && ta.width == 640 && ta.height == 360, "a GPU-only pool's target is placed in a heap of its own, at its own size");
        CHECK(resident(ta.heap) && !resident(ta), "its heap is what the residency set holds");
        CHECK(g_rh_made == 1 && g_rh_reused == 0, "a new heap is counted as made");
        MTLSizeAndAlign need = [g_dev heapTextureSizeAndAlignWithDescriptor:tex_desc(&a, 0)];
        CHECK(ta.heap.size >= need.size + need.size / 8, "the heap is made an eighth larger than its first target");
        fill(ta, 0x5a);
        first_heap = ta.heap;
        ta = nil; mtl_tex_release(&a); mtl_alias_drop_pool(&pool);   // the pool goes: the sharing table lets its texture go (deferred)
        settle();
    }
    CHECK(first_heap && waiting() == 1, "once its texture is freed the heap waits");
    CHECK(first_heap && resident(first_heap), "and stays resident while it waits");
    {
        tex_t b = target(&pool, 95002, 8 << 20, 632, 356); mtl_tex_init(&b);   // another place, a slightly smaller size
        id<MTLTexture> tb = tex_mtl(&b);
        CHECK(tb && tb.heap == first_heap && g_rh_reused == 1 && waiting() == 0, "the next target that fits is placed in the waiting heap");
        CHECK(tb.width == 632 && tb.height == 356, "at its own size");
        CHECK(all_zero(tb), "and is cleared as a new allocation is: nothing of the old target's pixels is read");
        tex_t c = target(&pool, 95003, 16 << 20, 1280, 720); mtl_tex_init(&c);
        id<MTLTexture> tc = tex_mtl(&c);
        CHECK(tc && tc.heap && tc.heap != tb.heap && g_rh_made == 2, "a target the waiting heaps cannot hold gets a new heap");
        tb = nil; tc = nil; mtl_tex_release(&b); mtl_tex_release(&c); mtl_alias_drop_pool(&pool);
        settle();
    }
    CHECK(waiting() == 2, "both heaps wait");
    {   // a heap more than an eighth larger than the target is not taken
        tex_t d = target(&pool, 95004, 0, 160, 90); mtl_tex_init(&d);
        id<MTLTexture> td = tex_mtl(&d);
        CHECK(td && td.heap && g_rh_made == 3 && waiting() == 2, "a much smaller target leaves the waiting heaps alone");
        td = nil; mtl_tex_release(&d); mtl_alias_drop_pool(&pool); settle();
    }
    {   // outside: a CPU-visible pool; inside: a texture without COMPRESSIBLE
        pool_t cpu = pool; cpu.flags = NVN_MEMORY_POOL_FLAGS_CPU_CACHED | NVN_MEMORY_POOL_FLAGS_GPU_CACHED | NVN_MEMORY_POOL_FLAGS_COMPRESSIBLE;
        tex_t e = target(&cpu, 95005, 0, 640, 360); mtl_tex_init(&e);
        CHECK(tex_mtl(&e) && !tex_mtl(&e).heap, "a CPU-visible pool's texture keeps its own allocation");
        tex_t f = target(&pool, 95006, 24 << 20, 644, 360); f.d.flags = 0; mtl_tex_init(&f);   // one step of Metal's sizes above the first target (1,982,464 -> 2,179,072 bytes on an M6)
        CHECK(tex_mtl(&f) && tex_mtl(&f).heap == first_heap, "a texture without COMPRESSIBLE, a size step larger than the heap's first target, takes the waiting heap");
        tex_t g = target(&pool, 95008, 32 << 20, 512, 512); g.d.flags = 0; g.d.format = NVN_FORMAT_RGBA_DXT1; nvn_tex_layout(&g.d, &g.L); mtl_tex_init(&g);
        CHECK(tex_mtl(&g) && !tex_mtl(&g).heap, "a block-compressed texture (an asset, never rendered to) keeps its own allocation");
        mtl_tex_release(&e); mtl_tex_release(&f); mtl_tex_release(&g); mtl_alias_drop_pool(&cpu); mtl_alias_drop_pool(&pool); settle();
    }
    unsigned before = waiting();
    CHECK(before == 3, "three heaps wait");
    os_unfair_lock_lock(&g_rh_lk); for (unsigned i = 0; i < g_rh_n; i++) g_rh[i].t -= RH_IDLE_NS; os_unfair_lock_unlock(&g_rh_lk);
    mtl_sub_commit(); mtl_drain(); mtl_sub_begin();   // rh_trim
    CHECK(waiting() == 0 && g_rh_trimmed == before && g_rh_bytes == 0, "heaps that waited too long are freed when a submission begins");
    CHECK(!first_heap || !resident(first_heap), "and leave the residency set");
    {   // a same-pass feedback snapshot is placed in recycled memory, and its heap waits once the snapshot goes
        int fresh = 0; uint64_t made = g_rh_made; fb_cache_t *e = fb_snap(95007, 0, 0, MTLPixelFormatRGBA16Float, 640, 360, 1, &fresh);
        CHECK(e && fresh && e->snap.heap && g_rh_made == made + 1 && resident(e->snap.heap), "a snapshot is placed in a recycled heap, resident");
        if (e) { defer_obj_release(e->snap); e->snap = nil; e->ser = 0; }
        settle();
        CHECK(waiting() == 1, "its heap waits once the snapshot is freed");
    }
    {   // iOS adaptive policy: drain on <768 MiB, exact allocation, max_entry rejection, >64 batch drain, and pressure trim
        rh_test_set(1, 1, 512ull << 20, 0);
        rh_trim();
        CHECK(waiting() == 0 && g_rh_bytes == 0, "switching to <768 MiB on iOS drains waiting heaps at once");

        rh_test_set(1, 1, 2048ull << 20, 0);
        tex_t ios_a = target(&pool, 95100, 0, 640, 360); mtl_tex_init(&ios_a);
        id<MTLTexture> tia = tex_mtl(&ios_a);
        MTLSizeAndAlign need_a = [g_dev heapTextureSizeAndAlignWithDescriptor:tex_desc(&ios_a, 0)];
        CHECK(tia && tia.heap && tia.heap.size >= need_a.size && tia.heap.size < need_a.size + need_a.size / 8, "iOS allocates exact placement heap without +12.5 percent headroom");
        id<MTLHeap> ios_heap = tia.heap;
        tia = nil; mtl_tex_release(&ios_a); mtl_alias_drop_pool(&pool); settle();
        CHECK(waiting() == 1, "iOS heap within budget pools after release");

        tex_t ios_b = target(&pool, 95101, 8 << 20, 632, 356); mtl_tex_init(&ios_b);
        id<MTLTexture> tib = tex_mtl(&ios_b);
        CHECK(tib && tib.heap == ios_heap && waiting() == 0, "smaller iOS target reuses the pooled exact heap");
        tib = nil; mtl_tex_release(&ios_b); mtl_alias_drop_pool(&pool); settle();
        rh_test_set(1, 1, 512ull << 20, 0);
        rh_trim();
        CHECK(waiting() == 0, "pooled iOS heap drains before entry-cap test");

        // Single-entry cap rejection under [768 MiB, 1024 MiB) where max_entry_bytes is 8 MiB
        rh_test_set(1, 1, 800ull << 20, 0);
        tex_t big = target(&pool, 95102, 0, 2048, 1024); mtl_tex_init(&big);
        id<MTLTexture> tbig = tex_mtl(&big);
        __weak id<MTLHeap> big_heap = tbig.heap;
        CHECK(tbig && big_heap && resident(big_heap), "oversized iOS target gets a live placement heap");
        tbig = nil; mtl_tex_release(&big); mtl_alias_drop_pool(&pool); settle();
        CHECK(waiting() == 0 && g_rh_bytes == 0, "heap exceeding iOS max_entry_bytes bypasses the pool");
        CHECK(!big_heap || !resident(big_heap), "bypassed oversized heap leaves residency at once");

        // >64 heaps drained to 0 via rh_wait when avail drops below 768 MiB
        rh_test_set(1, 1, 2048ull << 20, 0);
        __weak id<MTLHeap> batch_heaps[72];
        tex_t batch[72];
        for (unsigned i = 0; i < 72; i++) {
            batch[i] = target(&pool, 95200 + i, (uint64_t)i * (256ull << 10), 128, 128);
            mtl_tex_init(&batch[i]);
            batch_heaps[i] = tex_mtl(&batch[i]).heap;
        }
        for (unsigned i = 0; i < 72; i++) mtl_tex_release(&batch[i]);
        mtl_alias_drop_pool(&pool); settle();
        CHECK(waiting() == 72, "72 small iOS heaps pool under the 96 MiB high-memory budget");

        MTLHeapDescriptor *thd = [MTLHeapDescriptor new];
        thd.type = MTLHeapTypePlacement; thd.size = 256 << 10; thd.storageMode = MTLStorageModePrivate;
        id<MTLHeap> trig_h = [g_dev newHeapWithDescriptor:thd];
        res_add(trig_h);
        rh_test_set(1, 1, 512ull << 20, 0);
        uint64_t trimmed_before = g_rh_trimmed;
        rh_wait(trig_h);
        int all_unres = !resident(trig_h);
        trig_h = nil;
        for (unsigned i = 0; i < 72; i++) {
            if (batch_heaps[i] && resident(batch_heaps[i])) { all_unres = 0; break; }
        }
        CHECK(waiting() == 0 && g_rh_bytes == 0 && g_rh_trimmed == trimmed_before + 72, "rh_wait drains all 72 heaps across batches when iOS avail drops below 768 MiB");
        CHECK(all_unres, "all 72 drained heaps leave the residency set outside g_rh_lk");

        // >64 heaps drained to 0 under pressure != 0 via rh_trim
        rh_test_set(1, 1, 2048ull << 20, 0);
        for (unsigned i = 0; i < 70; i++) {
            batch[i] = target(&pool, 95300 + i, (uint64_t)i * (256ull << 10), 128, 128);
            mtl_tex_init(&batch[i]);
            (void)tex_mtl(&batch[i]);
        }
        for (unsigned i = 0; i < 70; i++) mtl_tex_release(&batch[i]);
        mtl_alias_drop_pool(&pool); settle();
        CHECK(waiting() == 70, "70 small iOS heaps pool before memory pressure signal");
        rh_test_set(1, 1, 2048ull << 20, 1);
        rh_trim();
        CHECK(waiting() == 0 && g_rh_bytes == 0, "iOS memory pressure drains all 70 heaps via rh_trim");

        // macOS keeps +12.5 percent headroom and 256 MiB budget even if avail is low and pressure is set
        rh_test_set(1, 0, 512ull << 20, 1);
        tex_t mac_t = target(&pool, 95400, 0, 640, 360); mtl_tex_init(&mac_t);
        id<MTLTexture> tmac = tex_mtl(&mac_t);
        MTLSizeAndAlign need_m = [g_dev heapTextureSizeAndAlignWithDescriptor:tex_desc(&mac_t, 0)];
        CHECK(tmac && tmac.heap && tmac.heap.size >= need_m.size + need_m.size / 8, "macOS keeps +12.5 percent headroom unchanged");
        tmac = nil; mtl_tex_release(&mac_t); mtl_alias_drop_pool(&pool); settle();
        CHECK(waiting() == 1, "macOS keeps pooling regardless of low avail or pressure");
        os_unfair_lock_lock(&g_rh_lk); for (unsigned i = 0; i < g_rh_n; i++) g_rh[i].t -= RH_IDLE_NS; os_unfair_lock_unlock(&g_rh_lk);
        rh_trim();
        CHECK(waiting() == 0, "aged macOS heap drains cleanly");
        rh_test_set(0, 0, 0, 0);
    }
    mtl_sub_commit(); mtl_drain(); free(mem);
    printf("t_rt_recycle: %d checks, %d failed\n", checks, failed); return failed != 0;
} }
