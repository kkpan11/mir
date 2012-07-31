/*
 * Copyright © 2012 Canonical Ltd.
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
 * Authored by: Thomas Voss <thomas.voss@canonical.com>
 */

#include "mir/frontend/protobuf_asio_communicator.h"
#include <gtest/gtest.h>

namespace mf = mir::frontend;

namespace ba = boost::asio;
namespace bs = boost::system;

namespace
{
struct SessionEventCollector
{
    SessionEventCollector() : session_count(0)
    {
    }

    SessionEventCollector(SessionEventCollector const &) = delete;

    void on_session_event(std::shared_ptr<mf::Session> const& session, mf::SessionEvent event)
    {
        std::unique_lock<std::mutex> ul(guard);
        if (event == mf::SessionEvent::connected)
        {
            ++session_count;
            sessions.insert(session);
        }
        else if (event == mf::SessionEvent::disconnected)
        {
            --session_count;
            EXPECT_EQ(sessions.erase(session), 1u);
        }
        else
        {
            FAIL() << "unknown session event!";
        }
        wait_condition.notify_one();
    }

    std::mutex guard;
    std::condition_variable wait_condition;
    int session_count;
    std::set<std::shared_ptr<mf::Session>> sessions;
};

struct ProtobufAsioCommunicatorTestFixture : public ::testing::Test
{
    static const std::string& socket_name()
    {
        static std::string socket_name("/tmp/mir_test_pb_asio_socket");
        return socket_name;
    }

    ProtobufAsioCommunicatorTestFixture() : comm(socket_name())
    {
    }

    void SetUp()
    {
        comm.signal_session_event().connect(
                boost::bind(
                    &SessionEventCollector::on_session_event,
                    &collector, _1, _2));

        comm.start();
    }

    void expect_session_count(int expected_count)
    {
        std::unique_lock<std::mutex> ul(collector.guard);
        for (int ntries = 20;
             ntries-- != 0 && collector.session_count != expected_count; )
        {
            collector.wait_condition.wait_for(ul, std::chrono::milliseconds(50));
        }
        EXPECT_EQ(collector.session_count, expected_count);
    }

    ba::io_service io_service;
    mf::ProtobufAsioCommunicator comm;
    SessionEventCollector collector;
};
}

TEST_F(ProtobufAsioCommunicatorTestFixture, connection_results_in_a_callback)
{
    ba::local::stream_protocol::socket socket(io_service);

    socket.connect(socket_name());
    expect_session_count(1);
}

TEST_F(ProtobufAsioCommunicatorTestFixture,
        a_connection_attempt_results_in_a_session_being_created)
{
    ba::local::stream_protocol::socket socket(io_service);

    socket.connect(socket_name());
    expect_session_count(1);

    EXPECT_FALSE(collector.sessions.empty());
}

TEST_F(ProtobufAsioCommunicatorTestFixture,
       each_connection_attempt_results_in_a_new_session_being_created)
{
    int const connection_count{5};

    for (int i = 0; i != connection_count; ++i)
    {
        ba::local::stream_protocol::socket socket(io_service);
        socket.connect(socket_name());
    }

    expect_session_count(connection_count);
    EXPECT_EQ(connection_count, (int)collector.sessions.size());
}

TEST_F(ProtobufAsioCommunicatorTestFixture,
       connect_then_disconnect_a_session)
{
    ba::local::stream_protocol::socket socket(io_service);

    socket.connect(socket_name());

    expect_session_count(1);

    bs::error_code error;
    ba::write(socket, ba::buffer(std::string("disconnect\n")), error);
    EXPECT_FALSE(error);

    expect_session_count(0);
}

namespace
{
// Synchronously writes the message to the socket one character at a time.
void write_fragmented_message(ba::local::stream_protocol::socket & socket, std::string const & message)
{
    bs::error_code error;

    for (size_t i = 0, n = message.size(); i != n; ++i)
    {
        ba::write(socket, ba::buffer(message.substr(i, 1)), error);
        EXPECT_FALSE(error);
    }
}
}

TEST_F(ProtobufAsioCommunicatorTestFixture,
       connect_then_disconnect_a_session_with_a_fragmented_message)
{
    ba::local::stream_protocol::socket socket(io_service);

    socket.connect(socket_name());
    expect_session_count(1);
    write_fragmented_message(socket, "disconnect\n");
    expect_session_count(0);
}

