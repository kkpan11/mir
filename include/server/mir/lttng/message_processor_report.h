/*
 * Copyright © 2013 Canonical Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3,
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alexandros Frantzis <alexandros.frantzis@canonical.com>
 */

#ifndef MIR_LTTNG_MESSAGE_PROCESSOR_REPORT_H_
#define MIR_LTTNG_MESSAGE_PROCESSOR_REPORT_H_

#include "mir/frontend/message_processor_report.h"
#include "mir/lttng/server_tracepoint_provider.h"

namespace mir
{
namespace lttng
{

class MessageProcessorReport : public mir::frontend::MessageProcessorReport
{
public:
    void received_invocation(void const* mediator, int id, std::string const& method) override;
    void completed_invocation(void const* mediator, int id, bool result) override;
    void unknown_method(void const* mediator, int id, std::string const& method) override;
    void exception_handled(void const* mediator, int id, std::exception const& error) override;
    void exception_handled(void const* mediator, std::exception const& error) override;
    void sent_event(void const* mediator, MirSurfaceEvent const& event) override;

private:
    ServerTracepointProvider tp_provider;
};

}
}

#endif /* MIR_LTTNG_MESSAGE_PROCESSOR_REPORT_H_ */
