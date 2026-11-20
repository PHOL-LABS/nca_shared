#include <nca/nca_topics.h>

#include <string.h>

typedef char nca_float_must_be_32_bits[(sizeof(float) == 4u) ? 1 : -1];

static void write_u16_le(uint8_t *output, uint16_t value)
{
    output[0] = (uint8_t)value;
    output[1] = (uint8_t)(value >> 8);
}

static void write_u32_le(uint8_t *output, uint32_t value)
{
    output[0] = (uint8_t)value;
    output[1] = (uint8_t)(value >> 8);
    output[2] = (uint8_t)(value >> 16);
    output[3] = (uint8_t)(value >> 24);
}

static void write_f32_le(uint8_t *output, float value)
{
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    write_u32_le(output, bits);
}

bool nca_topic_is_generic(uint8_t topic)
{
    return topic >= NCA_TOPIC_GENERIC_BASE &&
           topic < NCA_TOPIC_GENERIC_BASE + NCA_TOPIC_GENERIC_COUNT;
}

static size_t encode_scalar(uint8_t *output, size_t capacity,
                            uint8_t topic, uint16_t sequence, uint16_t value)
{
    if (output == NULL || capacity < NCA_PUBLISH_SCALAR_PAYLOAD_SIZE) return 0u;
    output[0] = NCA_INTERACTIVE_VERSION;
    output[1] = topic;
    write_u16_le(output + 2u, sequence);
    write_u16_le(output + 4u, value);
    return NCA_PUBLISH_SCALAR_PAYLOAD_SIZE;
}

size_t nca_publish_encode_u16(uint8_t *output, size_t capacity,
                              uint8_t topic, uint16_t sequence, uint16_t value)
{
    if (topic != NCA_TOPIC_RPM && topic != NCA_TOPIC_SPEED) return 0u;
    return encode_scalar(output, capacity, topic, sequence, value);
}

size_t nca_publish_encode_u32(uint8_t *output, size_t capacity,
                              uint8_t topic, uint16_t sequence, uint32_t value)
{
    if (output == NULL || capacity < NCA_PUBLISH_U32_PAYLOAD_SIZE ||
        topic != NCA_TOPIC_ODOMETER) return 0u;
    output[0] = NCA_INTERACTIVE_VERSION;
    output[1] = topic;
    write_u16_le(output + 2u, sequence);
    write_u32_le(output + 4u, value);
    return NCA_PUBLISH_U32_PAYLOAD_SIZE;
}

size_t nca_publish_encode_deci(uint8_t *output, size_t capacity,
                               uint8_t topic, uint16_t sequence, int16_t value_deci)
{
    if (topic != NCA_TOPIC_OIL_TEMPERATURE &&
        topic != NCA_TOPIC_OUTDOOR_TEMPERATURE &&
        topic != NCA_TOPIC_COOLANT_TEMPERATURE &&
        topic != NCA_TOPIC_FUEL_LEVEL && !nca_topic_is_generic(topic)) return 0u;
    return encode_scalar(output, capacity, topic, sequence, (uint16_t)value_deci);
}

size_t nca_publish_encode_indicators(uint8_t *output, size_t capacity,
                                     uint16_t sequence,
                                     uint32_t bits_low, uint32_t bits_high)
{
    if (output == NULL || capacity < NCA_PUBLISH_INDICATOR_PAYLOAD_SIZE) return 0u;
    output[0] = NCA_INTERACTIVE_VERSION;
    output[1] = NCA_TOPIC_INDICATORS;
    write_u16_le(output + 2u, sequence);
    write_u32_le(output + 4u, bits_low);
    write_u32_le(output + 8u, bits_high);
    return NCA_PUBLISH_INDICATOR_PAYLOAD_SIZE;
}

size_t nca_config_encode_range(uint8_t *output, size_t capacity,
                               uint8_t topic, float minimum, float maximum)
{
    if (output == NULL || capacity < 14u || !(minimum < maximum)) return 0u;
    output[0] = NCA_INTERACTIVE_VERSION;
    output[1] = topic;
    output[2] = NCA_CONFIG_MINIMUM_F32;
    output[3] = 4u;
    write_f32_le(output + 4u, minimum);
    output[8] = NCA_CONFIG_MAXIMUM_F32;
    output[9] = 4u;
    write_f32_le(output + 10u, maximum);
    return 14u;
}
