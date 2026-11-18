#ifndef NCA_TOPICS_H
#define NCA_TOPICS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NCA_INTERACTIVE_VERSION 0x01u

#define NCA_TOPIC_NONE                0x00u
#define NCA_TOPIC_RPM                 0x01u
#define NCA_TOPIC_SPEED               0x02u
#define NCA_TOPIC_OIL_TEMPERATURE     0x03u
#define NCA_TOPIC_OUTDOOR_TEMPERATURE 0x04u
#define NCA_TOPIC_COOLANT_TEMPERATURE 0x05u
#define NCA_TOPIC_FUEL_LEVEL          0x06u
#define NCA_TOPIC_INDICATORS          0x07u
#define NCA_TOPIC_GENERIC_BASE        0x20u
#define NCA_TOPIC_GENERIC_COUNT       10u

#define NCA_INDICATOR_OIL_TEMPERATURE_HIGH_BIT     24u
#define NCA_INDICATOR_OUTDOOR_TEMPERATURE_HIGH_BIT 25u
#define NCA_INDICATOR_COOLANT_TEMPERATURE_HIGH_BIT 26u
#define NCA_INDICATOR_FUEL_LOW_BIT                 27u

#define NCA_CONFIG_MINIMUM_F32 0x01u
#define NCA_CONFIG_MAXIMUM_F32 0x02u

#define NCA_PUBLISH_SCALAR_PAYLOAD_SIZE    6u
#define NCA_PUBLISH_INDICATOR_PAYLOAD_SIZE 12u

bool nca_topic_is_generic(uint8_t topic);
size_t nca_publish_encode_u16(uint8_t *output, size_t capacity,
                              uint8_t topic, uint16_t sequence, uint16_t value);
size_t nca_publish_encode_deci(uint8_t *output, size_t capacity,
                               uint8_t topic, uint16_t sequence, int16_t value_deci);
size_t nca_publish_encode_indicators(uint8_t *output, size_t capacity,
                                     uint16_t sequence,
                                     uint32_t bits_low, uint32_t bits_high);
size_t nca_config_encode_range(uint8_t *output, size_t capacity,
                               uint8_t topic, float minimum, float maximum);

#ifdef __cplusplus
}
#endif

#endif
