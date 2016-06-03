/*
 * Copyright © 2016 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Andreas Pokorny <andreas.pokorny@canonical.com>
 */

#ifndef MIR_TEST_DOUBLES_KEY_MAPPER_H_
#define MIR_TEST_DOUBLES_KEY_MAPPER_H_

#include "mir/input/key_mapper.h"
#include "mir/input/keymap.h"

#include <gmock/gmock.h>

namespace mir
{
namespace test
{
namespace doubles
{
struct MockKeyMapper : input::KeyMapper
{
    MOCK_METHOD2(set_key_state, void(MirInputDeviceId id, std::vector<uint32_t> const& key_state));
    MOCK_METHOD2(set_keymap, void(MirInputDeviceId id, input::Keymap const& map));
    MOCK_METHOD3(set_keymap, void(MirInputDeviceId id, char const* buffer, size_t len));
    MOCK_METHOD1(set_keymap, void(input::Keymap const& map));
    MOCK_METHOD2(set_keymap, void(char const* buffer, size_t len));
    MOCK_METHOD1(reset_keymap, void(MirInputDeviceId id));
    MOCK_METHOD0(reset_keymap, void());
    MOCK_METHOD1(map_event, void(MirEvent& event));
    MOCK_CONST_METHOD0(modifiers, MirInputEventModifiers());
};


}
}
}

#endif
