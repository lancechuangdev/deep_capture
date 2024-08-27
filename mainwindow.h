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
    Gtk::TreeView *m_camTreeView;
    Gtk::Button *m_discoverBtn;
    Gtk::Button *m_connectBtn;
    Gtk::Button *m_startBtn;
    Gtk::Button *m_stopBtn;
    Gtk::Button *m_disconnectBtn;

    // Signal handlers:
    void onDiscoverClicked();
    void onConnectClicked();
    void onStartClicked();
    void onStopClicked();
    void onDisconnectClicked();
    void onTreeviewSelectionChanged();

private:
    Glib::RefPtr<Gtk::Builder> m_builder;
    Glib::RefPtr<Gtk::ListStore> m_camListStore;
    MV_CC_DEVICE_INFO_LIST m_camList;
    void* m_selectedCam;
    CamColumns m_camcols;
};

#endif // DEEP_SCAN_MAINWINDOW_H
