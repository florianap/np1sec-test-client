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

/* purple headers */
#include "prefs.h"

/* plugin headers */
#include "room.h"

namespace np1sec_plugin {

class PluginToggleButton {
public:
    PluginToggleButton(PurpleConversation*);

    std::unique_ptr<Room> room;

    ~PluginToggleButton();

private:
    static void on_click(GtkWidget*, PluginToggleButton*);

    void enable();
    void disable();

    std::string prefs_str();

private:
    PurpleConversation* _conv;
    PidginConversation* _gtkconv;
    GtkWidget* _button = nullptr;
    bool _toggleable = true;
};

//------------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------------
inline
PluginToggleButton::PluginToggleButton(PurpleConversation* conv)
    : _conv(conv)
    , _gtkconv(PIDGIN_CONVERSATION(conv))
{
    if (_toggleable) {
        if (! purple_prefs_exists("/np1sec")) {
            purple_prefs_add_none("/np1sec");
        }

        if (! purple_prefs_exists("/np1sec/conversation/")) {
            purple_prefs_add_none("/np1sec/conversation");
        }

        auto s = std::string("/np1sec/conversation/") + _conv->name;

        if (! purple_prefs_exists(s.c_str())) {
            purple_prefs_add_none(s.c_str());
        }

        _button = gtk_button_new_with_label("Toggle np1sec");
        gtk_box_pack_start(GTK_BOX(_gtkconv->infopane_hbox), _button, FALSE, FALSE, 0);

	    if (purple_prefs_get_bool(prefs_str().c_str())) {
            enable();
        }
    }
    else {
        _button = gtk_button_new_with_label("Go secure");
        gtk_box_pack_start(GTK_BOX(_gtkconv->infopane_hbox), _button, FALSE, FALSE, 0);
    }

    gtk_signal_connect(GTK_OBJECT(_button), "clicked"
            , GTK_SIGNAL_FUNC(on_click), this);

    gtk_widget_show(_button);
}

inline
void PluginToggleButton::on_click(GtkWidget*, PluginToggleButton* self)
{
    //auto conv = self->_conv;
    //auto pc = purple_conversation_new(PURPLE_CONV_TYPE_CHAT, conv->account, conv->name);
    //int id = purple_conv_chat_get_id(PURPLE_CONV_CHAT(conv));
	//purple_conv_chat_set_id(PURPLE_CONV_CHAT(pc), id);
    ////self->_conv->ui_ops->create_conversation(pc);

    return;
    if (self->_toggleable) {
        if (self->room) {
            purple_prefs_set_bool(self->prefs_str().c_str(), false);
            self->disable();
        }
        else {
            purple_prefs_set_bool(self->prefs_str().c_str(), true);
            self->enable();
        }
    }
    else {
        gtk_container_remove(GTK_CONTAINER(self->_gtkconv->infopane_hbox), self->_button);
        self->_button = nullptr;
        self->enable();
    }
}

inline
std::string PluginToggleButton::prefs_str() {
    return std::string("/np1sec/conversation/") + _conv->name + "/enabled";
}

void PluginToggleButton::enable()
{
    room.reset(new Room(_conv));
    room->start();
}

inline
void PluginToggleButton::disable()
{
    room.reset();
}

inline
PluginToggleButton::~PluginToggleButton()
{
    if (_button) {
        gtk_container_remove(GTK_CONTAINER(_gtkconv->infopane_hbox), _button);
    }
}

} // np1sec_plugin namespace
