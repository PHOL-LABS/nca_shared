#include <nca/nca_protocol.h>

bool nca_protocol_address_is_valid(uint8_t address)
{
    return address != NCA_LISTEN_ALL_ADDRESS &&
           address != NCA_BROADCAST_ADDRESS && address != 0xFFu;
}

void nca_protocol_reset(NcaProtocol *protocol)
{
    if (protocol == NULL) return;
    protocol->state = NCA_WAIT_SOF1;
    protocol->destination = 0u;
    protocol->command = 0u;
    protocol->crc = 0u;
    protocol->length = 0u;
    protocol->position = 0u;
}

void nca_protocol_init(NcaProtocol *protocol, uint8_t device_address,
                       NcaFrameHandler handler, void *user)
{
    if (protocol == NULL) return;
    protocol->device_address = device_address;
    protocol->handler = handler;
    protocol->user = user;
    nca_protocol_reset(protocol);
}

void nca_protocol_set_address(NcaProtocol *protocol, uint8_t device_address)
{
    if (protocol != NULL && (nca_protocol_address_is_valid(device_address) ||
                             device_address == NCA_LISTEN_ALL_ADDRESS)) {
        protocol->device_address = device_address;
    }
}

uint8_t nca_protocol_get_address(const NcaProtocol *protocol)
{
    return protocol == NULL ? NCA_LISTEN_ALL_ADDRESS : protocol->device_address;
}

static bool destination_matches(const NcaProtocol *protocol)
{
    return protocol->device_address == NCA_LISTEN_ALL_ADDRESS ||
           protocol->destination == NCA_BROADCAST_ADDRESS ||
           protocol->destination == protocol->device_address;
}

static void nca_protocol_feed_byte(NcaProtocol *protocol, uint8_t byte)
{
    switch (protocol->state) {
    case NCA_WAIT_SOF1:
        if (byte == NCA_SOF1) protocol->state = NCA_WAIT_DESTINATION;
        break;
    case NCA_WAIT_DESTINATION:
        protocol->destination = byte;
        protocol->state = NCA_WAIT_COMMAND;
        break;
    case NCA_WAIT_COMMAND:
        protocol->command = byte;
        protocol->crc = byte;
        protocol->state = NCA_WAIT_LENGTH_LOW;
        break;
    case NCA_WAIT_LENGTH_LOW:
        protocol->length = byte;
        protocol->crc ^= byte;
        protocol->state = NCA_WAIT_LENGTH_HIGH;
        break;
    case NCA_WAIT_LENGTH_HIGH:
        protocol->length |= (uint16_t)byte << 8;
        protocol->crc ^= byte;
        protocol->position = 0u;
        if (protocol->length > NCA_PROTOCOL_MAX_PAYLOAD) {
            nca_protocol_reset(protocol);
        } else {
            protocol->state = protocol->length == 0u
                ? NCA_WAIT_CRC : NCA_WAIT_PAYLOAD;
        }
        break;
    case NCA_WAIT_PAYLOAD:
        protocol->payload[protocol->position++] = byte;
        protocol->crc ^= byte;
        if (protocol->position == protocol->length) protocol->state = NCA_WAIT_CRC;
        break;
    case NCA_WAIT_CRC:
        if (byte == protocol->crc && destination_matches(protocol) &&
            protocol->handler != NULL) {
            protocol->handler(protocol->user, protocol->command,
                              protocol->payload, protocol->length);
        }
        nca_protocol_reset(protocol);
        break;
    default:
        nca_protocol_reset(protocol);
        break;
    }
}

void nca_protocol_feed(NcaProtocol *protocol, const uint8_t *data, size_t length)
{
    size_t i;
    if (protocol == NULL || (data == NULL && length != 0u)) return;
    for (i = 0u; i < length; ++i) nca_protocol_feed_byte(protocol, data[i]);
}

size_t nca_frame_encoded_size(uint16_t payload_length)
{
    return (size_t)payload_length + NCA_FRAME_OVERHEAD;
}

size_t nca_frame_encode(uint8_t destination, uint8_t command,
                        const uint8_t *payload, uint16_t payload_length,
                        uint8_t *output, size_t output_capacity)
{
    size_t i;
    size_t required = nca_frame_encoded_size(payload_length);
    uint8_t crc;
    if (output == NULL || output_capacity < required ||
        (payload == NULL && payload_length != 0u)) return 0u;

    output[0] = NCA_SOF1;
    output[1] = destination;
    output[2] = command;
    output[3] = (uint8_t)payload_length;
    output[4] = (uint8_t)(payload_length >> 8);
    crc = command ^ output[3] ^ output[4];
    for (i = 0u; i < payload_length; ++i) {
        output[5u + i] = payload[i];
        crc ^= payload[i];
    }
    output[5u + payload_length] = crc;
    return required;
}
