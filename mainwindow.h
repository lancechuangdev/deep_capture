#ifndef DEEP_SCAN_MAINWINDOW_H
#define DEEP_SCAN_MAINWINDOW_H

#include <gtkmm/button.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>
#include <gtkmm/builder.h>
#include "MvCameraControl.h"
#include "camcols.h"

class MainWindow : public Gtk::Window
{
public:
    MainWindow(BaseObjectType *obj, Glib::RefPtr<Gtk::Builder> const &refBuilder);
    virtual ~MainWindow();

protected:
    // Member widgets:
    Gtk::TreeView *m_cams_tv;
    Gtk::Button *m_discover_btn;
    Gtk::Button *m_connect_btn;
    Gtk::Button *m_start_btn;
    Gtk::Button *m_stop_btn;
    Gtk::Button *m_disconnect_btn;

    // Signal handlers:
    void on_discover_clicked();
    void on_connect_clicked();
    void on_start_clicked();
    void on_stop_clicked();
    void on_disconnect_clicked();

private:
    Glib::RefPtr<Gtk::Builder> m_builder;
    Glib::RefPtr<Gtk::ListStore> m_cam_list_store;
    MV_CC_DEVICE_INFO_LIST m_stDeviceList;
    void* m_deviceHandle;
    CamColumns m_camcols;
};

#endif // DEEP_SCAN_MAINWINDOW_H
