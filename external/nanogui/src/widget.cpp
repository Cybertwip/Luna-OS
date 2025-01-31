/*
 Optimized src/widget.cpp -- Base class of all widgets
 
 NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
 The widget drawing code is based on the NanoVG demo application
 by Mikko Mononen.
 
 All rights reserved. Use of this source code is governed by a
 BSD-style license that can be found in the LICENSE.txt file.
 */

#include <nanogui/widget.h>
#include <nanogui/layout.h>
#include <nanogui/theme.h>
#include <nanogui/window.h>
#include <nanogui/opengl.h>
#include <nanogui/screen.h>

/* Uncomment the following definition to draw red bounding
 boxes around widgets (useful for debugging drawing code) */

// #define NANOGUI_SHOW_WIDGET_BOUNDS 1

NAMESPACE_BEGIN(nanogui)

Widget::Widget(Widget& parent) : Widget(std::make_optional<std::reference_wrapper<Widget>>(parent)){
	
}

Widget::Widget(std::optional<std::reference_wrapper<Widget>> parent)
: m_parent(parent), m_layout(nullptr),
m_pos(0), m_size(0), m_fixed_size(0), m_visible(true), m_enabled(true),
m_mouse_focus(false), m_tooltip(""), m_font_size(-1.f),
m_icon_extra_scale(1.f), m_cursor(Cursor::Arrow), m_initialized(false) {
	
	if (m_parent.has_value()){
		m_parent->get().add_child(std::ref(*this));
	}
}


Widget::~Widget() {
	if (std::uncaught_exceptions() > 0) {
		/* If a widget constructor throws an exception, it is immediately
		 deallocated but may still be referenced by a parent. Be conservative
		 and don't decrease the reference count of children while dispatching
		 exceptions. */
		return;
	}
	
	screen().remove_from_focus(*this);
}

void Widget::set_theme(std::optional<std::reference_wrapper<Theme>> theme) {
	m_theme = theme;
	for (auto child : m_children)
		child.get().set_theme(theme);
}

int Widget::font_size() {
	return (m_font_size < 0) ? theme().m_standard_font_size : m_font_size;
}

Vector2i Widget::preferred_size(NVGcontext *ctx) {
	if (m_layout)
		return m_layout->preferred_size(ctx, *this);
	else
		return m_size;
}

void Widget::perform_layout(NVGcontext *ctx) {	
	if (m_layout) {
		m_layout->perform_layout(ctx, *this);
	} else {
		for (auto& c : m_children) {
			const Vector2i &pref = c.get().preferred_size(ctx);
			const Vector2i &fix = c.get().fixed_size();
			c.get().set_size(Vector2i(
								 fix[0] ? fix[0] : pref[0],
								 fix[1] ? fix[1] : pref[1]
								 ));
			c.get().perform_layout(ctx);
		}
	}
}

Widget* Widget::find_widget(const Vector2i &p, bool absolute) {
	auto pos = m_pos;
	
	if (absolute) {
		pos = absolute_position();
	}
	for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
		Widget& child = *it;
		if (child.visible() && child.contains(p - pos, absolute))
			return child.find_widget(p - pos);
	}
	return contains(p, absolute) ? this : nullptr;
}

bool Widget::mouse_button_event(const Vector2i &p, int button, bool down, int modifiers) {
	for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
		Widget& child = *it;
		if (child.visible() && child.contains(p - m_pos) &&
			child.mouse_button_event(p - m_pos, button, down, modifiers))
			return true;
	}
	return false;
}

bool Widget::mouse_motion_event(const Vector2i &p, const Vector2i &rel, int button, int modifiers) {
	bool handled = false;
	
	for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
		Widget& child = *it;
		if (!child.visible())
			continue;
		
		bool contained      = child.contains(p - m_pos),
		prev_contained = child.contains(p - m_pos - rel);
		
		if (contained != prev_contained)
			handled |= child.mouse_enter_event(p, contained);
		
		if (contained || prev_contained)
			handled |= child.mouse_motion_event(p - m_pos, rel, button, modifiers);
	}
	
	return handled;
}

bool Widget::scroll_event(const Vector2i &p, const Vector2f &rel) {
	for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
		Widget& child = *it;
		if (!child.visible())
			continue;
		if (child.contains(p - m_pos) && child.scroll_event(p - m_pos, rel))
			return true;
	}
	return false;
}

