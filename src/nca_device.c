#include <nca/nca_device.h>

#include <string.h>

static uint16_t read_u16_le(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

static NcaProcessResult result_with(uint32_t flags)
{
    NcaProcessResult result;
    result.flags = flags;
    result.topic = NCA_TOPIC_NONE;
    result.address = 0u;
    return result;
}

void nca_device_init(NcaDeviceState *state,
                     uint8_t *segments, NcaRgb *colors, uint16_t segment_count,
                     bool default_segment_on, const NcaRgb *default_colors,
                     uint8_t assigned_topic,
                     float range_minimum, float range_maximum)
{
    uint16_t i;
    if (state == NULL) return;
    memset(state, 0, sizeof(*state));
    state->segments = segments;
    state->colors = colors;
    state->segment_count = segment_count;
    nca_interactive_init(&state->interactive, assigned_topic,
                         range_minimum, range_maximum);
    if (segment_count != 0u && (segments == NULL || colors == NULL)) {
        state->segment_count = 0u;
        return;
    }
    for (i = 0u; i < segment_count; ++i) {
        segments[i] = default_segment_on ? 1u : 0u;
        if (default_colors != NULL) colors[i] = default_colors[i];
        else memset(&colors[i], 0, sizeof(colors[i]));
    }
}

NcaProcessResult nca_device_process(NcaDeviceState *state,
                                    uint8_t command,
                                    const uint8_t *payload, uint16_t length,
                                    bool destination_is_broadcast)
{
    NcaProcessResult result = result_with(NCA_PROCESS_NONE);
    uint16_t offset;
    uint16_t count;
    uint16_t i;
    uint8_t interactive_change;
    if (state == NULL || (payload == NULL && length != 0u)) return result;

    if (command == NCA_CMD_PUBLISH || command == NCA_CMD_SET_MODE ||
        command == NCA_CMD_SET_CONFIG) {
        result.flags = NCA_PROCESS_HANDLED;
        interactive_change = nca_interactive_process(&state->interactive, command,
                                                      payload, length);
        if (interactive_change == NCA_INTERACTIVE_STATE_CHANGED) {
            result.flags |= command == NCA_CMD_SET_MODE
                ? NCA_PROCESS_MODE_CHANGED : NCA_PROCESS_CONFIG_CHANGED;
        } else if (interactive_change != NCA_TOPIC_NONE) {
            result.flags |= NCA_PROCESS_TOPIC_CHANGED;
            result.topic = interactive_change;
        }
        return result;
    }

    switch (command) {
    case NCA_CMD_SET_ALL:
        result.flags = NCA_PROCESS_HANDLED;
        if (length > state->segment_count) return result;
        for (i = 0u; i < length; ++i) state->segments[i] = payload[i] != 0u;
        result.flags |= NCA_PROCESS_RAW_CHANGED;
        break;
    case NCA_CMD_SET_RANGE:
        result.flags = NCA_PROCESS_HANDLED;
        if (length < 4u) return result;
        offset = read_u16_le(payload);
        count = read_u16_le(payload + 2u);
        if (count != (uint16_t)(length - 4u) || offset > state->segment_count ||
            count > (uint16_t)(state->segment_count - offset)) return result;
        for (i = 0u; i < count; ++i) state->segments[offset + i] = payload[4u + i] != 0u;
        result.flags |= NCA_PROCESS_RAW_CHANGED;
        break;
    case NCA_CMD_SET_PACKED_RANGE:
        result.flags = NCA_PROCESS_HANDLED;
        if (length < 4u) return result;
        offset = read_u16_le(payload);
        count = read_u16_le(payload + 2u);
        if (length != (uint16_t)(4u + ((count + 7u) / 8u)) ||
            offset > state->segment_count ||
            count > (uint16_t)(state->segment_count - offset)) return result;
        for (i = 0u; i < count; ++i) {
            state->segments[offset + i] =
                (payload[4u + (i >> 3)] & (uint8_t)(1u << (i & 7u))) != 0u;
        }
        result.flags |= NCA_PROCESS_RAW_CHANGED;
        break;
    case NCA_CMD_SET_RGB_ENABLED:
        result.flags = NCA_PROCESS_HANDLED;
        if (length != 1u) return result;
        state->custom_colors_enabled = payload[0] != 0u;
        result.flags |= NCA_PROCESS_RGB_CHANGED;
        break;
    case NCA_CMD_SET_RGB_ALL:
        result.flags = NCA_PROCESS_HANDLED;
        if (state->segment_count > UINT16_MAX / 3u ||
            length != (uint16_t)(state->segment_count * 3u)) return result;
        for (i = 0u; i < state->segment_count; ++i) {
            state->colors[i].r = payload[i * 3u];
            state->colors[i].g = payload[i * 3u + 1u];
            state->colors[i].b = payload[i * 3u + 2u];
        }
        result.flags |= NCA_PROCESS_RGB_CHANGED;
        break;
    case NCA_CMD_SET_RGB_RANGE:
        result.flags = NCA_PROCESS_HANDLED;
        if (length < 4u) return result;
        offset = read_u16_le(payload);
        count = read_u16_le(payload + 2u);
        if (offset > state->segment_count ||
            count > (uint16_t)(state->segment_count - offset) ||
            count > (UINT16_MAX - 4u) / 3u ||
            length != (uint16_t)(4u + count * 3u)) return result;
        for (i = 0u; i < count; ++i) {
            state->colors[offset + i].r = payload[4u + i * 3u];
            state->colors[offset + i].g = payload[5u + i * 3u];
            state->colors[offset + i].b = payload[6u + i * 3u];
        }
        result.flags |= NCA_PROCESS_RGB_CHANGED;
        break;
    case NCA_CMD_SET_ADDRESS:
        result.flags = NCA_PROCESS_HANDLED;
        if (length == 1u && !destination_is_broadcast &&
            nca_protocol_address_is_valid(payload[0])) {
            result.flags |= NCA_PROCESS_ADDRESS_CHANGED;
            result.address = payload[0];
        }
        break;
    default:
        break;
    }
    return result;
}
