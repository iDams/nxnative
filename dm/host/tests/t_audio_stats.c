// Exercise the real callback while its diagnostic writer is blocked, without a sound device.
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdarg.h>
#include <dispatch/dispatch.h>
#include <string.h>
static _Thread_local int in_callback;
static _Atomic int callback_io, block_report, reports, line_slots;
static dispatch_semaphore_t report_entered, report_allow, report_done, released_event;
static char lines[8][512];
static void checked_flockfile(FILE *f) { (void)f; if (in_callback) atomic_fetch_add(&callback_io, 1); }
static void checked_funlockfile(FILE *f) { (void)f; }
static int checked_fputc(int c, FILE *f) { (void)f; return c; }
static int checked_fprintf(FILE *f, const char *fmt, ...) {
    (void)f;
    if (in_callback) { atomic_fetch_add(&callback_io, 1); return 0; }
    if (!strstr(fmt, "device callbacks")) return 0;
    if (atomic_exchange(&block_report, 0)) {
        dispatch_semaphore_signal(report_entered);
        dispatch_semaphore_wait(report_allow, DISPATCH_TIME_FOREVER);
    }
    int n = atomic_fetch_add(&line_slots, 1);
    va_list ap; va_start(ap, fmt);
    if (n < 8) vsnprintf(lines[n], sizeof lines[n], fmt, ap);
    va_end(ap);
    atomic_fetch_add(&reports, 1); dispatch_semaphore_signal(report_done);
    return 0;
}
#define flockfile checked_flockfile
#define funlockfile checked_funlockfile
#define fputc checked_fputc
#define fprintf checked_fprintf
#include "../hle_audio.c"
#undef fprintf
#undef fputc
#undef funlockfile
#undef flockfile
__thread _Atomic int dm_host_work;
int dm_guest_as_contains(uintptr_t a) { (void)a; return 0; } // No guest address space: the real big-stack entry keeps the calling host stack.
uint64_t now_ns(void) { return 0; }
static void *key, *value;
void *objmap_get(const void *k) { return k == key ? value : NULL; }
void objmap_put(const void *k, void *v) { key = (void *)k; value = v; }
void *objmap_del(const void *k) { void *v = objmap_get(k); key = value = NULL; return v; }
static _Atomic int signals;
void hle_signal_system_event(void *g) { (void)g; atomic_fetch_add(&signals, 1); dispatch_semaphore_signal(released_event); }
void hle_bind_system_event(void *g, int c) { (void)g; (void)c; }
module_t *module_of(const void *p) { (void)p; return NULL; }
void dm_host_helper_qos(qos_class_t q) { (void)q; }
static int checks, failures;
#define CHECK(v, why) do { checks++; if (!(v)) { failures++; printf("FAIL: %s\n", why); } } while (0)
static int wait_for(dispatch_semaphore_t s) { return !dispatch_semaphore_wait(s, dispatch_time(DISPATCH_TIME_NOW, 2000000000)); }
static int render(void *out, int count, int with_pcm) {
    aout *a = ao(out);
    for (int i = 0; i < count; i++) {
        int frames = 1 + (i & 1); int16_t input[] = {123, -456, 789, -1234}, played[4] = {0};
        gbuf b = {input, sizeof input, (uint64_t)frames * 4};
        if (with_pcm && !h_au_Append(out, &b)) return 0;
        AudioBufferList bl = {.mNumberBuffers = 1, .mBuffers = {{2, frames * 4, played}}};
        in_callback = 1; au_render(a, NULL, NULL, 0, frames, &bl); in_callback = 0;
        if (with_pcm) {
            int woke = !a->event || wait_for(released_event);
            int same = !memcmp(input, played, (size_t)frames * 4);
            gbuf *returned = woke && same ? h_au_Released(out) : NULL;
            if (!woke || !same || returned != &b) {
                printf("release handoff %d/%d: event=%d PCM=%d buffer=%d pending=%d armed=%d\n",
                       i + 1, count, woke, same, returned == &b, atomic_load(&a->notify_pending), atomic_load(&a->notify_armed));
                return 0;
            }
        }
    }
    return 1;
}
static void *drain_stats(void *unused) { (void)unused; stats_drain(); return NULL; }
static int16_t stream_sample(uint64_t frame, int ch_idx) {
    int32_t base = (int32_t)(1 + (frame % 30000));
    return (int16_t)(ch_idx & 1 ? -base : base);
}
static void fill_guest_block(gbuf *b, uint64_t start_frame, unsigned frames, int ch) {
    int16_t *pcm = b->ptr;
    for (unsigned i = 0; i < frames; i++) {
        for (int c = 0; c < ch; c++) {
            pcm[(size_t)i * (size_t)ch + (size_t)c] = (c < 2) ? stream_sample(start_frame + i, c) : 0;
        }
    }
    b->size = (uint64_t)frames * (uint64_t)ch * sizeof(int16_t);
}
// Reproduces iPhone 17 Pro Max DM_AUDIO_MIX=1 (240-1024 frame callbacks against 4 x 240-frame guest/audren blocks),
// forced 2048-frame callbacks, and 6-channel 2 x 256-frame Bluetooth/mix callbacks.
static void replay_callback_regime(const char *label, int ch, unsigned nbufs, unsigned buf_frames, int mode, int mix_env) {
    if (mix_env) setenv("DM_AUDIO_MIX", "1", 1); else unsetenv("DM_AUDIO_MIX");
    uint32_t out[16] = {0};
    uint8_t prm[8] = {0};
    *(uint16_t *)(prm + 4) = (uint16_t)ch;
    CHECK(!h_au_OpenDef0(out, prm), label);
    aout *a = ao(out);
    atomic_store(&a->state, STATE_STARTED);

    int16_t pcm_storage[4][512 * 6];
    gbuf bufs[4];
    uint64_t submitted = 0;
    for (unsigned i = 0; i < nbufs; i++) {
        bufs[i] = (gbuf){.ptr = pcm_storage[i], .cap = sizeof pcm_storage[i], .size = 0};
        fill_guest_block(&bufs[i], submitted, buf_frames, ch);
        submitted += buf_frames;
        CHECK(h_au_Append(out, &bufs[i]) == 1, "initial guest buffer append");
    }
    // Drain any immediate early-released slots if DM_AUDIO_MIX pre-opened the target window
    gbuf *rel;
    while ((rel = h_au_Released(out)) != NULL) {
        fill_guest_block(rel, submitted, buf_frames, ch);
        submitted += buf_frames;
        if (!h_au_Append(out, rel)) break;
    }

    uint64_t consumed = 0, mismatches = 0;
    int started_stream = 0;
    unsigned total_cbs = mode == 3 ? 1000 : 2000;
    int16_t played[2048 * 2];
    for (unsigned i = 0; i < total_cbs; i++) {
        unsigned frames = 240;
        if (mode == 1) frames = 1024;
        else if (mode == 2) frames = (i % 5 == 0) ? 240 : ((i % 3 == 0) ? 512 : 1024); // mixed 240-1024 frames as on iOS
        else if (mode == 3) frames = 2048;
        else if (mode == 4) frames = (i + 1) * 40960 / 147 - i * 40960 / 147; // 278-279 frames (44.1->48 kHz Bluetooth)

        memset(played, 0, (size_t)frames * 4);
        AudioBufferList bl = {.mNumberBuffers = 1, .mBuffers = {{2, frames * 4, played}}};
        in_callback = 1;
        au_render(a, NULL, NULL, 0, frames, &bl);
        in_callback = 0;
        atomic_store(&a->st_pending, 0);

        unsigned j = 0;
        if (!started_stream) {
            while (j < frames && played[2 * j] == 0 && played[2 * j + 1] == 0) j++;
            if (j < frames) started_stream = 1;
        }
        for (; j < frames; j++) {
            int16_t exp_l = stream_sample(consumed, 0);
            int16_t exp_r = stream_sample(consumed, 1);
            if (played[2 * j] != exp_l || played[2 * j + 1] != exp_r) mismatches++;
            consumed++;
        }

        while ((rel = h_au_Released(out)) != NULL) {
            fill_guest_block(rel, submitted, buf_frames, ch);
            submitted += buf_frames;
            if (!h_au_Append(out, rel)) { CHECK(0, "requeue returned guest buffer"); break; }
        }
    }
    uint64_t underruns = atomic_load(&a->under_n);
    printf("%s: %u callbacks, %llu underruns, %llu mismatches, %llu frames verified\n",
           label, total_cbs, (unsigned long long)underruns, (unsigned long long)mismatches, (unsigned long long)consumed);
    CHECK(started_stream && consumed > 0, "stream produced audio samples");
    CHECK(underruns == 0, "large/mixed callbacks run with zero underruns");
    CHECK(mismatches == 0, "all played PCM samples are contiguous and in order");

    // Stop must return every guest buffer exactly once so guest middleware can join cleanly.
    h_au_Stop(out);
    unsigned returned_after_stop = 0;
    while (h_au_Released(out) != NULL) returned_after_stop++;
    CHECK(returned_after_stop == nbufs, "stop returns all in-flight guest buffers");

    // Exclusive 240-frame mode must keep zero extra prebuffer latency when DM_AUDIO_MIX is off.
    if (mode == 0) {
        CHECK(consumed == (uint64_t)total_cbs * 240, "240-frame exclusive mode plays from sample 0 with no prebuffer hold");
    }
    h_au_Close(out);
    unsetenv("DM_AUDIO_MIX");
}
int main(void) {
    setenv("DM_AUDIO", "0", 1); setenv("DM_AUDIO_MUTE", "0", 1);
    replay_callback_regime("exclusive-240", 2, 4, 240, 0, 0);
    replay_callback_regime("ios-mix-1024", 2, 4, 240, 1, 1);
    replay_callback_regime("ios-mix-variable-240-1024", 2, 4, 240, 2, 1);
    replay_callback_regime("unhinted-2048", 2, 4, 240, 3, 0);
    replay_callback_regime("bt-6ch-2x256", 6, 2, 256, 4, 0);
    report_entered = dispatch_semaphore_create(0); report_allow = dispatch_semaphore_create(0);
    report_done = dispatch_semaphore_create(0); released_event = dispatch_semaphore_create(0);
    uint32_t out[16] = {0}; uint8_t prm[8] = {0}, ev[64] = {0};
    CHECK(!h_au_OpenDef(out, ev, prm), "open output with a guest release event");
    atomic_store(&ao(out)->state, STATE_STARTED); atomic_store(&block_report, 1);
    CHECK(render(out, 2000, 1), "PCM and release order at first statistics boundary");
    CHECK(!atomic_load(&callback_io), "callback never enters stdio or its stream lock");
    if (atomic_load(&callback_io)) { h_au_Close(out); return 1; } // old code fails here, without waiting for a helper that never logs
    pthread_t logger; CHECK(!pthread_create(&logger, NULL, drain_stats, NULL), "run the watchdog's actual statistics drain");
    CHECK(wait_for(report_entered), "hold the diagnostic writer outside the callback");
    CHECK(render(out, 6000, 1), "a blocked diagnostic writer never stops PCM or guest release events");
    CHECK(atomic_load(&signals) == 8000, "every event-driven PCM buffer is released while diagnostics are blocked");
    CHECK(!atomic_load(&callback_io), "backlogged diagnostics still use no callback stdio");
    dispatch_semaphore_signal(report_allow);
    pthread_join(logger, NULL); stats_drain();
    CHECK(wait_for(report_done) && wait_for(report_done), "helper drains two complete snapshots");
    CHECK(render(out, 2000, 1), "callbacks continue after the diagnostic backlog");
    stats_drain();
    CHECK(wait_for(report_done), "accumulated callback statistics are eventually reported");
    CHECK(strstr(lines[0], "device callbacks 2000: 1\xe2\x80\x93" "2 frames") && strstr(lines[0], "1.0 blocks queued") && strstr(lines[0], "in these 2000, 0 underruns, 0.0 ms"), "first report preserves frame range, queue depth and underrun fields");
    CHECK(strstr(lines[1], "device callbacks 4000:") && strstr(lines[1], "in these 2000,"), "next pending snapshot is preserved");
    CHECK(strstr(lines[2], "device callbacks 10000:") && strstr(lines[2], "in these 6000,"), "a full mailbox merges later windows without losing their counts");
    h_au_Close(out);

    // The watchdog's local report may outlive the output; Close owns any newer pending snapshot.
    atomic_store(&signals, 0);
    CHECK(!h_au_OpenDef(out, ev, prm), "open another output with a guest event");
    atomic_store(&ao(out)->state, STATE_STARTED); atomic_store(&block_report, 1);
    CHECK(render(out, 2000, 0), "publish a close-case report");
    CHECK(!pthread_create(&logger, NULL, drain_stats, NULL) && wait_for(report_entered), "hold a copied watchdog report");
    CHECK(render(out, 2000, 0), "publish a second report before close");
    h_au_Close(out);
    CHECK(atomic_load(&reports) == 4, "close drains the pending snapshot while the watchdog retains only a local copy");
    dispatch_semaphore_signal(report_allow); pthread_join(logger, NULL);
    CHECK(atomic_load(&reports) == 5, "the watchdog logs its copied report after the output was freed");
    CHECK(!atomic_load(&signals), "statistics wakes never signal a guest buffer-release event");
    CHECK(!atomic_load(&callback_io), "all callback paths remain free of diagnostic stdio");

    // One buffer per event repeatedly crosses the notifier's disarm/recheck
    // boundary. Bulk PCM stress can coalesce releases and miss a stranded wake.
    atomic_store(&signals, 0);
    CHECK(!h_au_OpenDef(out, ev, prm), "open output for release-handoff stress");
    atomic_store(&ao(out)->state, STATE_STARTED);
    CHECK(render(out, 100000, 1), "every individual release handoff makes progress");
    CHECK(atomic_load(&signals) == 100000, "all 100000 event-driven buffers are released");
    CHECK(!atomic_load(&callback_io), "release-handoff stress keeps diagnostics off the callback");
    h_au_Close(out);
    dispatch_release(report_entered); dispatch_release(report_allow); dispatch_release(report_done); dispatch_release(released_event);
    printf("audio callback statistics: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