bool Widget::mouse_drag_event(const Vector2i &, const Vector2i &, int, int) {
	return false;
}

bool Widget::mouse_enter_event(const Vector2i &, bool enter) {
	m_mouse_focus = enter;
	return false;
}

bool Widget::keyboard_event(int, int, int, int) {
	return false;
}

bool Widget::keyboard_character_event(unsigned int) {
	return false;
}

void Widget::add_child(int index, Widget& widget) {
	assert(index <= child_count());
	m_children.insert(m_children.begin() + index, widget);
	widget.set_parent(*this);
	widget.set_screen(screen());
	widget.set_theme(m_theme);
}

void Widget::add_child(Widget& widget) {
	add_child(child_count(), widget);
}

void Widget::remove_child(Widget& widget) {
	auto it = std::find_if(m_children.begin(), m_children.end(),
						   [&widget](const std::reference_wrapper<Widget>& item) {
		return &item.get() == &widget;
	});
	if (it == m_children.end()) {
		throw std::runtime_error("Widget::remove_child(): widget not found!");
	}
	
	m_children.erase(it);
}

void Widget::remove_child_at(int index) {
	if (index < 0 || index >= (int) m_children.size())
		throw std::runtime_error("Widget::remove_child_at(): out of bounds!");
	m_children.erase(m_children.begin() + index);
}

void Widget::clear_children() {
	m_children.clear(); // maybe managed somewhere else
}

void Widget::shed_children() {
	// Continue removing children until m_children is empty
	while (!m_children.empty()) {
		// Get the last child
		Widget& child = m_children.back();
				
		// Recursively shed the child's children
		child.shed_children();
		
		// Remove the child from m_children
		m_children.pop_back();

	}
}

int Widget::child_index(Widget& widget) const {
	auto it = std::find_if(m_children.begin(), m_children.end(), [&widget](std::reference_wrapper<Widget> item){
		return &item.get() == &widget;
	});
	if (it == m_children.end())
		return -1;
	return (int)(it - m_children.begin());
}

Window* Widget::window() {
	std::optional<std::reference_wrapper<Widget>> widget = *this;
	
	while (widget != std::nullopt) {
		auto win = dynamic_cast<Window*>(&widget->get());
		if (win)
			return win;
		widget = widget->get().parent();
	}
	return nullptr;
}

Screen& Widget::screen() {
	return m_screen->get(); // Directly return the cached screen pointer
}

void Widget::request_focus() {
	screen().update_focus(*this);
}

void Widget::draw(NVGcontext *ctx) {
#if defined(NANOGUI_SHOW_WIDGET_BOUNDS)
	nvgStrokeWidth(ctx, 1.0f);
	nvgBeginPath(ctx);
	nvgRect(ctx, m_pos.x() - 0.5f, m_pos.y() - 0.5f,
			m_size.x() + 1, m_size.y() + 1);
	nvgStrokeColor(ctx, nvgRGBA(255, 0, 0, 255));
	nvgStroke(ctx);
#endif
	
	if (m_children.empty())
		return;
	
	nvgTranslate(ctx, m_pos.x(), m_pos.y());
	for (auto child : m_children) {
		if (!child.get().visible())
			continue;
#if !defined(NANOGUI_SHOW_WIDGET_BOUNDS)
		nvgSave(ctx);
		nvgIntersectScissor(ctx, child.get().m_pos.x(), child.get().m_pos.y(),
							child.get().m_size.x(), child.get().m_size.y());
#endif
		
		child.get().draw(ctx);
		
#if !defined(NANOGUI_SHOW_WIDGET_BOUNDS)
		nvgRestore(ctx);
#endif
	}
	nvgTranslate(ctx, -m_pos.x(), -m_pos.y());
}

void Widget::set_parent(Widget& parent) {
	m_parent = parent;
	m_screen = parent.screen(); // Update screen pointer
}

void Widget::set_screen(Screen& screen) {
	m_screen = std::ref(screen);
	// Propagate to children
	for (auto child : m_children) {
		child.get().set_screen(screen);
	}
}

NAMESPACE_END(nanogui)
