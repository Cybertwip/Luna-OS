/*
 src/popup.cpp -- Simple popup widget which is attached to another given
 window (can be nested)
 
 NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
 The widget drawing code is based on the NanoVG demo application
 by Mikko Mononen.
 
 All rights reserved. Use of this source code is governed by a
 BSD-style license that can be found in the LICENSE.txt file.
 */

#include <nanogui/popup.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>

NAMESPACE_BEGIN(nanogui)

Popup::Popup(std::optional<std::reference_wrapper<Widget>> parent,  Window* parent_window)
: Window(parent, ""), m_parent_window(parent_window), m_anchor_pos(Vector2i(0)),
m_anchor_offset(30), m_anchor_size(15), m_side(Side::Right) { }
void Popup::perform_layout(NVGcontext *ctx) {
	Widget::perform_layout(ctx);
	
	if (m_side == Side::Left)
		m_anchor_pos.x() -= size().x();
	
	int max_width = 0;
	m_size.y() = 0;
	
	// First pass: Calculate total height (including spacing) and maximum width
	const float spacing_factor = 1.125f; // Factor to determine spacing between children
	for (auto &child : m_children) {
		m_size.y() += static_cast<int>(child.get().height() * spacing_factor);
		
		if (child.get().width() > max_width) {
			max_width = child.get().width();
		}
	}
	
	m_size.x() = max_width;
	
	int offset = 0;
	
	// Second pass: Adjust positions of child widgets
	for (auto &child : m_children) {
		// Set the child width to 95% of the maximum width
		child.get().set_width(static_cast<int>(max_width * 0.95f));
		
		// Calculate horizontal position to center the child within the Popup
		int x = (m_size.x() - child.get().width()) / 2;
		
		// Set the vertical position based on the current offset
		child.get().set_position(Vector2i(x, offset));
		
		// Increment the offset by the child's height multiplied by the spacing factor
		offset += static_cast<int>(child.get().height() * spacing_factor);
	}
}

void Popup::refresh_relative_placement() {
	if (!m_parent_window)
		return;
	m_parent_window->refresh_relative_placement();
	m_visible &= m_parent_window->visible_recursive();
	m_pos = m_parent_window->position() + m_anchor_pos - Vector2i(0, m_anchor_offset);
}

void Popup::draw(NVGcontext* ctx) {
	refresh_relative_placement();
	
	if (!m_visible)
		return;
	
	int ds = theme().m_window_drop_shadow_size,
	cr = theme().m_window_corner_radius;
	
	nvgSave(ctx);
	nvgResetScissor(ctx);
	
	/* Draw a drop shadow */
	NVGpaint shadow_paint = nvgBoxGradient(
										   ctx, m_pos.x(), m_pos.y(), m_size.x(), m_size.y(), cr * 2, ds * 2,
										   theme().m_drop_shadow, theme().m_transparent);
	
	nvgBeginPath(ctx);
	nvgRect(ctx, m_pos.x() - ds, m_pos.y() - ds, m_size.x() + 2 * ds, m_size.y() + 2 * ds);
	nvgRoundedRect(ctx, m_pos.x(), m_pos.y(), m_size.x(), m_size.y(), cr);
	nvgPathWinding(ctx, NVG_HOLE);
	nvgFillPaint(ctx, shadow_paint);
	nvgFill(ctx);
	
	/* Draw window */
	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, m_pos.x(), m_pos.y(), m_size.x(), m_size.y(), cr);
	
	Vector2i base = m_pos + Vector2i(0, m_anchor_offset);
	int sign = -1;
	if (m_side == Side::Left) {
		base.x() += m_size.x();
		sign = 1;
	}
	
	nvgMoveTo(ctx, base.x() + m_anchor_size * sign, base.y());
	nvgLineTo(ctx, base.x() - 1 * sign, base.y() - m_anchor_size);
	nvgLineTo(ctx, base.x() - 1 * sign, base.y() + m_anchor_size);
	
	nvgFillColor(ctx, theme().m_window_popup);
	nvgFill(ctx);
	nvgRestore(ctx);
	
	Widget::draw(ctx);
}

NAMESPACE_END(nanogui)
