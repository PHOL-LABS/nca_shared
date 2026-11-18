#include <catch2/catch_test_macros.hpp>

#include <array>
#include <cstdint>
#include <vector>

#include <nca/nca_protocol.h>

namespace {
struct Capture {
    unsigned calls{};
    std::uint8_t command{};
    std::vector<std::uint8_t> payload;
};

void capture_frame(void *user, std::uint8_t command,
                   const std::uint8_t *payload, std::uint16_t length)
{
    auto &capture = *static_cast<Capture *>(user);
    ++capture.calls;
    capture.command = command;
    capture.payload.assign(payload, payload + length);
}
}

TEST_CASE("frame encoder produces the documented envelope")
{
    const std::array<std::uint8_t, 3> payload{0x10, 0x20, 0x30};
    std::array<std::uint8_t, 16> frame{};
    const auto size = nca_frame_encode(4, NCA_CMD_SET_ALL, payload.data(),
                                       payload.size(), frame.data(), frame.size());
    REQUIRE(size == 9);
    REQUIRE(frame[0] == NCA_SOF1);
    REQUIRE(frame[1] == 4);
    REQUIRE(frame[2] == NCA_CMD_SET_ALL);
    REQUIRE(frame[3] == 3);
    REQUIRE(frame[4] == 0);
    REQUIRE(frame[5] == 0x10);
    REQUIRE(frame[8] == (NCA_CMD_SET_ALL ^ 3u ^ 0x10u ^ 0x20u ^ 0x30u));
    REQUIRE(nca_frame_encode(4, 1, payload.data(), payload.size(), frame.data(), 8) == 0);
}

TEST_CASE("parser accepts fragmented addressed and broadcast frames")
{
    std::array<std::uint8_t, 32> bytes{};
    const std::array<std::uint8_t, 2> payload{7, 9};
    Capture capture;
    NcaProtocol parser;
    nca_protocol_init(&parser, 4, capture_frame, &capture);
    auto size = nca_frame_encode(4, NCA_CMD_SET_RANGE, payload.data(), payload.size(),
                                 bytes.data(), bytes.size());
    for (std::size_t i = 0; i < size; ++i) nca_protocol_feed(&parser, &bytes[i], 1);
    REQUIRE(capture.calls == 1);
    REQUIRE(capture.command == NCA_CMD_SET_RANGE);
    REQUIRE(capture.payload == std::vector<std::uint8_t>{7, 9});

    size = nca_frame_encode(NCA_BROADCAST_ADDRESS, NCA_CMD_SET_MODE,
                            payload.data(), 1, bytes.data(), bytes.size());
    nca_protocol_feed(&parser, bytes.data(), size);
    REQUIRE(capture.calls == 2);
}

TEST_CASE("parser rejects wrong address and damaged checksum")
{
    std::array<std::uint8_t, 16> bytes{};
    const std::array<std::uint8_t, 1> payload{1};
    Capture capture;
    NcaProtocol parser;
    nca_protocol_init(&parser, 4, capture_frame, &capture);
    auto size = nca_frame_encode(5, NCA_CMD_SET_MODE, payload.data(), payload.size(),
                                 bytes.data(), bytes.size());
    nca_protocol_feed(&parser, bytes.data(), size);
    REQUIRE(capture.calls == 0);
    size = nca_frame_encode(4, NCA_CMD_SET_MODE, payload.data(), payload.size(),
                            bytes.data(), bytes.size());
    bytes[size - 1] ^= 1;
    nca_protocol_feed(&parser, bytes.data(), size);
    REQUIRE(capture.calls == 0);
}

TEST_CASE("listen-all parser supports host receivers")
{
    std::array<std::uint8_t, 16> bytes{};
    Capture capture;
    NcaProtocol parser;
    nca_protocol_init(&parser, NCA_LISTEN_ALL_ADDRESS, capture_frame, &capture);
    const auto size = nca_frame_encode(99, NCA_CMD_FW_ABORT, nullptr, 0,
                                       bytes.data(), bytes.size());
    nca_protocol_feed(&parser, bytes.data(), size);
    REQUIRE(capture.calls == 1);
    REQUIRE(parser.device_address == NCA_LISTEN_ALL_ADDRESS);
}

TEST_CASE("invalid runtime addresses are rejected")
{
    NcaProtocol parser;
    nca_protocol_init(&parser, 7, nullptr, nullptr);
    nca_protocol_set_address(&parser, NCA_BROADCAST_ADDRESS);
    REQUIRE(nca_protocol_get_address(&parser) == 7);
    nca_protocol_set_address(&parser, 0xFF);
    REQUIRE(nca_protocol_get_address(&parser) == 7);
    nca_protocol_set_address(&parser, 9);
    REQUIRE(nca_protocol_get_address(&parser) == 9);
}
