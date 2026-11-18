#ifndef NCA_PROTOCOL_H
#define NCA_PROTOCOL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NCA_SOF1               0xA5u
#define NCA_BROADCAST_ADDRESS  0x5Au
#define NCA_LISTEN_ALL_ADDRESS 0x00u

#define NCA_CMD_SET_ALL          0x01u
#define NCA_CMD_SET_RANGE        0x02u
#define NCA_CMD_SET_PACKED_RANGE 0x03u
#define NCA_CMD_PUBLISH          0x10u
#define NCA_CMD_SET_MODE         0x20u
#define NCA_CMD_SET_CONFIG       0x21u
#define NCA_CMD_SET_RGB_ENABLED  0x30u
#define NCA_CMD_SET_RGB_ALL      0x31u
#define NCA_CMD_SET_RGB_RANGE    0x32u
#define NCA_CMD_SET_ADDRESS      0x33u
#define NCA_CMD_ENTER_BOOTLOADER 0x40u
#define NCA_CMD_FW_BEGIN         0x41u
#define NCA_CMD_FW_DATA          0x42u
#define NCA_CMD_FW_END           0x43u
#define NCA_CMD_FW_ABORT         0x44u
#define NCA_CMD_FW_SEGMENT_CRC   0x45u

#ifndef NCA_PROTOCOL_MAX_PAYLOAD
#define NCA_PROTOCOL_MAX_PAYLOAD 192u
#endif

#define NCA_FRAME_OVERHEAD 6u

typedef void (*NcaFrameHandler)(void *user, uint8_t command,
                                const uint8_t *payload, uint16_t length);

typedef enum {
    NCA_WAIT_SOF1 = 0,
    NCA_WAIT_DESTINATION,
    NCA_WAIT_COMMAND,
    NCA_WAIT_LENGTH_LOW,
    NCA_WAIT_LENGTH_HIGH,
    NCA_WAIT_PAYLOAD,
    NCA_WAIT_CRC
} NcaParserState;

typedef struct {
    NcaParserState state;
    uint8_t device_address;
    uint8_t destination;
    uint8_t command;
    uint8_t crc;
    uint16_t length;
    uint16_t position;
    uint8_t payload[NCA_PROTOCOL_MAX_PAYLOAD];
    NcaFrameHandler handler;
    void *user;
} NcaProtocol;

void nca_protocol_init(NcaProtocol *protocol, uint8_t device_address,
                       NcaFrameHandler handler, void *user);
void nca_protocol_reset(NcaProtocol *protocol);
void nca_protocol_feed(NcaProtocol *protocol, const uint8_t *data, size_t length);
void nca_protocol_set_address(NcaProtocol *protocol, uint8_t device_address);
uint8_t nca_protocol_get_address(const NcaProtocol *protocol);
bool nca_protocol_address_is_valid(uint8_t address);

size_t nca_frame_encoded_size(uint16_t payload_length);
size_t nca_frame_encode(uint8_t destination, uint8_t command,
                        const uint8_t *payload, uint16_t payload_length,
                        uint8_t *output, size_t output_capacity);

#ifdef __cplusplus
}
#endif

#endif
