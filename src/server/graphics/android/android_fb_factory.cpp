/*
 * Copyright © 2013 Canonical Ltd.
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
 * Authored by: Kevin DuBois <kevin.dubois@canonical.com>
 */

#include "android_fb_factory.h"
#include "hwc_factory.h"
#include "display_factory.h"
#include "android_display.h"
#include "hwc_display.h"

#include <boost/throw_exception.hpp>
#include <stdexcept>

namespace mg=mir::graphics;
namespace mga=mir::graphics::android;

mga::AndroidFBFactory::AndroidFBFactory(std::shared_ptr<DisplayFactory> const& fb_factory,
                                        std::shared_ptr<HWCFactory> const& hwc_factory)
    : fb_factory(fb_factory),
      hwc_factory(hwc_factory)
{
    const hw_module_t *hw_module;
    int rc = hw_get_module(HWC_HARDWARE_MODULE_ID, &hw_module);
    if ((rc != 0) || (hw_module == nullptr))
    {
        return;
    }

    setup_hwc_dev(hw_module);
}

void mga::AndroidFBFactory::setup_hwc_dev(const hw_module_t* module)
{
    if ((!module->methods) || !(module->methods->open))
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("display factory cannot create hwc display"));
    }

    hwc_composer_device_1* hwc_device_raw;
    int rc = module->methods->open(module, HWC_HARDWARE_COMPOSER, reinterpret_cast<hw_device_t**>(&hwc_device_raw));

    if ((rc != 0) || (hwc_device_raw == nullptr))
    {
        BOOST_THROW_EXCEPTION(std::runtime_error("display hwc module unusable"));
    }

    hwc_dev = std::shared_ptr<hwc_composer_device_1>(hwc_device_raw,
                  [](hwc_composer_device_1* device)
                  {
                     device->common.close((hw_device_t*) device);
                  });
}

std::shared_ptr<mg::Display> mga::AndroidFBFactory::create_fb() const
{ 
    if (hwc_dev && (hwc_dev->common.version == HWC_DEVICE_API_VERSION_1_1))
    {
        //TODO: once we can log things here, if this throws, we should log and recover to a gpu display
        auto hwc_device = hwc_factory->create_hwc_1_1(hwc_dev);
        return fb_factory->create_hwc_display(hwc_device);
    }
    else
    {
        return fb_factory->create_gpu_display();
    }
}
