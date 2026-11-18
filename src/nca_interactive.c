#include <nca/nca_interactive.h>

#include <string.h>

static uint16_t read_u16_le(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static uint32_t read_u32_le(const uint8_t *data)
{
    return (uint32_t)data[0] | ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
}

static float read_f32_le(const uint8_t *data)
{
    uint32_t bits = read_u32_le(data);
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

void nca_interactive_init(NcaInteractiveState *state, uint8_t assigned_topic,
                          float range_minimum, float range_maximum)
{
    if (state == NULL) return;
    memset(state, 0, sizeof(*state));
    state->mode = NCA_MODE_RAW;
    state->assigned_topic = assigned_topic;
    state->range_minimum = range_minimum;
    state->range_maximum = range_maximum;
}

static uint8_t process_publish(NcaInteractiveState *state,
                               const uint8_t *payload, uint16_t length)
{
    uint8_t topic;
    const uint8_t *data;
    uint16_t data_length;
    int16_t signed_deci;
    if (length < 4u || payload[0] != NCA_INTERACTIVE_VERSION) return NCA_TOPIC_NONE;
    topic = payload[1];
    data = payload + 4u;
    data_length = length - 4u;

    switch (topic) {
    case NCA_TOPIC_RPM:
        if (data_length != 2u) return NCA_TOPIC_NONE;
        state->rpm = read_u16_le(data);
        break;
    case NCA_TOPIC_SPEED:
        if (data_length != 2u) return NCA_TOPIC_NONE;
        state->speed_deci = read_u16_le(data);
        break;
    case NCA_TOPIC_OIL_TEMPERATURE:
    case NCA_TOPIC_OUTDOOR_TEMPERATURE:
    case NCA_TOPIC_COOLANT_TEMPERATURE:
    case NCA_TOPIC_FUEL_LEVEL:
        if (data_length != 2u) return NCA_TOPIC_NONE;
        signed_deci = (int16_t)read_u16_le(data);
        if (topic == NCA_TOPIC_OIL_TEMPERATURE) state->oil_temperature = (float)signed_deci / 10.0f;
        else if (topic == NCA_TOPIC_OUTDOOR_TEMPERATURE) state->outdoor_temperature = (float)signed_deci / 10.0f;
        else if (topic == NCA_TOPIC_COOLANT_TEMPERATURE) state->coolant_temperature = (float)signed_deci / 10.0f;
        else state->fuel_level = (float)signed_deci / 10.0f;
        break;
    case NCA_TOPIC_INDICATORS:
        if (data_length != 8u) return NCA_TOPIC_NONE;
        state->indicators[0] = read_u32_le(data);
        state->indicators[1] = read_u32_le(data + 4u);
        break;
    default:
        if (!nca_topic_is_generic(topic) || data_length != 2u) return NCA_TOPIC_NONE;
        signed_deci = (int16_t)read_u16_le(data);
        state->generic[topic - NCA_TOPIC_GENERIC_BASE] = (float)signed_deci / 10.0f;
        break;
    }
    state->last_sequence = read_u16_le(payload + 2u);
    state->last_topic = topic;
    return topic;
}

static uint8_t process_config(NcaInteractiveState *state,
                              const uint8_t *payload, uint16_t length)
{
    uint16_t position = 2u;
    float minimum = state->range_minimum;
    float maximum = state->range_maximum;
    uint8_t topic;
    if (length < 2u || payload[0] != NCA_INTERACTIVE_VERSION) return NCA_TOPIC_NONE;
    topic = payload[1];
    if (topic != NCA_TOPIC_NONE && topic != state->assigned_topic) return NCA_TOPIC_NONE;
    while (position < length) {
        uint8_t key;
        uint8_t value_length;
        if ((uint16_t)(length - position) < 2u) return NCA_TOPIC_NONE;
        key = payload[position++];
        value_length = payload[position++];
        if (value_length > (uint16_t)(length - position)) return NCA_TOPIC_NONE;
        if (key == NCA_CONFIG_MINIMUM_F32 && value_length == 4u) minimum = read_f32_le(payload + position);
        else if (key == NCA_CONFIG_MAXIMUM_F32 && value_length == 4u) maximum = read_f32_le(payload + position);
        position += value_length;
    }
    if (!(minimum < maximum)) return NCA_TOPIC_NONE;
    state->range_minimum = minimum;
    state->range_maximum = maximum;
    return NCA_INTERACTIVE_STATE_CHANGED;
}

uint8_t nca_interactive_process(NcaInteractiveState *state, uint8_t command,
                                const uint8_t *payload, uint16_t length)
{
    if (state == NULL || (payload == NULL && length != 0u)) return NCA_TOPIC_NONE;
    if (command == NCA_CMD_PUBLISH) return process_publish(state, payload, length);
    if (command == NCA_CMD_SET_MODE) {
        if (length != 1u || payload[0] > NCA_MODE_INTERACTIVE) return NCA_TOPIC_NONE;
        state->mode = payload[0];
        return NCA_INTERACTIVE_STATE_CHANGED;
    }
    if (command == NCA_CMD_SET_CONFIG) return process_config(state, payload, length);
    return NCA_TOPIC_NONE;
}

float nca_interactive_assigned_value(const NcaInteractiveState *state)
{
    if (state == NULL) return 0.0f;
    switch (state->assigned_topic) {
    case NCA_TOPIC_RPM: return (float)state->rpm;
    case NCA_TOPIC_SPEED: return (float)state->speed_deci / 10.0f;
    case NCA_TOPIC_OIL_TEMPERATURE: return state->oil_temperature;
    case NCA_TOPIC_OUTDOOR_TEMPERATURE: return state->outdoor_temperature;
    case NCA_TOPIC_COOLANT_TEMPERATURE: return state->coolant_temperature;
    case NCA_TOPIC_FUEL_LEVEL: return state->fuel_level;
    default:
        if (nca_topic_is_generic(state->assigned_topic))
            return state->generic[state->assigned_topic - NCA_TOPIC_GENERIC_BASE];
        return 0.0f;
    }
}

bool nca_interactive_indicator_is_set(const NcaInteractiveState *state,
                                      uint8_t bit_number)
{
    uint8_t word = bit_number >> 5;
    uint8_t bit = bit_number & 31u;
    return state != NULL && word < 2u &&
           (state->indicators[word] & (1u << bit)) != 0u;
}
