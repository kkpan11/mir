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
 * Authored by: Daniel van Vugt <daniel.van.vugt@canonical.com>
 */

#ifndef MIR_TEST_DOUBLES_FAKE_RENDERABLE_H_
#define MIR_TEST_DOUBLES_FAKE_RENDERABLE_H_

#include "mir/graphics/renderable.h"
#include <glm/gtc/matrix_transform.hpp>
#include <gmock/gmock.h>

namespace mir
{
namespace test
{
namespace doubles
{

class FakeRenderable : public graphics::Renderable
{
public:
    FakeRenderable(int x, int y, int width, int height,
                   float opacity=1.0f,
                   bool rectangular=true,
                   bool visible=true,
                   bool posted=true)
        : rect{{x, y}, {width, height}},
          opacity(opacity),
          rectangular(rectangular),
          visible(visible),
          posted(posted)
    {
    }

    float alpha() const override
    {
        return opacity;
    }

    glm::mat4 transformation() const override
    {
        return glm::mat4();
    }

    bool should_be_rendered_in(const mir::geometry::Rectangle &r) const override
    {
        return visible && posted && rect.overlaps(r);
    }

    bool shaped() const override
    {
        return !rectangular;
    }

    void set_buffer(std::shared_ptr<graphics::Buffer> b)
    {
        buf = b;
    }

    std::shared_ptr<graphics::Buffer> buffer(unsigned long) const override
    {
        return buf;
    }

    bool alpha_enabled() const override
    {
        return shaped() || alpha() < 1.0f;
    }

    geometry::Rectangle screen_position() const override
    {
        return rect;
    }

    int buffers_ready_for_compositor() const override
    {
        return 1;
    }

private:
    std::shared_ptr<graphics::Buffer> buf;
    mir::geometry::Rectangle rect;
    float opacity;
    bool rectangular;
    bool visible;
    bool posted;
};

} // namespace doubles
} // namespace test
} // namespace mir
#endif // MIR_TEST_DOUBLES_FAKE_RENDERABLE_H_
