/*
 * Copyright © 2018 Canonical Ltd.
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
 */

#ifndef MIR_FRONTEND_OUTPUT_MANAGER_H_
#define MIR_FRONTEND_OUTPUT_MANAGER_H_

#include <mir/graphics/display_configuration.h>
#include <mir/graphics/null_display_configuration_observer.h>

#include "wayland_wrapper.h"

#include <optional>
#include <memory>
#include <vector>
#include <unordered_map>

namespace mir
{
template<typename T>
class ObserverRegistrar;
class Executor;
namespace frontend
{
class DisplayChanger;
class OutputGlobal;

class OutputInstance : public wayland::Output
{
public:
    OutputInstance(wl_resource* resource, OutputGlobal* global);
    ~OutputInstance();

    static auto from(wl_resource* output) -> OutputInstance*;

    void send_config(graphics::DisplayConfigurationOutput const& initial_configuration);

    wayland::Weak<OutputGlobal> const global;
};

class OutputGlobal: public wayland::LifetimeTracker, wayland::Output::Global
{
public:
    OutputGlobal(wl_display* display, graphics::DisplayConfigurationOutput const& initial_configuration);

    static auto from(wl_resource* output) -> OutputGlobal*;
    static auto from_or_throw(wl_resource* output) -> OutputGlobal&;

    void handle_configuration_changed(graphics::DisplayConfigurationOutput const& config);
    void for_each_output_bound_by(wl_client* client, std::function<void(OutputInstance*)> const& functor);

    auto current_config() -> graphics::DisplayConfigurationOutput const& { return output_config; }

private:
    friend OutputInstance;

    void bind(wl_resource* resource) override;
    void instance_destroyed(OutputInstance* instance);

    graphics::DisplayConfigurationOutput output_config;
    std::unordered_map<wl_client*, std::vector<OutputInstance*>> instances;
};

class OutputManager
{
public:
    OutputManager(
        wl_display* display,
        std::shared_ptr<Executor> const& executor,
        std::shared_ptr<ObserverRegistrar<graphics::DisplayConfigurationObserver>> const& registrar);
    ~OutputManager();

    static auto output_id_for(std::optional<wl_resource*> output)
        -> std::optional<graphics::DisplayConfigurationOutputId>;

    auto output_for(graphics::DisplayConfigurationOutputId id) -> std::optional<OutputGlobal*>;
    auto current_config() -> graphics::DisplayConfiguration const& { return *display_config; }

private:
    void handle_configuration_change(std::shared_ptr<graphics::DisplayConfiguration const> const& config);

    struct DisplayConfigObserver;

    wl_display* const display;
    std::shared_ptr<ObserverRegistrar<graphics::DisplayConfigurationObserver>> const registrar;
    std::shared_ptr<DisplayConfigObserver> const display_config_observer;
    std::unordered_map<graphics::DisplayConfigurationOutputId, std::unique_ptr<OutputGlobal>> outputs;
    std::shared_ptr<graphics::DisplayConfiguration const> display_config;
};
}
}

#endif //MIR_FRONTEND_OUTPUT_MANAGER_H_
