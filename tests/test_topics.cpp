#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <array>

#include <nca/nca_interactive.h>
#include <nca/nca_topics.h>

TEST_CASE("scalar topic payloads round-trip through interactive processing")
{
    NcaInteractiveState state;
    std::array<std::uint8_t, 16> payload{};
    nca_interactive_init(&state, NCA_TOPIC_SPEED, 0.0f, 300.0f);

    auto size = nca_publish_encode_u16(payload.data(), payload.size(),
                                       NCA_TOPIC_SPEED, 17, 1234);
    REQUIRE(size == NCA_PUBLISH_SCALAR_PAYLOAD_SIZE);
    REQUIRE(nca_interactive_process(&state, NCA_CMD_PUBLISH,
                                    payload.data(), size) == NCA_TOPIC_SPEED);
    REQUIRE(state.speed_deci == 1234);
    REQUIRE(state.last_sequence == 17);
    REQUIRE(nca_interactive_assigned_value(&state) == Catch::Approx(123.4f));

    size = nca_publish_encode_deci(payload.data(), payload.size(),
                                   NCA_TOPIC_OUTDOOR_TEMPERATURE, 18, -125);
    REQUIRE(nca_interactive_process(&state, NCA_CMD_PUBLISH,
                                    payload.data(), size) == NCA_TOPIC_OUTDOOR_TEMPERATURE);
    REQUIRE(state.outdoor_temperature == Catch::Approx(-12.5f));
}

TEST_CASE("indicator and generic topics are decoded")
{
    NcaInteractiveState state;
    std::array<std::uint8_t, 16> payload{};
    nca_interactive_init(&state, NCA_TOPIC_GENERIC_BASE, -10.0f, 10.0f);
    auto size = nca_publish_encode_indicators(payload.data(), payload.size(), 2,
                                               1u << 27, 1u << 3);
    REQUIRE(nca_interactive_process(&state, NCA_CMD_PUBLISH,
                                    payload.data(), size) == NCA_TOPIC_INDICATORS);
    REQUIRE(nca_interactive_indicator_is_set(&state, 27));
    REQUIRE(nca_interactive_indicator_is_set(&state, 35));

    size = nca_publish_encode_deci(payload.data(), payload.size(),
                                   NCA_TOPIC_GENERIC_BASE, 3, 42);
    REQUIRE(nca_interactive_process(&state, NCA_CMD_PUBLISH,
                                    payload.data(), size) == NCA_TOPIC_GENERIC_BASE);
    REQUIRE(nca_interactive_assigned_value(&state) == Catch::Approx(4.2f));
}

TEST_CASE("mode and range configuration are processed")
{
    NcaInteractiveState state;
    std::array<std::uint8_t, 16> payload{};
    nca_interactive_init(&state, NCA_TOPIC_RPM, 0.0f, 8000.0f);
    payload[0] = NCA_MODE_INTERACTIVE;
    REQUIRE(nca_interactive_process(&state, NCA_CMD_SET_MODE,
                                    payload.data(), 1) == NCA_INTERACTIVE_STATE_CHANGED);
    REQUIRE(state.mode == NCA_MODE_INTERACTIVE);
    const auto size = nca_config_encode_range(payload.data(), payload.size(),
                                               NCA_TOPIC_RPM, 500.0f, 9000.0f);
    REQUIRE(nca_interactive_process(&state, NCA_CMD_SET_CONFIG,
                                    payload.data(), size) == NCA_INTERACTIVE_STATE_CHANGED);
    REQUIRE(state.range_minimum == Catch::Approx(500.0f));
    REQUIRE(state.range_maximum == Catch::Approx(9000.0f));
}

TEST_CASE("malformed publications and configurations are atomic")
{
    NcaInteractiveState state;
    std::array<std::uint8_t, 20> payload{};
    nca_interactive_init(&state, NCA_TOPIC_COOLANT_TEMPERATURE, -40.0f, 130.0f);
    state.rpm = 7000;

    auto size = nca_publish_encode_u16(payload.data(), payload.size(),
                                       NCA_TOPIC_RPM, 1, 1234);
    REQUIRE(nca_interactive_process(&state, NCA_CMD_PUBLISH,
                                    payload.data(), size - 1) == NCA_TOPIC_NONE);
    REQUIRE(state.rpm == 7000);

    payload[0] = NCA_INTERACTIVE_VERSION;
    payload[1] = NCA_TOPIC_NONE;
    payload[2] = NCA_CONFIG_MINIMUM_F32;
    payload[3] = 4;
    payload[8] = 0x80; // unknown extension TLV
    payload[9] = 20;   // extends past the payload and rejects the whole update
    REQUIRE(nca_interactive_process(&state, NCA_CMD_SET_CONFIG,
                                    payload.data(), 11) == NCA_TOPIC_NONE);
    REQUIRE(state.range_minimum == Catch::Approx(-40.0f));
    REQUIRE(state.range_maximum == Catch::Approx(130.0f));
}
