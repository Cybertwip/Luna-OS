/*
 src/label.cpp -- Text label with an arbitrary font, color, size, and alignment
 
 NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
 The widget drawing code is based on the NanoVG demo application
 by Mikko Mononen.
 
 All rights reserved. Use of this source code is governed by a
 BSD-style license that can be found in the LICENSE.txt file.
 */

#include <nanogui/label.h>
#include <nanogui/theme.h>
#include <nanogui/opengl.h>

NAMESPACE_BEGIN(nanogui)

Label::Label(Widget& parent,  const std::string &caption, const std::string &font, int font_size, Alignment alignment)
: Widget(std::make_optional<std::reference_wrapper<Widget>>(parent)), m_caption(caption), m_font(font), m_alignment(alignment) {
	m_font_size = theme().m_standard_font_size;
	m_color = theme().m_text_color;

	if (font_size >= 0) m_font_size = font_size;
}

Vector2i Label::preferred_size(NVGcontext *ctx) {
	if (m_caption.empty())
		return Vector2i(0);
	nvgFontFace(ctx, m_font.c_str());
	nvgFontSize(ctx, font_size());
	
	// Determine text alignment
	int align = NVG_ALIGN_LEFT;
	if (m_alignment == Alignment::Center)
		align = NVG_ALIGN_CENTER;
	else if (m_alignment == Alignment::Right)
		align = NVG_ALIGN_RIGHT;
	
	if (m_fixed_size.x() > 0) {
		float bounds[4];
		nvgTextAlign(ctx, align | NVG_ALIGN_TOP);
		nvgTextBoxBounds(ctx, m_pos.x(), m_pos.y(), m_fixed_size.x(), m_caption.c_str(), nullptr, bounds);
		return Vector2i(m_fixed_size.x(), static_cast<int>(bounds[3] - bounds[1]));
	} else {
		nvgTextAlign(ctx, align | NVG_ALIGN_MIDDLE);
		float width = nvgTextBounds(ctx, 0, 0, m_caption.c_str(), nullptr, nullptr);
		return Vector2i(static_cast<int>(width + 2), font_size());
	}
}

void Label::draw(NVGcontext *ctx) {
	Widget::draw(ctx);
	nvgFontFace(ctx, m_font.c_str());
	nvgFontSize(ctx, font_size());
	nvgFillColor(ctx, m_color);
	
	// Determine text alignment
	int align = NVG_ALIGN_LEFT;
	if (m_alignment == Alignment::Center)
		align = NVG_ALIGN_CENTER;
	else if (m_alignment == Alignment::Right)
		align = NVG_ALIGN_RIGHT;
	
	if (m_fixed_size.x() > 0) {
		nvgTextAlign(ctx, align | NVG_ALIGN_TOP);
		nvgTextBox(ctx, m_pos.x(), m_pos.y(), m_fixed_size.x(), m_caption.c_str(), nullptr);
	} else {
		nvgTextAlign(ctx, align | NVG_ALIGN_MIDDLE);
		nvgText(ctx, m_pos.x(), m_pos.y() + m_size.y() * 0.5f, m_caption.c_str(), nullptr);
	}
}

NAMESPACE_END(nanogui)
