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

#include "mir/graphics/android/sync_fence.h"
#include "src/platform/graphics/android/framebuffer_bundle.h"
#include "src/platform/graphics/android/hwc_device.h"
#include "src/platform/graphics/android/hwc_layerlist.h"
#include "mir_test_doubles/mock_hwc_composer_device_1.h"
#include "mir_test_doubles/mock_android_native_buffer.h"
#include "mir_test_doubles/mock_buffer.h"
#include "mir_test_doubles/mock_hwc_vsync_coordinator.h"
#include "mir_test_doubles/stub_renderable.h"
#include "mir_test_doubles/mock_framebuffer_bundle.h"
#include "mir_test_doubles/stub_buffer.h"
#include "mir_test_doubles/mock_render_function.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdexcept>

namespace mg=mir::graphics;
namespace mga=mir::graphics::android;
namespace mtd=mir::test::doubles;
namespace geom=mir::geometry;

namespace
{

struct MockSwappingContext
{
    MOCK_CONST_METHOD0(make_current, void());
    MOCK_CONST_METHOD0(release_current, void());
    MOCK_CONST_METHOD0(swap_buffers, void());
};

struct StubSwappingContext
{
    void make_current() const {}
    void release_current() const {}
    void swap_buffers() const {}
};

struct MockFileOps : public mga::SyncFileOps
{
    MOCK_METHOD3(ioctl, int(int,int,void*));
    MOCK_METHOD1(dup, int(int));
    MOCK_METHOD1(close, int(int));
};
}

class HwcDevice : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        using namespace testing;

        mock_native_buffer = std::make_shared<testing::NiceMock<mtd::MockAndroidNativeBuffer>>();
        mock_buffer = std::make_shared<testing::NiceMock<mtd::MockBuffer>>();
        mock_device = std::make_shared<testing::NiceMock<mtd::MockHWCComposerDevice1>>();
        mock_vsync = std::make_shared<testing::NiceMock<mtd::MockVsyncCoordinator>>();
        mock_file_ops = std::make_shared<MockFileOps>();

        ON_CALL(*mock_buffer, size())
            .WillByDefault(Return(geom::Size{0,0}));
        ON_CALL(*mock_buffer, native_buffer_handle())
            .WillByDefault(Return(mock_native_buffer));
    }

    std::shared_ptr<MockFileOps> mock_file_ops;
    std::shared_ptr<mtd::MockVsyncCoordinator> mock_vsync;
    std::shared_ptr<mtd::MockHWCComposerDevice1> mock_device;
    std::shared_ptr<mtd::MockAndroidNativeBuffer> mock_native_buffer;
    std::shared_ptr<mtd::MockBuffer> mock_buffer;
    MockSwappingContext mock_context;
    StubSwappingContext stub_context;
};

TEST_F(HwcDevice, hwc_displays)
{
    using namespace testing;
    EXPECT_CALL(*mock_device, prepare_interface(mock_device.get(),_,_))
        .Times(1);
    EXPECT_CALL(*mock_device, set_interface(mock_device.get(),_,_))
        .Times(1);

    mga::HwcDevice device(mock_device, mock_vsync, mock_file_ops);
    device.prepare_gl(stub_context);
    device.post(*mock_buffer);

    /* primary phone display */
    EXPECT_TRUE(mock_device->primary_prepare);
    EXPECT_TRUE(mock_device->primary_set);
    /* external monitor display not supported yet */
    EXPECT_FALSE(mock_device->external_prepare);
    EXPECT_FALSE(mock_device->external_set);
    /* virtual monitor display not supported yet */
    EXPECT_FALSE(mock_device->virtual_prepare);
    EXPECT_FALSE(mock_device->virtual_set);
}

TEST_F(HwcDevice, hwc_prepare)
{
    using namespace testing;

    Sequence seq;
    EXPECT_CALL(*mock_device, prepare_interface(mock_device.get(), 1, _))
        .InSequence(seq);
    EXPECT_CALL(mock_context, swap_buffers())
        .InSequence(seq);

    mga::HwcDevice device(mock_device, mock_vsync, mock_file_ops);
    device.prepare_gl(mock_context);
    EXPECT_EQ(2, mock_device->display0_prepare_content.numHwLayers);
    EXPECT_EQ(-1, mock_device->display0_prepare_content.retireFenceFd);
}

