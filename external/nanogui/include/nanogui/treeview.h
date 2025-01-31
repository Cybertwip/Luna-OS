#pragma once

#include <nanogui/nanogui.h>
#include <functional>
#include <vector>

NAMESPACE_BEGIN(nanogui)

class TreeViewItem;

class TreeView : public Widget {
public:
    TreeView(Widget& parent);

    virtual void draw(NVGcontext *ctx) override;

    void add_item(std::shared_ptr<TreeViewItem> item);
	std::shared_ptr<TreeViewItem> add_node(const std::string &caption, std::function<void()> callback);
    void clear();
    
    void set_selected(TreeViewItem* item);

private:
    std::vector<std::shared_ptr<TreeViewItem>> m_items;
	TreeViewItem* m_selected_item;
};

NAMESPACE_END(nanogui)

