/**
 * Multiparty Off-the-Record Messaging library
 * Copyright (C) 2016, eQualit.ie
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of version 3 of the GNU Lesser General
 * Public License as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#pragma once

#include "defer.h"
#include "popup.h"

namespace np1sec_plugin {

//------------------------------------------------------------------------------
// ChannelList
//------------------------------------------------------------------------------
class ChannelList {
public:
    class Channel;
    class User;

public:
    ChannelList();

    ChannelList(ChannelList&&) = delete;
    ChannelList(const ChannelList&) = delete;
    ChannelList& operator=(const ChannelList&) = delete;
    ChannelList& operator=(ChannelList&&) = delete;

    ~ChannelList();

    GtkWidget* root_widget() { return GTK_WIDGET(_tree_view); }

private:
    void setup_callbacks(GtkTreeView* tree_view);

    static
    void on_double_click( GtkTreeView*
                        , GtkTreePath*
                        , GtkTreeViewColumn*
                        , ChannelList*);

    static
    gint on_button_pressed(GtkWidget*, GdkEventButton*, ChannelList*);

private:
    friend class Channel;
    friend class UserView;

    std::set<Channel*> _channels;

    GtkTreeView* _tree_view;
    GtkTreeStore* _tree_store;

    //            +--> Path in the tree
    //            |
    std::map<std::string, std::function<void()>> _double_click_callbacks;
    std::map<std::string, std::function<void(GdkEventButton*)>> _show_popup_callbacks;
};

//------------------------------------------------------------------------------
// ChannelList::Channel
//------------------------------------------------------------------------------
class ChannelList::Channel {
    friend class ChannelList;
    friend class ChannelList::User;

    enum
    {
      COL_NAME = 0,
      NUM_COLS
    };

public:
    Channel(ChannelList&, const std::string& name);

    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;

    Channel(Channel&&) = delete;
    Channel& operator=(Channel&&) = delete;

    std::function<void()> on_double_click;
    PopupActions popup_actions;

    const std::string& name() const { return _name; }

    ~Channel();

private:
    std::string path() const;

private:
    std::set<User*> _users;
    ChannelList* _channel_list;
    std::string _name;
    GtkTreeIter _iter;
};

//------------------------------------------------------------------------------
// ChannelList::User
//------------------------------------------------------------------------------
class ChannelList::User {
public:
    User(ChannelList::Channel&);
    ~User();

    User(const User&) = delete;
    User& operator=(const User&) = delete;

    User(User&&) = delete;
    User& operator=(User&&) = delete;

    PopupActions popup_actions;

    void set_text(const std::string&);

    std::function<void()> on_double_click;

private:
    void expand(GtkTreeIter& iter);

private:
    friend class Channel;

    ChannelList::Channel* _channel_view;
    GtkTreeIter _iter;
};

//------------------------------------------------------------------------------
// ChannelList implementation
//------------------------------------------------------------------------------
inline ChannelList::ChannelList()
{
    _tree_view = GTK_TREE_VIEW(gtk_tree_view_new());
    g_object_ref(_tree_view);

    gtk_widget_show(GTK_WIDGET(_tree_view));

    gtk_tree_view_insert_column_with_attributes( _tree_view
                                               , -1
                                               , "Channels"
                                               , gtk_cell_renderer_text_new()
                                               , "text", Channel::COL_NAME
                                               , NULL);

    _tree_store = gtk_tree_store_new(Channel::NUM_COLS, G_TYPE_STRING);
    gtk_tree_view_set_model(_tree_view, GTK_TREE_MODEL(_tree_store));

    setup_callbacks(_tree_view);
}

inline ChannelList::~ChannelList()
{
    for (auto* ch : _channels) {
        ch->_channel_list = nullptr;
    }

    g_object_unref(_tree_store);
    g_object_unref(_tree_view);
}

inline
void ChannelList::on_double_click( GtkTreeView *tree_view
                                 , GtkTreePath *path
                                 , GtkTreeViewColumn *column
                                 , ChannelList* v)
{
    gchar* c_str = gtk_tree_path_to_string(path);
    auto free_str = defer([c_str] { g_free(c_str); });

    auto cb_i = v->_double_click_callbacks.find(c_str);

    if (cb_i == v->_double_click_callbacks.end()) {
        return;
    }

    cb_i->second();
}

// Return TRUE if we handled it.
inline
gboolean ChannelList::on_button_pressed( GtkWidget*
                                       , GdkEventButton* event
                                       , ChannelList* v)
{
    /* Is right mouse button? */
    if (event->type == GDK_BUTTON_PRESS && event->button == 3) {
        GtkTreePath *path;
        gtk_tree_view_get_path_at_pos(v->_tree_view,
                                      event->x, event->y,
                                      &path, NULL, NULL, NULL);
        auto free_path = defer([path] { gtk_tree_path_free(path); });

        if (path == NULL)
            return FALSE;

        GtkTreeIter iter;
        gtk_tree_model_get_iter(GTK_TREE_MODEL(v->_tree_store), &iter, path);

        auto path_str = util::gtk::tree_iter_to_path(iter, v->_tree_store);

        auto action_i = v->_show_popup_callbacks.find(path_str);

        if (action_i == v->_show_popup_callbacks.end())
            return FALSE;

        action_i->second(event);

        return TRUE;
    }

    return FALSE;
}

