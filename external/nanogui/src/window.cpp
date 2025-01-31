/*
    src/window.cpp -- Top-level window widget

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/window.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>
#include <nanogui/screen.h>
#include <nanogui/layout.h>

NAMESPACE_BEGIN(nanogui)

Window::Window(std::optional<std::reference_wrapper<Widget>> parent,  const std::string &title)
    : Widget(parent), m_title(title), m_button_panel(nullptr), m_modal(false),
m_drag(false), m_draggable(false), m_background_color(std::nullopt) { }

Vector2i Window::preferred_size(NVGcontext *ctx) {
	// Start with the preferred size from the base Widget
	Vector2i result = Widget::preferred_size(ctx);
	
	// Calculate the size required for the window title
	nvgFontSize(ctx, 18.0f);
	nvgFontFace(ctx, "sans-bold");
	float bounds[4];
	nvgTextBounds(ctx, 0, 0, m_title.c_str(), nullptr, bounds);
	int title_width = static_cast<int>(bounds[2] - bounds[0] + 20);
	int title_height = static_cast<int>(bounds[3] - bounds[1]);
	
	// Calculate the size required for the button panel, if it exists
	Vector2i button_size(0, 0);
	if (m_button_panel) {
		button_size = m_button_panel->preferred_size(ctx);
	}
	
	// Determine the final preferred size by considering the title and button panel
	return Vector2i(
					std::max(result.x(), title_width + button_size.x()),
					std::max(result.y(), title_height + 20) // Add some padding if necessary
					);
}

Widget& Window::button_panel() {
    if (!m_button_panel) {
        m_button_panel = std::make_unique<Widget>(std::make_optional<std::reference_wrapper<Widget>>(*this));
        m_button_panel->set_layout(std::make_unique< BoxLayout>(Orientation::Horizontal, Alignment::Middle, 0, 4));
    }
    return *m_button_panel;
}
void Window::perform_layout(NVGcontext *ctx) {
	if (m_modal) {
		center(); // Ensure the window remains centered if modal
	}
	
	Widget::perform_layout(ctx);
	
	if (m_button_panel) {
		for (auto &w : m_button_panel->children()) {
			w.get().set_fixed_size(Vector2i(22, 22));
			w.get().set_font_size(15);
		}
		m_button_panel->set_size(Vector2i(width(), 22));
		m_button_panel->set_position(Vector2i(
											  width() - (m_button_panel->preferred_size(ctx).x() + 5), 3));
		m_button_panel->perform_layout(ctx);
	}
}

void Window::set_modal(bool modal) {
	m_modal = modal;
	if (m_modal) {
		// Bring to front by moving to the end of the parent's children list
		auto& p = parent()->get();
		
		// Remove the window from its current position
		p.remove_child(std::ref(*this));
		// Add it back to the end, which typically renders it on top
		p.add_child(std::ref(*this));

		center();
		
		request_focus();
	}
}


void Window::draw(NVGcontext *ctx) {
    int ds = theme().m_window_drop_shadow_size, cr = theme().m_window_corner_radius;
    int hh = theme().m_window_header_height;

    /* Draw window */
    nvgSave(ctx);
    nvgBeginPath(ctx);
    nvgRoundedRect(ctx, m_pos.x(), m_pos.y(), m_size.x(), m_size.y(), cr);

	if (m_background_color != std::nullopt) {
		nvgFillColor(ctx, m_background_color.value());
	} else {
		nvgFillColor(ctx, m_mouse_focus ? theme().m_window_fill_focused
					 : theme().m_window_fill_unfocused);
	}

    nvgFill(ctx);


    /* Draw a drop shadow */
    NVGpaint shadow_paint = nvgBoxGradient(
        ctx, m_pos.x(), m_pos.y(), m_size.x(), m_size.y(), cr*2, ds*2,
        theme().m_drop_shadow, theme().m_transparent);

    nvgSave(ctx);
    nvgResetScissor(ctx);
    nvgBeginPath(ctx);
    nvgRect(ctx, m_pos.x()-ds,m_pos.y()-ds, m_size.x()+2*ds, m_size.y()+2*ds);
    nvgRoundedRect(ctx, m_pos.x(), m_pos.y(), m_size.x(), m_size.y(), cr);
    nvgPathWinding(ctx, NVG_HOLE);
    nvgFillPaint(ctx, shadow_paint);
    nvgFill(ctx);
    nvgRestore(ctx);

    if (!m_title.empty()) {
        /* Draw header */
        NVGpaint header_paint = nvgLinearGradient(
            ctx, m_pos.x(), m_pos.y(), m_pos.x(),
            m_pos.y() + hh,
            theme().m_window_header_gradient_top,
            theme().m_window_header_gradient_bot);

        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, m_pos.x(), m_pos.y(), m_size.x(), hh, cr);

        nvgFillPaint(ctx, header_paint);
        nvgFill(ctx);

        nvgBeginPath(ctx);
        nvgRoundedRect(ctx, m_pos.x(), m_pos.y(), m_size.x(), hh, cr);
        nvgStrokeColor(ctx, theme().m_window_header_sep_top);

        nvgSave(ctx);
        nvgIntersectScissor(ctx, m_pos.x(), m_pos.y(), m_size.x(), 0.5f);
        nvgStroke(ctx);
        nvgRestore(ctx);

        nvgBeginPath(ctx);
        nvgMoveTo(ctx, m_pos.x() + 0.5f, m_pos.y() + hh - 1.5f);
        nvgLineTo(ctx, m_pos.x() + m_size.x() - 0.5f, m_pos.y() + hh - 1.5);
        nvgStrokeColor(ctx, theme().m_window_header_sep_bot);
        nvgStroke(ctx);

        nvgFontSize(ctx, 18.0f);
        nvgFontFace(ctx, "sans-bold");
        nvgTextAlign(ctx, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

        nvgFontBlur(ctx, 2);
        nvgFillColor(ctx, theme().m_drop_shadow);
        nvgText(ctx, m_pos.x() + m_size.x() / 2,
                m_pos.y() + hh / 2, m_title.c_str(), nullptr);

        nvgFontBlur(ctx, 0);
        nvgFillColor(ctx, theme().m_window_title_focused);
        nvgText(ctx, m_pos.x() + m_size.x() / 2, m_pos.y() + hh / 2 - 1,
                m_title.c_str(), nullptr);
    }

    nvgRestore(ctx);
    Widget::draw(ctx);
}

