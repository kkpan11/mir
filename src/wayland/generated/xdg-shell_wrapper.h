/*
 * AUTOGENERATED - DO NOT EDIT
 *
 * This file is generated from xdg-shell.xml
 * To regenerate, run the “refresh-wayland-wrapper” target.
 */

#ifndef MIR_FRONTEND_WAYLAND_XDG_SHELL_XML_WRAPPER
#define MIR_FRONTEND_WAYLAND_XDG_SHELL_XML_WRAPPER

#include <optional>

#include "mir/fd.h"
#include <wayland-server-core.h>

#include "mir/wayland/wayland_base.h"

namespace mir
{
namespace wayland
{

class XdgWmBase;
class XdgPositioner;
class XdgSurface;
class XdgToplevel;
class XdgPopup;

class XdgWmBase : public Resource
{
public:
    static char const constexpr* interface_name = "xdg_wm_base";

    static XdgWmBase* from(struct wl_resource*);

    XdgWmBase(struct wl_resource* resource, Version<1>);
    virtual ~XdgWmBase();

    void send_ping_event(uint32_t serial) const;

    struct wl_client* const client;
    struct wl_resource* const resource;

    struct Error
    {
        static uint32_t const role = 0;
        static uint32_t const defunct_surfaces = 1;
        static uint32_t const not_the_topmost_popup = 2;
        static uint32_t const invalid_popup_parent = 3;
        static uint32_t const invalid_surface_state = 4;
        static uint32_t const invalid_positioner = 5;
    };

    struct Opcode
    {
        static uint32_t const ping = 0;
    };

    struct Thunks;

    static bool is_instance(wl_resource* resource);

    class Global : public wayland::Global
    {
    public:
        Global(wl_display* display, Version<1>);

        auto interface_name() const -> char const* override;

    private:
        virtual void bind(wl_resource* new_xdg_wm_base) = 0;
        friend XdgWmBase::Thunks;
    };

private:
    virtual void create_positioner(struct wl_resource* id) = 0;
    virtual void get_xdg_surface(struct wl_resource* id, struct wl_resource* surface) = 0;
    virtual void pong(uint32_t serial) = 0;
};

class XdgPositioner : public Resource
{
public:
    static char const constexpr* interface_name = "xdg_positioner";

    static XdgPositioner* from(struct wl_resource*);

    XdgPositioner(struct wl_resource* resource, Version<1>);
    virtual ~XdgPositioner();

    struct wl_client* const client;
    struct wl_resource* const resource;

    struct Error
    {
        static uint32_t const invalid_input = 0;
    };

    struct Anchor
    {
        static uint32_t const none = 0;
        static uint32_t const top = 1;
        static uint32_t const bottom = 2;
        static uint32_t const left = 3;
        static uint32_t const right = 4;
        static uint32_t const top_left = 5;
        static uint32_t const bottom_left = 6;
        static uint32_t const top_right = 7;
        static uint32_t const bottom_right = 8;
    };

    struct Gravity
    {
        static uint32_t const none = 0;
        static uint32_t const top = 1;
        static uint32_t const bottom = 2;
        static uint32_t const left = 3;
        static uint32_t const right = 4;
        static uint32_t const top_left = 5;
        static uint32_t const bottom_left = 6;
        static uint32_t const top_right = 7;
        static uint32_t const bottom_right = 8;
    };

    struct ConstraintAdjustment
    {
        static uint32_t const none = 0;
        static uint32_t const slide_x = 1;
        static uint32_t const slide_y = 2;
        static uint32_t const flip_x = 4;
        static uint32_t const flip_y = 8;
        static uint32_t const resize_x = 16;
        static uint32_t const resize_y = 32;
    };

    struct Thunks;

    static bool is_instance(wl_resource* resource);

private:
    virtual void set_size(int32_t width, int32_t height) = 0;
    virtual void set_anchor_rect(int32_t x, int32_t y, int32_t width, int32_t height) = 0;
    virtual void set_anchor(uint32_t anchor) = 0;
    virtual void set_gravity(uint32_t gravity) = 0;
    virtual void set_constraint_adjustment(uint32_t constraint_adjustment) = 0;
    virtual void set_offset(int32_t x, int32_t y) = 0;
};

class XdgSurface : public Resource
{
public:
    static char const constexpr* interface_name = "xdg_surface";

