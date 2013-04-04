/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Daniel van Vugt <daniel.van.vugt@canonical.com>
 */

#ifndef MIR_CLIENT_EVENT_HANDLER_
#define MIR_CLIENT_EVENT_HANDLER_

#include "mir/event.h"

namespace mir
{
namespace client
{

class EventHandler
{
public:
    virtual ~EventHandler() {}
    virtual void handle_event(mir::Event const& e) = 0;
};

class NullEventHandler : public EventHandler
{
public:
    void handle_event(mir::Event const &) {}
};

} // namespace client
} // namespace mir

#endif
