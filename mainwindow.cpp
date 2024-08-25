#include "mainwindow.h"
#include <iostream>

void __stdcall GrabImageCallBack(unsigned char *pData, MV_FRAME_OUT_INFO_EX *pFrameInfo, void *pUser)
{
    if (pFrameInfo)
    {
        std::cout << "GetOneFrame, Width: " << pFrameInfo->nWidth << ", Height: " << pFrameInfo->nHeight << ", Frame num: " << pFrameInfo->nFrameNum << std::endl;
    }
}

MainWindow::MainWindow(BaseObjectType *obj, Glib::RefPtr<Gtk::Builder> const &refBuilder)
    : Gtk::Window(obj), m_builder(refBuilder)
{
    m_builder->get_widget("camera_list", m_cams_tv);
    // Get the button by ID and connect the signal handler.
    m_builder->get_widget("discover_btn", m_discover_btn);
    m_builder->get_widget("connect_btn", m_connect_btn);
    m_builder->get_widget("start_btn", m_start_btn);
    m_builder->get_widget("stop_btn", m_stop_btn);
    m_builder->get_widget("disconnect_btn", m_disconnect_btn);

    // Create the ListStore, with 'm_camcols' as the column model
    m_cam_list_store = Gtk::ListStore::create(m_camcols);

    // Set the ListStore as the model for the cams TreeView
    m_cams_tv->set_model(m_cam_list_store);

    // Append columns to the TreeView
    m_cams_tv->append_column("Model", m_camcols.col_model);
    m_cams_tv->append_column("Friendly Name", m_camcols.col_friendly_name);
    m_cams_tv->append_column("IP Address", m_camcols.col_ip);
    m_cams_tv->append_column("State", m_camcols.col_state);

    if (m_discover_btn)
    {
        m_discover_btn->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_discover_clicked));
    }
    if (m_connect_btn)
    {
        m_connect_btn->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_connect_clicked));
    }
    if (m_start_btn)
    {
        m_start_btn->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_start_clicked));
    }
    if (m_stop_btn)
    {
        m_stop_btn->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_stop_clicked));
    }
    if (m_disconnect_btn)
    {
        m_disconnect_btn->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_disconnect_clicked));
    }
}

MainWindow::~MainWindow()
{
}

std::string getIpV4AddressString(uint32_t ip) {
    std::ostringstream ipStream;
    for (int i = 0; i < 4; ++i) {
        if (i > 0) {
            ipStream << ".";
        }
        ipStream << ((ip >> (24 - 8 * i)) & 0xFF);
    }
    return ipStream.str();
}

void MainWindow::on_discover_clicked()
{
    // Clear the TreeView before adding new data
    m_cam_list_store->clear();

    do
    {
        memset(&m_stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

        // enum device
        int nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDeviceList);
        if (MV_OK != nRet)
        {
            std::cout << "MV_CC_EnumDevices fail! Error code: " << nRet << std::endl;
            break;
        }

        if (m_stDeviceList.nDeviceNum > 0)
        {
            for (unsigned int i = 0; i < m_stDeviceList.nDeviceNum; i++)
            {
                std::cout << "device: " << i << std::endl;
                MV_CC_DEVICE_INFO *pDeviceInfo = m_stDeviceList.pDeviceInfo[i];
                if (NULL == pDeviceInfo)
                {
                    break;
                }

                Gtk::TreeModel::Row row = *(m_cam_list_store->append());

                if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
                {
                    row[m_camcols.col_model] = Glib::ustring(reinterpret_cast<const char*>(pDeviceInfo->SpecialInfo.stGigEInfo.chModelName));
                    row[m_camcols.col_friendly_name] = Glib::ustring(reinterpret_cast<const char*>(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName));
                    row[m_camcols.col_ip] = getIpV4AddressString(pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp);
                    row[m_camcols.col_state] = "Idle"; // Replace with actual state if available
                }
                else if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
                {
                    row[m_camcols.col_model] = Glib::ustring(reinterpret_cast<const char*>(pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName));
                    row[m_camcols.col_friendly_name] = Glib::ustring(reinterpret_cast<const char*>(pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName));
                    row[m_camcols.col_state] = "Idle"; // Replace with actual state if available
                }
            }
        }
        else
        {
            std::cout << "Find No Devices!" << std::endl;
            break;
        }
    } while (false);
}

void MainWindow::on_connect_clicked()
{
    do
    {
        // Select the first camera
        unsigned int nIndex = 0;

        // Select device and create handle
        int nRet = MV_CC_CreateHandle(&m_deviceHandle, m_stDeviceList.pDeviceInfo[nIndex]);
        if (nRet != MV_OK)
        {
            std::cout << "MV_CC_CreateHandle fail! Error code: " << nRet << std::endl;
            break;
        }

        // Connect device
        nRet = MV_CC_OpenDevice(m_deviceHandle);
        if (nRet != MV_OK)
        {
            std::cout << "MV_CC_OpenDevice fail! Error code: " << nRet << std::endl;
            break;
        }

        // Detect network optimal package size(It only works for the GigE camera)
        if (m_stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
        {
            int nPacketSize = MV_CC_GetOptimalPacketSize(m_deviceHandle);
            if (nPacketSize > 0)
            {
                nRet = MV_CC_SetIntValue(m_deviceHandle, "GevSCPSPacketSize", nPacketSize);
                if (nRet != MV_OK)
                {
                    std::cout << "Set Packet Size fail. Error code: " << nRet << std::endl;
                }
            }
            else
            {
                std::cout << "Get Packet Size fail. Error code: " << nRet << std::endl;
            }
        }

        // Turn trigger mode off
        nRet = MV_CC_SetEnumValue(m_deviceHandle, "TriggerMode", 0);
        if (MV_OK != nRet)
        {
            std::cout << "MV_CC_SetTriggerMode fail. Error code: " << nRet << std::endl;
            break;
        }

        // Register image callback
        nRet = MV_CC_RegisterImageCallBackEx(m_deviceHandle, GrabImageCallBack, m_deviceHandle);
        if (MV_OK != nRet)
        {
            std::cout << "MV_CC_RegisterImageCallBackEx fail. Error code: " << nRet << std::endl;
            break;
        }
    } while (false);
}

void MainWindow::on_start_clicked()
{
    // Start grab images
    int nRet = MV_CC_StartGrabbing(m_deviceHandle);
    if (MV_OK != nRet)
    {
        std::cout << "MV_CC_StartGrabbing fail. Error code: " << nRet << std::endl;
    }
}

void MainWindow::on_stop_clicked()
{
    int nRet = MV_CC_StopGrabbing(m_deviceHandle);
    if (MV_OK != nRet)
    {
        std::cout << "MV_CC_StopGrabbing fail. Error code: " << nRet << std::endl;
    }
}

void MainWindow::on_disconnect_clicked()
{
    int nRet = MV_CC_CloseDevice(m_deviceHandle);
    if (MV_OK != nRet)
    {
        std::cout << "MV_CC_CloseDevice fail. Error code: " << nRet << std::endl;
    }

    // destroy handle
    nRet = MV_CC_DestroyHandle(m_deviceHandle);
    if (MV_OK != nRet)
    {
        std::cout << "MV_CC_DestroyHandle fail. Error code: " << nRet << std::endl;
    }
    m_deviceHandle = NULL;
}