    static XdgSurface* from(struct wl_resource*);

    XdgSurface(struct wl_resource* resource, Version<1>);
    virtual ~XdgSurface();

    void send_configure_event(uint32_t serial) const;

    struct wl_client* const client;
    struct wl_resource* const resource;

    struct Error
    {
        static uint32_t const not_constructed = 1;
        static uint32_t const already_constructed = 2;
        static uint32_t const unconfigured_buffer = 3;
    };

    struct Opcode
    {
        static uint32_t const configure = 0;
    };

    struct Thunks;

    static bool is_instance(wl_resource* resource);

private:
    virtual void get_toplevel(struct wl_resource* id) = 0;
    virtual void get_popup(struct wl_resource* id, std::optional<struct wl_resource*> const& parent, struct wl_resource* positioner) = 0;
    virtual void set_window_geometry(int32_t x, int32_t y, int32_t width, int32_t height) = 0;
    virtual void ack_configure(uint32_t serial) = 0;
};

class XdgToplevel : public Resource
{
public:
    static char const constexpr* interface_name = "xdg_toplevel";

    static XdgToplevel* from(struct wl_resource*);

    XdgToplevel(struct wl_resource* resource, Version<1>);
    virtual ~XdgToplevel();

    void send_configure_event(int32_t width, int32_t height, struct wl_array* states) const;
    void send_close_event() const;

    struct wl_client* const client;
    struct wl_resource* const resource;

    struct ResizeEdge
    {
        static uint32_t const none = 0;
        static uint32_t const top = 1;
        static uint32_t const bottom = 2;
        static uint32_t const left = 4;
        static uint32_t const top_left = 5;
        static uint32_t const bottom_left = 6;
        static uint32_t const right = 8;
        static uint32_t const top_right = 9;
        static uint32_t const bottom_right = 10;
    };

    struct State
    {
        static uint32_t const maximized = 1;
        static uint32_t const fullscreen = 2;
        static uint32_t const resizing = 3;
        static uint32_t const activated = 4;
    };

    struct Opcode
    {
        static uint32_t const configure = 0;
        static uint32_t const close = 1;
    };

    struct Thunks;

    static bool is_instance(wl_resource* resource);

private:
    virtual void set_parent(std::optional<struct wl_resource*> const& parent) = 0;
    virtual void set_title(std::string const& title) = 0;
    virtual void set_app_id(std::string const& app_id) = 0;
    virtual void show_window_menu(struct wl_resource* seat, uint32_t serial, int32_t x, int32_t y) = 0;
    virtual void move(struct wl_resource* seat, uint32_t serial) = 0;
    virtual void resize(struct wl_resource* seat, uint32_t serial, uint32_t edges) = 0;
    virtual void set_max_size(int32_t width, int32_t height) = 0;
    virtual void set_min_size(int32_t width, int32_t height) = 0;
    virtual void set_maximized() = 0;
    virtual void unset_maximized() = 0;
    virtual void set_fullscreen(std::optional<struct wl_resource*> const& output) = 0;
    virtual void unset_fullscreen() = 0;
    virtual void set_minimized() = 0;
};

class XdgPopup : public Resource
{
public:
    static char const constexpr* interface_name = "xdg_popup";

    static XdgPopup* from(struct wl_resource*);

    XdgPopup(struct wl_resource* resource, Version<1>);
    virtual ~XdgPopup();

    void send_configure_event(int32_t x, int32_t y, int32_t width, int32_t height) const;
    void send_popup_done_event() const;

    struct wl_client* const client;
    struct wl_resource* const resource;

    struct Error
    {
        static uint32_t const invalid_grab = 0;
    };

    struct Opcode
    {
        static uint32_t const configure = 0;
        static uint32_t const popup_done = 1;
    };

    struct Thunks;

    static bool is_instance(wl_resource* resource);

private:
    virtual void grab(struct wl_resource* seat, uint32_t serial) = 0;
};

}
}

#endif // MIR_FRONTEND_WAYLAND_XDG_SHELL_XML_WRAPPER
