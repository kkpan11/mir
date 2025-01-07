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

#include "window.h"
#include "threadsafe_access.h"
#include "basic_decoration.h"
#include "decoration_strategy.h"

#include "mir/scene/surface.h"
#include "mir/scene/null_surface_observer.h"

namespace ms = mir::scene;
namespace geom = mir::geometry;
namespace msd = mir::shell::decoration;

msd::WindowState::WindowState(
    std::shared_ptr<const DecorationStrategy>&& decoration_strategy,
    std::shared_ptr<scene::Surface> const& surface,
    float scale)
    : decoration_strategy{std::move(decoration_strategy)},
      window_size_{surface->window_size()},
      border_type_{border_type_for(surface->type(), surface->state())},
      focus_state_{surface->focus_state()},
      window_name_{surface->name()},
      scale_{scale}
{
}

auto msd::WindowState::window_size() const -> geom::Size
{
    return window_size_;
}

auto msd::WindowState::border_type() const -> BorderType
{
    return border_type_;
}

auto msd::WindowState::focused_state() const -> MirWindowFocusState
{
    return focus_state_;
}

auto msd::WindowState::window_name() const -> std::string
{
    return window_name_;
}

auto msd::WindowState::titlebar_width() const -> geom::Width
{
    switch (border_type_)
    {
    case BorderType::Full:
    case BorderType::Titlebar:
        return window_size().width;
    case BorderType::None:
        return {};
    }

    mir::fatal_error("%s:%d: should be unreachable", __FILE__, __LINE__);
    return {};
}

auto msd::WindowState::titlebar_height() const -> geom::Height
{
    switch (border_type_)
    {
    case BorderType::Full:
    case BorderType::Titlebar:
        return decoration_strategy->titlebar_height();
    case BorderType::None:
        return {};
    }

    mir::fatal_error("%s:%d: should be unreachable", __FILE__, __LINE__);
    return {};
}

auto msd::WindowState::side_border_width() const -> geom::Width
{
    switch (border_type_)
    {
    case BorderType::Full:
        return decoration_strategy->side_border_width();
    case BorderType::Titlebar:
    case BorderType::None:
        return {};
    }

    mir::fatal_error("%s:%d: should be unreachable", __FILE__, __LINE__);
    return {};
}

auto msd::WindowState::side_border_height() const -> geom::Height
{
    switch (border_type_)
    {
    case BorderType::Full:
        return window_size().height - as_delta(titlebar_height()) - as_delta(bottom_border_height());
    case BorderType::Titlebar:
    case BorderType::None:
        return {};
    }

    mir::fatal_error("%s:%d: should be unreachable", __FILE__, __LINE__);
    return {};
}

auto msd::WindowState::bottom_border_width() const -> geom::Width
{
    return titlebar_width();
}

auto msd::WindowState::bottom_border_height() const -> geom::Height
{
    switch (border_type_)
    {
    case BorderType::Full:
        return decoration_strategy->bottom_border_height();
    case BorderType::Titlebar:
    case BorderType::None:
        return {};
    }

    mir::fatal_error("%s:%d: should be unreachable", __FILE__, __LINE__);
    return {};
}

auto msd::WindowState::titlebar_rect() const -> geom::Rectangle
{
    return {
        geom::Point{},
        {titlebar_width(), titlebar_height()}};
}

auto msd::WindowState::left_border_rect() const -> geom::Rectangle
{
    return {
        {geom::X{}, as_y(titlebar_height())},
        {side_border_width(), side_border_height()}};
}

auto msd::WindowState::right_border_rect() const -> geom::Rectangle
{
    return {
        {as_x(window_size().width - side_border_width()), as_y(titlebar_height())},
        {side_border_width(), side_border_height()}};
}

auto msd::WindowState::bottom_border_rect() const -> geom::Rectangle
{
    return {
        {geom::X{}, as_y(window_size().height - bottom_border_height())},
        {bottom_border_width(), bottom_border_height()}};
}

auto msd::WindowState::scale() const -> float
{
    return scale_;
}

class msd::WindowSurfaceObserverManager::Observer
    : public ms::NullSurfaceObserver
{
public:
    Observer(std::shared_ptr<ThreadsafeAccess<BasicDecoration>> const& decoration)
        : decoration{decoration}
    {
    }

    /// Overrides from NullSurfaceObserver
    /// @{
    void attrib_changed(ms::Surface const*, MirWindowAttrib attrib, int /*value*/) override
    {
        switch(attrib)
        {
        case mir_window_attrib_type:
        case mir_window_attrib_state:
        case mir_window_attrib_focus:
        case mir_window_attrib_visibility:
            update();
            break;

        case mir_window_attrib_dpi:
        case mir_window_attrib_preferred_orientation:
        case mir_window_attribs:
            break;
        }
    }

    void window_resized_to(ms::Surface const*, geom::Size const& /*window_size*/) override
    {
        update();
    }

    void renamed(ms::Surface const*, std::string const& /*name*/) override
    {
        update();
    }
    /// @}

private:
    void update()
    {
        decoration->spawn([](auto decoration)
            {
                decoration->window_state_updated();
            });
    }

    std::shared_ptr<ThreadsafeAccess<BasicDecoration>> const decoration;
};

msd::WindowSurfaceObserverManager::WindowSurfaceObserverManager(
    std::shared_ptr<scene::Surface> const& window_surface,
    std::shared_ptr<ThreadsafeAccess<BasicDecoration>> const& decoration)
    : surface_{window_surface},
      observer{std::make_shared<Observer>(decoration)}
{
    surface_->register_interest(observer);
}

msd::WindowSurfaceObserverManager::~WindowSurfaceObserverManager()
{
    surface_->unregister_interest(*observer);
}
