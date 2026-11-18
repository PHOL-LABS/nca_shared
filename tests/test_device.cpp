#include <catch2/catch_test_macros.hpp>

#include <array>

#include <nca/nca_device.h>
#include <nca/nca_topics.h>

TEST_CASE("device owns caller-provided raw and RGB state")
{
    std::array<std::uint8_t, 8> segments{};
    std::array<NcaRgb, 8> colors{};
    std::array<NcaRgb, 8> defaults{};
    defaults.fill(NcaRgb{1, 2, 3});
    NcaDeviceState state;
    nca_device_init(&state, segments.data(), colors.data(), segments.size(),
                    true, defaults.data(), NCA_TOPIC_RPM, 0.0f, 8000.0f);
    REQUIRE(segments[7] == 1);
    REQUIRE(colors[0].g == 2);

    const std::array<std::uint8_t, 5> packed{2, 0, 4, 0, 0b00000101};
    auto result = nca_device_process(&state, NCA_CMD_SET_PACKED_RANGE,
                                     packed.data(), packed.size(), false);
    REQUIRE((result.flags & NCA_PROCESS_RAW_CHANGED) != 0);
    REQUIRE(segments[2] == 1);
    REQUIRE(segments[3] == 0);
    REQUIRE(segments[4] == 1);

    const std::array<std::uint8_t, 7> rgb{1, 0, 1, 0, 10, 20, 30};
    result = nca_device_process(&state, NCA_CMD_SET_RGB_RANGE,
                                rgb.data(), rgb.size(), false);
    REQUIRE((result.flags & NCA_PROCESS_RGB_CHANGED) != 0);
    REQUIRE(colors[1].r == 10);
    REQUIRE(colors[1].g == 20);
    REQUIRE(colors[1].b == 30);
}

TEST_CASE("device reports interactive changes independently from rendering")
{
    std::array<std::uint8_t, 2> segments{};
    std::array<NcaRgb, 2> colors{};
    NcaDeviceState state;
    nca_device_init(&state, segments.data(), colors.data(), segments.size(),
                    false, nullptr, NCA_TOPIC_RPM, 0.0f, 8000.0f);
    std::array<std::uint8_t, 8> payload{};
    auto size = nca_publish_encode_u16(payload.data(), payload.size(),
                                       NCA_TOPIC_RPM, 1, 6500);
    auto result = nca_device_process(&state, NCA_CMD_PUBLISH,
                                     payload.data(), size, true);
    REQUIRE((result.flags & NCA_PROCESS_TOPIC_CHANGED) != 0);
    REQUIRE(result.topic == NCA_TOPIC_RPM);
    REQUIRE(state.interactive.rpm == 6500);
}

TEST_CASE("address changes require unicast and a valid address")
{
    std::array<std::uint8_t, 1> segments{};
    std::array<NcaRgb, 1> colors{};
    NcaDeviceState state;
    nca_device_init(&state, segments.data(), colors.data(), 1,
                    false, nullptr, NCA_TOPIC_NONE, 0.0f, 1.0f);
    const std::array<std::uint8_t, 1> address{9};
    auto result = nca_device_process(&state, NCA_CMD_SET_ADDRESS,
                                     address.data(), address.size(), true);
    REQUIRE((result.flags & NCA_PROCESS_ADDRESS_CHANGED) == 0);
    result = nca_device_process(&state, NCA_CMD_SET_ADDRESS,
                                address.data(), address.size(), false);
    REQUIRE((result.flags & NCA_PROCESS_ADDRESS_CHANGED) != 0);
    REQUIRE(result.address == 9);
}
