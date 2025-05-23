/*
 * Copyright © Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 or 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "include/server/mir/shell/keyboard_helper.h"
#include "src/server/input/default_input_device_hub.h"
#include "src/server/shell/basic_accessibility_manager.h"
#include "src/server/shell/mouse_keys_transformer.h"

#include "mir/dispatch/multiplexing_dispatchable.h"
#include "mir/glib_main_loop.h"
#include "mir/test/fake_shared.h"
#include "mir/input/input_event_transformer.h"
#include "mir/input/mousekeys_keymap.h"

#include "mir/test/doubles/mock_input_seat.h"
#include "mir/test/doubles/mock_key_mapper.h"
#include "mir/test/doubles/mock_led_observer_registrar.h"
#include "mir/test/doubles/mock_server_status_listener.h"
#include "mir/test/doubles/advanceable_clock.h"
#include "mir/test/doubles/stub_cursor.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>


using namespace testing;
namespace mt = mir::test;
namespace mtd = mt::doubles;

struct MockKeyboardHelper: public mir::shell::KeyboardHelper
{
    MockKeyboardHelper() = default;
    virtual ~MockKeyboardHelper() = default;
    MOCK_METHOD(void, repeat_info_changed, (std::optional<int> rate, int delay), (const override));
};

struct MockMouseKeysTransformer : public mir::shell::MouseKeysTransformer
{
    MockMouseKeysTransformer() = default;

    MOCK_METHOD(void, keymap, (mir::input::MouseKeysKeymap const& new_keymap), (override));
    MOCK_METHOD(void, acceleration_factors, (double constant, double linear, double quadratic), (override));
    MOCK_METHOD(void, max_speed, (double x_axis, double y_axis), (override));
    MOCK_METHOD(
        bool,
        transform_input_event,
        (mir::input::InputEventTransformer::EventDispatcher const&, mir::input::EventBuilder*, MirEvent const&),
        (override));
};

struct TestBasicAccessibilityManager : Test
{
    TestBasicAccessibilityManager() :
        main_loop{std::make_shared<mir::GLibMainLoop>(mt::fake_shared(clock))},
        input_device_hub{std::make_shared<mir::input::DefaultInputDeviceHub>(
            mt::fake_shared(mock_seat),
            mt::fake_shared(multiplexer),
            mt::fake_shared(clock),
            mt::fake_shared(mock_key_mapper),
            mt::fake_shared(mock_server_status_listener),
            mt::fake_shared(led_observer_registrar))},
        input_event_transformer{std::make_shared<mir::input::InputEventTransformer>(input_device_hub, main_loop)},
        basic_accessibility_manager{
            input_event_transformer,
            true,
            std::make_shared<mir::test::doubles::StubCursor>(),
            mock_mousekeys_transformer}
    {
        basic_accessibility_manager.register_keyboard_helper(mock_key_helper);
    }

    mtd::AdvanceableClock clock;
    mir::dispatch::MultiplexingDispatchable multiplexer;
    NiceMock<mtd::MockLedObserverRegistrar> led_observer_registrar;
    NiceMock<mtd::MockInputSeat> mock_seat;
    NiceMock<mtd::MockKeyMapper> mock_key_mapper;
    NiceMock<mtd::MockServerStatusListener> mock_server_status_listener;
    std::shared_ptr<NiceMock<MockKeyboardHelper>> mock_key_helper{std::make_shared<NiceMock<MockKeyboardHelper>>()};
    std::shared_ptr<NiceMock<MockMouseKeysTransformer>> mock_mousekeys_transformer{
        std::make_shared<NiceMock<MockMouseKeysTransformer>>()};

    std::shared_ptr<mir::MainLoop> const main_loop;
    std::shared_ptr<mir::input::DefaultInputDeviceHub> const input_device_hub;
    std::shared_ptr<mir::input::InputEventTransformer> const input_event_transformer;

    mir::shell::BasicAccessibilityManager basic_accessibility_manager;
};

TEST_F(TestBasicAccessibilityManager, changing_repeat_rate_and_notifying_calls_repeat_info_changed)
{
    EXPECT_CALL(*mock_key_helper, repeat_info_changed({70}, _));
    basic_accessibility_manager.repeat_rate(70);
    basic_accessibility_manager.notify_helpers();
}

TEST_F(TestBasicAccessibilityManager, changing_repeat_delay_and_notifying_calls_repeat_info_changed)
{
    EXPECT_CALL(*mock_key_helper, repeat_info_changed(_, 1337));
    basic_accessibility_manager.repeat_delay(1337);
    basic_accessibility_manager.notify_helpers();
}

TEST_F(
    TestBasicAccessibilityManager,
    changing_repeat_rate_to_nullopt_and_notifying_calls_repeat_info_changed_with_zero_rate)
{
    EXPECT_CALL(*mock_key_helper, repeat_info_changed(std::optional(0), _));
    basic_accessibility_manager.repeat_rate({});
    basic_accessibility_manager.notify_helpers();
}

TEST_F(TestBasicAccessibilityManager, set_repeat_rate_value_is_the_same_as_the_get_value)
{
    auto const expected = 12;
    basic_accessibility_manager.repeat_rate(expected);

    ASSERT_EQ(basic_accessibility_manager.repeat_rate(), expected);
}

TEST_F(TestBasicAccessibilityManager, set_repeat_delay_value_is_the_same_as_the_get_value)
{
    auto const expected = 9001;
    basic_accessibility_manager.repeat_delay(expected);

    ASSERT_EQ(basic_accessibility_manager.repeat_delay(), expected);
}

namespace
{
MATCHER_P(KeymapMatches, expected, "")
{
    std::vector<std::pair<mir::input::XkbSymkey, mir::input::MouseKeysKeymap::Action>> expectedPairs, actualPairs;

    expected.for_each_key_action_pair(
        [&expectedPairs](auto key, auto action)
        {
            expectedPairs.emplace_back(key, action);
        });

    arg.for_each_key_action_pair(
        [&actualPairs](auto key, auto action)
        {
            actualPairs.emplace_back(key, action);
        });

    return expectedPairs == actualPairs;
}
}

TEST_F(TestBasicAccessibilityManager, calling_set_mousekeys_keymap_calls_set_keymap_on_transformer)
{
    using enum mir::input::MouseKeysKeymap::Action;
    auto const keymap = mir::input::MouseKeysKeymap{{
        {XKB_KEY_w, move_up},
        {XKB_KEY_s, move_down},
        {XKB_KEY_a, move_left},
        {XKB_KEY_d, move_right},
    }};

    EXPECT_CALL(*mock_mousekeys_transformer, keymap(KeymapMatches(keymap)));

    basic_accessibility_manager.mousekeys_enabled(true);
    basic_accessibility_manager.mousekeys_keymap(keymap);
}

TEST_F(TestBasicAccessibilityManager, calling_set_acceleration_factors_calls_set_acceleration_factors_on_transformer)
{
    auto const constant = 12.9, linear = 3737.0, quadratic = 111.0;
    EXPECT_CALL(*mock_mousekeys_transformer, acceleration_factors(constant, linear, quadratic));

    basic_accessibility_manager.mousekeys_enabled(true);
    basic_accessibility_manager.acceleration_factors(constant, linear, quadratic);
}

TEST_F(TestBasicAccessibilityManager, calling_set_max_speed_calls_set_max_speed_on_transformer)
{
    auto const max_x = 100, max_y = 10710;
    EXPECT_CALL(*mock_mousekeys_transformer, max_speed(max_x, max_y));

    basic_accessibility_manager.mousekeys_enabled(true);
    basic_accessibility_manager.max_speed(max_x, max_y);
}