inline
void ChannelList::setup_callbacks(GtkTreeView* tree_view)
{
    g_signal_connect(GTK_WIDGET(tree_view), "row-activated",
                     G_CALLBACK(on_double_click), this);

    g_signal_connect(G_OBJECT(tree_view), "button-press-event",
                     (GCallback) on_button_pressed, this);

    // // TODO: popup-menu
    // // http://scentric.net/tutorial/sec-selections-context-menus.html
    // g_signal_connect(G_OBJECT(tree_view), "popup-menu",
    //                  (GCallback) on_show_popup, this);
}

//------------------------------------------------------------------------------
// ChannelList::Channel Implementation
//------------------------------------------------------------------------------
inline ChannelList::Channel::Channel(ChannelList& list, const std::string& name)
    : _channel_list(&list)
    , _name(name)
{
    _channel_list->_channels.insert(this);

    gtk_tree_store_append(_channel_list->_tree_store, &_iter, NULL);
    gtk_tree_store_set(_channel_list->_tree_store, &_iter,
                       COL_NAME, _name.c_str(),
                       -1);

    auto p = path();

    _channel_list->_double_click_callbacks[p] = [this] {
        if (on_double_click) on_double_click();
    };

    _channel_list->_show_popup_callbacks[p] = [this] (GdkEventButton* e) {
        show_popup(e, popup_actions);
    };
}

inline
std::string ChannelList::Channel::path() const
{
    return util::gtk::tree_iter_to_path(_iter, _channel_list->_tree_store);
}

inline ChannelList::Channel::~Channel()
{
    for (auto* u : _users) {
        u->_channel_view = nullptr;
    }

    if (!_channel_list) return;
    _channel_list->_channels.erase(this);
    _channel_list->_double_click_callbacks.erase(path());
    gtk_tree_store_remove(_channel_list->_tree_store, &_iter);
}

//------------------------------------------------------------------------------
// ChannelList::User Implementation
//------------------------------------------------------------------------------
inline
ChannelList::User::User(ChannelList::Channel& channel)
    : _channel_view(&channel)
{
    assert(_channel_view->_channel_list);

    auto& cl = *_channel_view->_channel_list;
    auto* tree_store = cl._tree_store;

    gtk_tree_store_append(tree_store, &_iter, &_channel_view->_iter);

    expand(_iter);

    auto path = util::gtk::tree_iter_to_path(_iter, cl._tree_store);

    cl._show_popup_callbacks[path] = [this] (GdkEventButton* e) {
        show_popup(e, popup_actions);
    };

    cl._double_click_callbacks[path] = [this] {
        if (on_double_click) on_double_click();
    };
}

inline
ChannelList::User::~User() {

    if (!_channel_view) return;

    _channel_view->_users.erase(this);

    if (!_channel_view->_channel_list) return;

    auto cl = _channel_view->_channel_list;
    auto path = util::gtk::tree_iter_to_path(_iter, cl->_tree_store);

    gtk_tree_store_remove(cl->_tree_store, &_iter);

    cl->_double_click_callbacks.erase(path);
}

inline
void ChannelList::User::set_text(const std::string& txt)
{
    if (!_channel_view) return;
    assert(_channel_view->_channel_list);
    gtk_tree_store_set(_channel_view->_channel_list->_tree_store,
                       &_iter,
                       0, txt.c_str(), -1);
}

inline
void ChannelList::User::expand(GtkTreeIter& iter) {
    if (!_channel_view) return;
    assert(_channel_view->_channel_list);
    auto* rv = _channel_view->_channel_list;

    auto tree_store = rv->_tree_store;
    auto tree_view  = rv->_tree_view;

    auto model = GTK_TREE_MODEL(tree_store);

    gchar* path_str = gtk_tree_model_get_string_from_iter(model, &iter);
    auto free_path_str = defer([path_str] { g_free(path_str); });

    GtkTreePath* path = gtk_tree_path_new_from_string(path_str);
    auto free_path = defer([path] { gtk_tree_path_free(path); });

    gtk_tree_view_expand_to_path(tree_view, path);
}

} // np1sec_plugin namespace


