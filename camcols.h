#include <gtkmm.h>

class CamColumns : public Gtk::TreeModel::ColumnRecord {
public:
    CamColumns() {
        add(col_model);
        add(col_friendly_name);
        add(col_ip);
        add(col_state);
    }

    Gtk::TreeModelColumn<Glib::ustring> col_model;
    Gtk::TreeModelColumn<Glib::ustring> col_friendly_name;
    Gtk::TreeModelColumn<Glib::ustring> col_ip;
    Gtk::TreeModelColumn<Glib::ustring> col_state;
};