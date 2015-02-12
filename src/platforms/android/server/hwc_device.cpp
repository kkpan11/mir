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
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by:
 *   Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "swapping_gl_context.h"
#include "hwc_device.h"
#include "hwc_layerlist.h"
#include "hwc_wrapper.h"
#include "framebuffer_bundle.h"
#include "buffer.h"
#include "hwc_fallback_gl_renderer.h"
#include <limits>
#include <algorithm>

namespace mg = mir::graphics;
namespace mga=mir::graphics::android;
namespace geom = mir::geometry;

namespace
{
bool plane_alpha_is_translucent(mg::Renderable const& renderable)
{
    float static const tolerance
    {
        1.0f/(2.0 * static_cast<float>(std::numeric_limits<decltype(hwc_layer_1_t::planeAlpha)>::max()))
    };
    return (renderable.alpha() < 1.0f - tolerance);
}
}

bool mga::HwcDevice::compatible_renderlist(RenderableList const& list)
{
    if (list.empty())
        return false;

    for (auto const& renderable : list)
    {
        //TODO: enable planeAlpha for (hwc version >= 1.2), 90 deg rotation
        static glm::mat4 const identity;
        if (plane_alpha_is_translucent(*renderable) ||
           (renderable->transformation() != identity))
        {
            return false;
        }
    }
    return true;
}

mga::HwcDevice::HwcDevice(std::shared_ptr<HwcWrapper> const& hwc_wrapper) :
    hwc_wrapper(hwc_wrapper)
{
}

bool mga::HwcDevice::buffer_is_onscreen(mg::Buffer const& buffer) const
{
    /* check the handles, as the buffer ptrs might change between sets */
    auto const handle = buffer.native_buffer_handle().get();
    auto it = std::find_if(
        onscreen_overlay_buffers.begin(), onscreen_overlay_buffers.end(),
        [&handle](std::shared_ptr<mg::Buffer> const& b)
        {
            return (handle == b->native_buffer_handle().get());
        });
    return it != onscreen_overlay_buffers.end();
}

void mga::HwcDevice::commit(
    mga::DisplayName,
    mga::LayerList& hwc_list,
    SwappingGLContext const& context,
    RenderableListCompositor const& list_compositor)
{
    hwc_list.setup_fb(context.last_rendered_buffer());

    hwc_wrapper->prepare({{hwc_list.native_list(), nullptr, nullptr}});

    if (hwc_list.needs_swapbuffers())
    {
        auto rejected_renderables = hwc_list.rejected_renderables();
        if (rejected_renderables.empty())
            context.swap_buffers();
        else
            list_compositor.render(std::move(rejected_renderables), context);
        hwc_list.setup_fb(context.last_rendered_buffer());
        hwc_list.swap_occurred();
    }

    //setup overlays
    std::vector<std::shared_ptr<mg::Buffer>> next_onscreen_overlay_buffers;
    for (auto& layer : hwc_list)
    {
        auto buffer = layer.layer.buffer();
        if (layer.layer.is_overlay() && buffer)
        {
            if (!buffer_is_onscreen(*buffer))
                layer.layer.set_acquirefence();
            next_onscreen_overlay_buffers.push_back(buffer);
        }
    }

    hwc_wrapper->set({{hwc_list.native_list(), nullptr, nullptr}});
    onscreen_overlay_buffers = std::move(next_onscreen_overlay_buffers);

    for (auto& it : hwc_list)
        it.layer.release_buffer();

    mir::Fd retire_fd(hwc_list.retirement_fence());
}

void mga::HwcDevice::content_cleared()
{
    onscreen_overlay_buffers.clear();
}
