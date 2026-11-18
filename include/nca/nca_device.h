#ifndef NCA_DEVICE_H
#define NCA_DEVICE_H

#include <stdbool.h>
#include <stdint.h>

#include <nca/nca_interactive.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} NcaRgb;

typedef struct {
    NcaInteractiveState interactive;
    uint8_t *segments;
    NcaRgb *colors;
    uint16_t segment_count;
    bool custom_colors_enabled;
} NcaDeviceState;

enum {
    NCA_PROCESS_NONE            = 0u,
    NCA_PROCESS_HANDLED         = 1u << 0,
    NCA_PROCESS_RAW_CHANGED     = 1u << 1,
    NCA_PROCESS_RGB_CHANGED     = 1u << 2,
    NCA_PROCESS_MODE_CHANGED    = 1u << 3,
    NCA_PROCESS_CONFIG_CHANGED  = 1u << 4,
    NCA_PROCESS_TOPIC_CHANGED   = 1u << 5,
    NCA_PROCESS_ADDRESS_CHANGED = 1u << 6
};

typedef struct {
    uint32_t flags;
    uint8_t topic;
    uint8_t address;
} NcaProcessResult;

void nca_device_init(NcaDeviceState *state,
                     uint8_t *segments, NcaRgb *colors, uint16_t segment_count,
                     bool default_segment_on, const NcaRgb *default_colors,
                     uint8_t assigned_topic,
                     float range_minimum, float range_maximum);
NcaProcessResult nca_device_process(NcaDeviceState *state,
                                    uint8_t command,
                                    const uint8_t *payload, uint16_t length,
                                    bool destination_is_broadcast);

#ifdef __cplusplus
}
#endif

#endif
