#ifndef DEEP_SCAN_MAINWINDOW_H
#define DEEP_SCAN_MAINWINDOW_H

#include <gtkmm/button.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>
#include <gtkmm/label.h>
#include <gtkmm/builder.h>
#include "MvCameraControl.h"
#include "camcols.h"
#include <iostream>
#include <thread>
#include <future>
#include <filesystem> 

class MainWindow : public Gtk::Window
{
public:
    MainWindow(BaseObjectType *obj, Glib::RefPtr<Gtk::Builder> const &refBuilder);
    virtual ~MainWindow();
    void* m_selectedCam;
    std::string m_folderPath;

protected:

    // Member widgets:
    Gtk::TreeView *m_camTreeView;
    Gtk::Button *m_discoverBtn;
    Gtk::Button *m_connectBtn;
    Gtk::Button *m_startBtn;
    Gtk::Button *m_stopBtn;
    Gtk::Button *m_disconnectBtn;
    Gtk::Label *m_exposureTimeLbl;
    Gtk::Label *m_frameRateLbl;
    Gtk::Label *m_widthLbl;
    Gtk::Label *m_heightLbl;
    Gtk::Label *m_gainLbl;
    Gtk::FileChooserButton *m_pickerFcb;
    Gtk::SpinButton *m_captureDurationSb; 
    Gtk::SpinButton *m_captureRateSb;

    // Signal handlers:
    void onDiscoverClicked();
    void onConnectClicked();
    void onStartClicked();
    void onStopClicked();
    void onDisconnectClicked();
    void onTreeviewSelectionChanged();
    void onFolderSelected();

private:
    Glib::RefPtr<Gtk::Builder> m_builder;
    Glib::RefPtr<Gtk::ListStore> m_camListStore;
    MV_CC_DEVICE_INFO_LIST m_camList;
    CamColumns m_camcols;
    int m_captureDuration;
    int m_captureRate;
    void populateDeviceSettings();
    void clearDeviceSettings();
    void onCaptureDurationChanged();
};

#endif // DEEP_SCAN_MAINWINDOW_H