TEST_F(HwcDevice, hwc_prepare_resets_layers)
{
    using namespace testing;
    EXPECT_CALL(*mock_device, prepare_interface(mock_device.get(), 1, _))
        .Times(2);

    mga::HwcDevice device(mock_device, mock_vsync, mock_file_ops);

    std::list<std::shared_ptr<mg::Renderable>> renderlist
    {
        std::make_shared<mtd::StubRenderable>(),
        std::make_shared<mtd::StubRenderable>()
    };
    device.prepare_gl_and_overlays(stub_context, renderlist, [](mg::Renderable const&){});
    EXPECT_EQ(3, mock_device->display0_prepare_content.numHwLayers);

    device.prepare_gl(stub_context);
    EXPECT_EQ(2, mock_device->display0_prepare_content.numHwLayers);
}

TEST_F(HwcDevice, hwc_prepare_with_overlays)
{
    using namespace testing;
    mtd::MockRenderFunction mock_call_counter;

    auto renderable1 = std::make_shared<mtd::StubRenderable>();
    auto renderable2 = std::make_shared<mtd::StubRenderable>();
    std::list<std::shared_ptr<mg::Renderable>> renderlist
    {
        renderable1,
        renderable2
    };

    testing::Sequence seq;
    EXPECT_CALL(*mock_device, prepare_interface(mock_device.get(), 1, _))
        .Times(1);
    EXPECT_CALL(mock_call_counter, called(testing::Ref(*renderable1)))
        .InSequence(seq);
    EXPECT_CALL(mock_call_counter, called(testing::Ref(*renderable2)))
        .InSequence(seq);

    mga::HwcDevice device(mock_device, mock_vsync, mock_file_ops);
    device.prepare_gl_and_overlays(renderlist, [&](mg::Renderable const& renderable)
    {
        mock_call_counter.called(renderable);
    });
    device.post(*mock_buffer);

    EXPECT_EQ(3, mock_device->display0_prepare_content.numHwLayers);
    EXPECT_EQ(3, mock_device->display0_set_content.numHwLayers);
}

#if 0
TEST_F(HwcDevice, hwc_render)
{
    EXPECT_CALL(mock_egl, eglSwapBuffers(dpy,surf))
        .Times(1);
    mga::HwcDevice device(mock_device, mock_vsync, mock_file_ops);
    device.gpu_render(dpy, surf);
}

TEST_F(HwcDevice, hwc_swapbuffers_failure)
{
    using namespace testing;
    EXPECT_CALL(mock_egl, eglSwapBuffers(dpy,surf))
        .Times(1)
        .WillOnce(Return(EGL_FALSE));

    mga::HwcDevice device(mock_device, mock_vsync, mock_file_ops);

    EXPECT_THROW({
        device.gpu_render(dpy, surf);
    }, std::runtime_error);
}
#endif
TEST_F(HwcDevice, hwc_commit)
{
    using namespace testing;
    int hwc_return_fence = 94;
    int hwc_retire_fence = 74;
    mock_device->hwc_set_return_fence(hwc_return_fence);
    mock_device->hwc_set_retire_fence(hwc_retire_fence);

    mga::HwcDevice device(mock_device, mock_vsync, mock_file_ops);

    InSequence seq;
    EXPECT_CALL(*mock_device, set_interface(mock_device.get(), 1, _))
        .Times(1);
    EXPECT_CALL(*mock_native_buffer, update_fence(_))
        .Times(1);
    EXPECT_CALL(*mock_native_buffer, update_fence(hwc_return_fence))
        .Times(1);
    EXPECT_CALL(*mock_file_ops, close(hwc_retire_fence))
        .Times(1);

    device.post(*mock_buffer);

    //set
    EXPECT_EQ(2, mock_device->display0_set_content.numHwLayers);
    EXPECT_EQ(-1, mock_device->display0_set_content.retireFenceFd);
    EXPECT_EQ(HWC_FRAMEBUFFER, mock_device->set_layerlist[0].compositionType);
    EXPECT_EQ(HWC_SKIP_LAYER, mock_device->set_layerlist[0].flags);
    EXPECT_EQ(HWC_FRAMEBUFFER_TARGET, mock_device->set_layerlist[1].compositionType);
    EXPECT_EQ(0, mock_device->set_layerlist[1].flags);
}

TEST_F(HwcDevice, hwc_commit_failure)
{
    using namespace testing;

    mga::HwcDevice device(mock_device, mock_vsync, mock_file_ops);

    EXPECT_CALL(*mock_device, set_interface(mock_device.get(), _, _))
        .Times(1)
        .WillOnce(Return(-1));

    EXPECT_THROW({
        device.post(*mock_buffer);
    }, std::runtime_error);
}
