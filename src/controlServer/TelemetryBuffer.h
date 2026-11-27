#pragma once
#include <cstddef>
#include <cstdint>

#pragma pack(push, 1)
struct PIDSample {
    uint32_t sequence;
    int16_t target_speed1;
    int16_t measured_speed1;
    int16_t target_speed2;
    int16_t measured_speed2;
};
#pragma pack(pop)

struct PIDBuffer {
    static constexpr size_t capacity = 128;
    PIDSample samples[capacity];
    size_t next = 0;   // Slot for the next write
    size_t count = 0;  // Number of valid samples

    void push(const PIDSample& sample) {
        samples[next] = sample;
        next = (next + 1) % capacity;  // Wrap to zero
        if (count < capacity) ++count;
    }

    size_t drain(PIDSample* destination, size_t max_count) {
        const size_t drained = count < max_count ? count : max_count;
        size_t oldest = (next + capacity - count) % capacity;
        for (size_t i = 0; i < drained; ++i) {
            destination[i] = samples[oldest];
            oldest = (oldest + 1) % capacity;
        }
        count -= drained;
        return drained;
    }

    // TODO: Track overwritten samples so telemetry clients can detect gaps.
};

static_assert(sizeof(PIDSample) == 12, "Sample size mismatch");

