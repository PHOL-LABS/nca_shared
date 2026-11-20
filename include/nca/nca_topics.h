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
#define NCA_TOPIC_ODOMETER            0x08u
#define NCA_TOPIC_GENERIC_BASE        0x20u
#define NCA_TOPIC_GENERIC_COUNT       10u

/* Project-wide meanings for the ten deci-unit generic telemetry channels. */
#define NCA_TOPIC_MFA_VALUE            (NCA_TOPIC_GENERIC_BASE + 0u)
#define NCA_TOPIC_MFA_CLOCK_VALUE      (NCA_TOPIC_GENERIC_BASE + 1u)
#define NCA_TOPIC_FUEL_CONSUMPTION     (NCA_TOPIC_GENERIC_BASE + 2u)
#define NCA_TOPIC_INTAKE_VOLTAGE       (NCA_TOPIC_GENERIC_BASE + 3u)
#define NCA_TOPIC_FUEL_PRESSURE        (NCA_TOPIC_GENERIC_BASE + 4u)
#define NCA_TOPIC_UPTIME_HOURS         (NCA_TOPIC_GENERIC_BASE + 5u)
#define NCA_TOPIC_SPEED_RAW             (NCA_TOPIC_GENERIC_BASE + 6u)
#define NCA_TOPIC_BRIGHTNESS_PERCENT   (NCA_TOPIC_GENERIC_BASE + 7u)
#define NCA_TOPIC_GEAR                 (NCA_TOPIC_GENERIC_BASE + 8u)
#define NCA_TOPIC_MFA_PAGE             (NCA_TOPIC_GENERIC_BASE + 9u)

#define NCA_INDICATOR_LIGHTS_ON_BIT          0u
#define NCA_INDICATOR_FRONT_FOG_BIT          1u
#define NCA_INDICATOR_REAR_FOG_BIT           2u
#define NCA_INDICATOR_REAR_WINDOW_HEAT_BIT   3u
#define NCA_INDICATOR_HIGH_BEAM_BIT           4u
#define NCA_INDICATOR_LEFT_TURN_BIT           5u
#define NCA_INDICATOR_RIGHT_TURN_BIT          6u
#define NCA_INDICATOR_BRAKE_BIT               7u
#define NCA_INDICATOR_OIL_PRESSURE_BIT        8u
#define NCA_INDICATOR_BATTERY_BIT             9u
#define NCA_INDICATOR_MFA1_BIT               10u
#define NCA_INDICATOR_MFA2_BIT               11u
#define NCA_INDICATOR_CHECK_ENGINE_BIT       12u
#define NCA_INDICATOR_LOW_FUEL_BIT           13u
#define NCA_INDICATOR_BACKLIGHT_BIT          14u

#define NCA_INDICATOR_OIL_TEMPERATURE_HIGH_BIT     24u
#define NCA_INDICATOR_OUTDOOR_TEMPERATURE_HIGH_BIT 25u
#define NCA_INDICATOR_COOLANT_TEMPERATURE_HIGH_BIT 26u
#define NCA_INDICATOR_FUEL_LOW_BIT                 27u

#define NCA_CONFIG_MINIMUM_F32 0x01u
#define NCA_CONFIG_MAXIMUM_F32 0x02u

#define NCA_PUBLISH_SCALAR_PAYLOAD_SIZE    6u
#define NCA_PUBLISH_U32_PAYLOAD_SIZE       8u
#define NCA_PUBLISH_INDICATOR_PAYLOAD_SIZE 12u

bool nca_topic_is_generic(uint8_t topic);
size_t nca_publish_encode_u16(uint8_t *output, size_t capacity,
                              uint8_t topic, uint16_t sequence, uint16_t value);
size_t nca_publish_encode_u32(uint8_t *output, size_t capacity,
                              uint8_t topic, uint16_t sequence, uint32_t value);
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