void Window::center() {
	screen().center_window(*this);
}

bool Window::mouse_enter_event(const Vector2i &p, bool enter) {
    Widget::mouse_enter_event(p, enter);
    return true;
}

bool Window::mouse_drag_event(const Vector2i &, const Vector2i &rel,
                            int button, int /* modifiers */) {
    if (m_drag && m_draggable && (button & (1 << GLFW_MOUSE_BUTTON_1)) != 0) {
        m_pos += rel;
        m_pos = max(m_pos, Vector2i(0));
        m_pos = min(m_pos, parent()->get().size() - m_size);
        return true;
    }
    return false;
}

bool Window::mouse_button_event(const Vector2i &p, int button, bool down, int modifiers) {
	if (m_modal) {
		// Check if the click is outside the window
		if (!contains(p)) {
			// Optionally, you can add logic here to handle clicks outside,
			// such as bringing the modal window back to front
			return true; // Consume the event
		}
	}
	
	// Existing event handling
	if (Widget::mouse_button_event(p, button, down, modifiers))
		return true;
	if (button == GLFW_MOUSE_BUTTON_1) {
		m_drag = down && (p.y() - m_pos.y()) < theme().m_window_header_height;
		return m_drag && m_draggable;
	}
	return false;
}

bool Window::scroll_event(const Vector2i &p, const Vector2f &rel) {
    return Widget::scroll_event(p, rel);
}

void Window::refresh_relative_placement() {
    /* Overridden in \ref Popup */
}

NAMESPACE_END(nanogui)
