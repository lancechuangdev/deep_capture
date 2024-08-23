#include "mainwindow.h"
#include <iostream>
#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <glibmm/fileutils.h> // For Glib::FileError
#include <glibmm/markup.h>    // For Glib::MarkupError

int main(int argc, char **argv)
{
    auto app = Gtk::Application::create(argc, argv, "org.gtkmm.example");
    auto builder = Gtk::Builder::create();

    try
    {
        builder->add_from_file("../ui.glade");
    }
    catch (const Glib::FileError &ex)
    {
        std::cerr << "FileError: " << ex.what() << std::endl;
        return 1;
    }
    catch (const Glib::MarkupError &ex)
    {
        std::cerr << "MarkupError: " << ex.what() << std::endl;
        return 1;
    }
    catch (const Gtk::BuilderError &ex)
    {
        std::cerr << "BuilderError: " << ex.what() << std::endl;
        return 1;
    }

    // Initialize MV camera control.
    int nRet = MV_CC_Initialize();
    if (nRet != MV_OK)
    {
        std::cout << "Initialize SDK fail!" << std::endl;
    }

    // Load top level window from glade.
    MainWindow *wnd = nullptr;
    builder->get_widget_derived("root", wnd);

    // Shows the window and returns when it is closed.
    return app->run(*wnd);
}