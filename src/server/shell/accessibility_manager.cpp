/*
 * Copyright © Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 or 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mir/shell/accessibility_manager.h"

#include "mir/event_printer.h"
#include "mir/input/input_sink.h"
#include "mir/log.h"
#include "mir/options/configuration.h"
#include "mir/shell/keyboard_helper.h"

#include <xkbcommon/xkbcommon-keysyms.h>

#include <memory>
#include <optional>
#include <sstream>

void mir::shell::AccessibilityManager::register_keyboard_helper(std::shared_ptr<KeyboardHelper> const& helper)
{
    keyboard_helpers.push_back(helper);
}

std::optional<int> mir::shell::AccessibilityManager::repeat_rate() const {
    if(!enable_key_repeat)
        return {};
    return repeat_rate_;
}

int mir::shell::AccessibilityManager::repeat_delay() const {
    return repeat_delay_;
}

void mir::shell::AccessibilityManager::repeat_rate(int new_rate) {
    repeat_rate_ = new_rate;
}

void mir::shell::AccessibilityManager::repeat_delay(int new_delay) {
    repeat_delay_ = new_delay;
}

void mir::shell::AccessibilityManager::notify_helpers() const {
    for(auto const& helper: keyboard_helpers)
        helper->repeat_info_changed(repeat_rate(), repeat_delay());
}

struct MouseKeysTransformer: public mir::input::InputEventTransformer::Transformer
{
    bool transform_input_event(
        std::shared_ptr<mir::input::InputEventTransformer::EventDispatcher> const& dispatcher,
        MirEvent const& event) override
    {
        using namespace mir; // For operator<<

        std::stringstream ss;
        ss << const_cast<MirEvent&>(event);
        mir::log_debug("%s", ss.str().c_str());

        if (mir_event_get_type(&event) != mir_event_type_input)
            return false;

        MirInputEvent const* input_event = mir_event_get_input_event(&event);

        if (mir_input_event_get_type(input_event) == mir_input_event_type_key)
        {
            MirKeyboardEvent const* kev = mir_input_event_get_keyboard_event(input_event);
            if (mir_keyboard_event_action(kev) != mir_keyboard_action_down &&
                mir_keyboard_event_action(kev) != mir_keyboard_action_repeat)
                return false;

            auto offset = geometry::DisplacementF{};
            switch (mir_keyboard_event_keysym(kev))
            {
            case XKB_KEY_KP_2:
                offset = geometry::DisplacementF{0, 10};
                break;
            case XKB_KEY_KP_4:
                offset = geometry::DisplacementF{-10, 0};
                break;
            case XKB_KEY_KP_6:
                offset = geometry::DisplacementF{10, 0};
                break;
            case XKB_KEY_KP_8:
                offset = geometry::DisplacementF{0, -10};
                break;

            default:
                return false;
            }

            dispatcher->dispatch_pointer_event(
                std::nullopt,
                mir_pointer_action_motion,
                0,
                std::nullopt,
                offset,
                mir_pointer_axis_source_none,
                mir::events::ScrollAxisH{},
                mir::events::ScrollAxisV{});

            return true;
        }

        return false;
    }
};

mir::shell::AccessibilityManager::AccessibilityManager(
    std::shared_ptr<mir::options::Option> const& options,
    std::shared_ptr<input::InputEventTransformer> const& event_transformer) :
    enable_key_repeat{options->get<bool>(options::enable_key_repeat_opt)},
    enable_mouse_keys{options->get<bool>(options::enable_mouse_keys_opt)},
    event_transformer{event_transformer},
    transformer{std::make_shared<MouseKeysTransformer>()}
{
    if (enable_mouse_keys)
        event_transformer->append(transformer);
}
