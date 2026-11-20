#ifndef NCA_INTERACTIVE_H
#define NCA_INTERACTIVE_H

#include <stdbool.h>
#include <stdint.h>

#include <nca/nca_protocol.h>
#include <nca/nca_topics.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NCA_MODE_RAW         0x00u
#define NCA_MODE_INTERACTIVE 0x01u
#define NCA_INTERACTIVE_STATE_CHANGED 0xFFu

typedef struct {
    uint8_t mode;
    uint8_t assigned_topic;
    float range_minimum;
    float range_maximum;
    uint16_t rpm;
    uint16_t speed_deci;
    uint32_t odometer_deci_km;
    float oil_temperature;
    float outdoor_temperature;
    float coolant_temperature;
    float fuel_level;
    uint32_t indicators[2];
    float generic[NCA_TOPIC_GENERIC_COUNT];
    uint16_t last_sequence;
    uint8_t last_topic;
} NcaInteractiveState;

void nca_interactive_init(NcaInteractiveState *state, uint8_t assigned_topic,
                          float range_minimum, float range_maximum);
uint8_t nca_interactive_process(NcaInteractiveState *state, uint8_t command,
                                const uint8_t *payload, uint16_t length);
float nca_interactive_assigned_value(const NcaInteractiveState *state);
bool nca_interactive_indicator_is_set(const NcaInteractiveState *state,
                                      uint8_t bit_number);

#ifdef __cplusplus
}
#endif

#endif
