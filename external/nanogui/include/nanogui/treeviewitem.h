#pragma once

#include <nanogui/nanogui.h>
#include <functional>
#include <vector>

NAMESPACE_BEGIN(nanogui)

class TreeView;

class TreeViewItem : public Widget {
public:
    TreeViewItem(Widget& parent, TreeView& tree, const std::string &caption, std::function<void()> callback);

    virtual void draw(NVGcontext *ctx) override;
    virtual Vector2i preferred_size(NVGcontext *ctx) override;
    virtual bool mouse_button_event(const Vector2i &p, int button, bool down, int modifiers) override;

	std::shared_ptr<TreeViewItem> add_node(const std::string &caption, std::function<void()> callback);

	void set_selected(bool selected) {
		m_selected = selected;
		if (m_selected) {
			m_selection_callback();
		}
	}
    void set_expanded(bool expanded) { m_expanded = expanded; }
    bool selected() const { return m_selected; }
    bool expanded() const { return m_expanded; }

private:
	TreeView& m_tree;
    std::string m_caption;
    bool m_selected;
    bool m_expanded;
    std::vector<std::shared_ptr<TreeViewItem>> m_children;
    
    std::function<void()> m_selection_callback;
};


NAMESPACE_END(nanogui)

