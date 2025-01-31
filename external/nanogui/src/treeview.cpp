#include <nanogui/treeview.h>
#include <nanogui/treeviewitem.h>
#include <nanogui/opengl.h>

NAMESPACE_BEGIN(nanogui)

TreeView::TreeView(Widget& parent)
    : Widget(std::make_optional<std::reference_wrapper<Widget>>(parent)), m_selected_item(nullptr) {
}

void TreeView::draw(NVGcontext *ctx) {
    Widget::draw(ctx);
}

void TreeView::add_item(std::shared_ptr<TreeViewItem> item) {
    m_items.push_back(item);
}

std::shared_ptr<TreeViewItem> TreeView::add_node(const std::string &caption, std::function<void()> callback) {
	auto node = std::make_shared<TreeViewItem>(*this, *this, caption, callback);
    add_item(node);
    return node;
}

void TreeView::clear() {
    for (auto item : m_items) {
        remove_child(*item);
    }
    m_items.clear();
	
	m_selected_item = nullptr;
}

void TreeView::set_selected(TreeViewItem* item) {
    if (m_selected_item) {
        m_selected_item->set_selected(false);
    }
    
	if (item) {
		m_selected_item = item;
		
		m_selected_item->set_selected(true);
	}
}

NAMESPACE_END(nanogui)
