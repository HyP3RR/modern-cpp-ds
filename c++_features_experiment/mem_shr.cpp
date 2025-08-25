#include <pthread.h>
#include <sched.h>
#include <atomic>
#include <cstdint>
#include <iostream>


alignas(128) static uint64_t data[SIZE];
alignas(128) static std::atomic<unsigned> shared;
#ifdef EMPTY_PRODUCER
alignas(128) std::atomic<unsigned> unshared;
#endif
alignas(128) static std::atomic<bool> stop_producer;
alignas(128) static std::atomic<uint64_t> elapsed;

static inline uint64_t rdtsc()
{
    unsigned int l, h;
    __asm__ __volatile__ (
        "rdtsc"
        : "=a" (l), "=d" (h)
    );
    return ((uint64_t)h << 32) | l;
}

static void * consume(void *)
{
    uint64_t    value = 0;
    uint64_t    start = rdtsc();

    for (unsigned n = 0; n < LOOPS; ++n) {
        for (unsigned idx = 0; idx < SIZE; ++idx) {
            value += data[idx] + shared.load(std::memory_order_relaxed);
        }
    }

    elapsed = rdtsc() - start;
    return reinterpret_cast<void*>(value);
}

static void * produce(void *)
{
    do {
#ifdef EMPTY_PRODUCER
        unshared.store(0, std::memory_order_relaxed);
#else
        shared.store(0, std::memory_order_relaxed);
#endif
    } while (!stop_producer);
    return nullptr;
}



int main()
{
    pthread_t consumerId, producerId;
    pthread_attr_t consumerAttrs, producerAttrs;
    cpu_set_t cpuset;

    for (unsigned idx = 0; idx < SIZE; ++idx) { data[idx] = 1; }
    shared = 0;
    stop_producer = false;

    pthread_attr_init(&consumerAttrs);
    CPU_ZERO(&cpuset);
    CPU_SET(CONSUMER_CPU, &cpuset);
    pthread_attr_setaffinity_np(&consumerAttrs, sizeof(cpuset), &cpuset);

    pthread_attr_init(&producerAttrs);
    CPU_ZERO(&cpuset);
    CPU_SET(PRODUCER_CPU, &cpuset);
    pthread_attr_setaffinity_np(&producerAttrs, sizeof(cpuset), &cpuset);

    pthread_create(&consumerId, &consumerAttrs, consume, NULL);
    pthread_create(&producerId, &producerAttrs, produce, NULL);

    pthread_attr_destroy(&consumerAttrs);
    pthread_attr_destroy(&producerAttrs);

    pthread_join(consumerId, NULL);
    stop_producer = true;
    pthread_join(producerId, NULL);

    std::cout <<"Elapsed cycles: " <<elapsed <<std::endl;
    return 0;
}
