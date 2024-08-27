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
    // Get the button by ID and connect the signal handler.
    m_builder->get_widget("discover_btn", m_discoverBtn);
    m_builder->get_widget("connect_btn", m_connectBtn);
    m_builder->get_widget("start_btn", m_startBtn);
    m_builder->get_widget("stop_btn", m_stopBtn);
    m_builder->get_widget("disconnect_btn", m_disconnectBtn);

    if (m_discoverBtn)
    {
        m_discoverBtn->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onDiscoverClicked));
    }
    if (m_connectBtn)
    {
        m_connectBtn->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onConnectClicked));
    }
    if (m_startBtn)
    {
        m_startBtn->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onStartClicked));
    }
    if (m_stopBtn)
    {
        m_stopBtn->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onStopClicked));
    }
    if (m_disconnectBtn)
    {
        m_disconnectBtn->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::onDisconnectClicked));
    }

    m_builder->get_widget("camera_list", m_camTreeView);

    // Connect the signal to a handler function
    Glib::RefPtr<Gtk::TreeSelection> selection = m_camTreeView->get_selection();
    selection->signal_changed().connect(sigc::mem_fun(*this, &MainWindow::onTreeviewSelectionChanged));

    // Create the ListStore, with 'm_camcols' as the column model
    m_camListStore = Gtk::ListStore::create(m_camcols);

    // Set the ListStore as the model for the cams TreeView
    m_camTreeView->set_model(m_camListStore);

    // Append columns to the TreeView
    m_camTreeView->append_column("Model", m_camcols.col_model);
    m_camTreeView->append_column("Friendly Name", m_camcols.col_friendly_name);
    m_camTreeView->append_column("IP Address", m_camcols.col_ip);

    // Device settings
    m_builder->get_widget("exposure_lbl", m_exposureTimeLbl);
    m_builder->get_widget("frame_rate_lbl", m_frameRateLbl);
    m_builder->get_widget("width_lbl", m_widthLbl);
    m_builder->get_widget("height_lbl", m_heightLbl);
    m_builder->get_widget("gain_lbl", m_gainLbl);

    // Image Acquiring
    m_builder->get_widget("picker_fcb", m_pickerFcb);
    if (m_pickerFcb)
    {
        // Connect to the file-set signal
        m_pickerFcb->signal_file_set().connect(sigc::mem_fun(*this, &MainWindow::onFolderSelected));
    }

    m_builder->get_widget("capture_duration_sb", m_captureDurationSb);
    if (m_captureDurationSb) 
    {
        m_captureDurationSb->signal_value_changed().connect(sigc::mem_fun(*this, &MainWindow::onCaptureDurationChanged));
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

int getSelectedCamIndex(Gtk::TreeView *camTreeView, Glib::RefPtr<Gtk::ListStore> camsListStore)
{
    int index = 0;
    Glib::RefPtr<Gtk::TreeSelection> selection = camTreeView->get_selection();
    Gtk::TreeModel::iterator iter = selection->get_selected();
    if(iter)
    {
        Gtk::TreeModel::Children::iterator it;
        Gtk::TreeModel::Children children = camsListStore->children();

        for(it = children.begin(); it != children.end(); ++it)
        {
            if(it == iter)
            {
                return index;
            }
            ++index;
        }
    }
    return -1;
}

void MainWindow::onTreeviewSelectionChanged() {
    int nIndex = getSelectedCamIndex(m_camTreeView, m_camListStore);
    if (nIndex < 0) 
    {
        std::cout << "No camera was selected." << std::endl;
    }
    else
    {
        MV_CC_DEVICE_INFO* pSelectedCam = m_camList.pDeviceInfo[nIndex];
        int nRet = MV_CC_CreateHandle(&m_selectedCam, pSelectedCam);
        if (nRet != MV_OK)
        {
            std::cout << "MV_CC_CreateHandle fail! Error code: " << nRet << std::endl;
        }
    }
}


void MainWindow::onDiscoverClicked()
{
    // Clear the TreeView before adding new data
    m_camListStore->clear();

    do
    {
        memset(&m_camList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));

        // enum device
        int nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_camList);
        if (MV_OK != nRet)
        {
            std::cout << "MV_CC_EnumDevices fail! Error code: " << nRet << std::endl;
            break;
        }

        if (m_camList.nDeviceNum > 0)
        {
            for (unsigned int i = 0; i < m_camList.nDeviceNum; i++)
            {
                std::cout << "device: " << i << std::endl;
                MV_CC_DEVICE_INFO *pDeviceInfo = m_camList.pDeviceInfo[i];
                if (NULL == pDeviceInfo)
                {
                    break;
                }

                Gtk::TreeModel::Row row = *(m_camListStore->append());

                if (pDeviceInfo->nTLayerType == MV_GIGE_DEVICE)
                {
                    row[m_camcols.col_model] = Glib::ustring(reinterpret_cast<const char*>(pDeviceInfo->SpecialInfo.stGigEInfo.chModelName));
                    row[m_camcols.col_friendly_name] = Glib::ustring(reinterpret_cast<const char*>(pDeviceInfo->SpecialInfo.stGigEInfo.chUserDefinedName));
                    row[m_camcols.col_ip] = getIpV4AddressString(pDeviceInfo->SpecialInfo.stGigEInfo.nCurrentIp);
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

void MainWindow::onConnectClicked()
{
    int nIndex = getSelectedCamIndex(m_camTreeView, m_camListStore);

    // Create device handler if needed
    if (!m_selectedCam)
    {
        if (nIndex < 0) 
        {
            std::cout << "No camera was selected." << std::endl;
            return;
        }
        else
        {
            MV_CC_DEVICE_INFO* pSelectedCam = m_camList.pDeviceInfo[nIndex];
            int nRet = MV_CC_CreateHandle(&m_selectedCam, pSelectedCam);
            if (nRet != MV_OK)
            {
                std::cout << "MV_CC_CreateHandle fail! Error code: " << nRet << std::endl;
                return;
            }
        }
    }

    // Connect device
    int nRet = MV_CC_OpenDevice(m_selectedCam);
    if (nRet != MV_OK)
    {
        std::cout << "MV_CC_OpenDevice fail! Error code: " << nRet << std::endl;
        return;
    }

    // Detect network optimal package size(It only works for the GigE camera)
    if (m_camList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
    {
        int nPacketSize = MV_CC_GetOptimalPacketSize(m_selectedCam);
        if (nPacketSize > 0)
        {
            nRet = MV_CC_SetIntValue(m_selectedCam, "GevSCPSPacketSize", nPacketSize);
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
    nRet = MV_CC_SetEnumValue(m_selectedCam, "TriggerMode", 0);
    if (MV_OK != nRet)
    {
        std::cout << "MV_CC_SetTriggerMode fail. Error code: " << nRet << std::endl;
        return;
    }

    // Register image callback
    nRet = MV_CC_RegisterImageCallBackEx(m_selectedCam, GrabImageCallBack, m_selectedCam);
    if (MV_OK != nRet)
    {
        std::cout << "MV_CC_RegisterImageCallBackEx fail. Error code: " << nRet << std::endl;
        return;
    }

    populateDeviceSettings();
}

void MainWindow::populateDeviceSettings()
{
    // Exposure time
    MVCC_FLOATVALUE exposureTime = {0};
    int nRet = MV_CC_GetFloatValue(m_selectedCam, "ExposureTime", &exposureTime);
    if (MV_OK == nRet && m_exposureTimeLbl)
    {
        // Convert float to string
        std::ostringstream oss;
        oss << exposureTime.fCurValue;

        // Set the label text
        m_exposureTimeLbl->set_text(Glib::ustring(oss.str()));
    }
    else
    {
        std::cout << "Failed to get exposure time. Error code: " << nRet << std::endl;
    }

    // Resulting Frame Rate
    MVCC_FLOATVALUE frameRate = {0};
    nRet = MV_CC_GetFloatValue(m_selectedCam, "ResultingFrameRate", &frameRate);
    if (MV_OK == nRet && m_frameRateLbl)
    {
        // Convert float to string
        std::ostringstream oss;
        oss << frameRate.fCurValue;

        // Set the label text
        m_frameRateLbl->set_text(oss.str());
    }
    else
    {
        std::cout << "Failed to get frame rate. Error code: " << nRet << std::endl;
    }

    // Width
    MVCC_INTVALUE width = {0};
    nRet = MV_CC_GetIntValue(m_selectedCam, "Width", &width);
    if (MV_OK == nRet && m_widthLbl)
    {
        m_widthLbl->set_text(std::to_string(width.nCurValue));
    }
    else
    {
        std::cout << "Failed to get width. Error code: " << nRet << std::endl;
    }

    // Height
    MVCC_INTVALUE height = {0};
    nRet = MV_CC_GetIntValue(m_selectedCam, "Height", &height);
    if (MV_OK == nRet && m_heightLbl)
    {
        m_heightLbl->set_text(std::to_string(height.nCurValue));
    }
    else
    {
        std::cout << "Failed to get height. Error code: " << nRet << std::endl;
    }

    // Gain
    MVCC_FLOATVALUE gain = {0};
    nRet = MV_CC_GetFloatValue(m_selectedCam, "Gain", &gain);
    if (MV_OK == nRet && m_gainLbl)
    {
        // Convert float to string
        std::ostringstream oss;
        oss << gain.fCurValue;

        // Set the label text
        m_gainLbl->set_text(oss.str());
    }
    else
    {
        std::cout << "Failed to get gain. Error code: " << nRet << std::endl;
    }
}

void MainWindow::clearDeviceSettings()
{
    if (m_exposureTimeLbl)
    {
        m_exposureTimeLbl->set_text(std::string());
    }
    if (m_frameRateLbl)
    {
        m_frameRateLbl->set_text(std::string());
    }
    if (m_widthLbl)
    {
        m_widthLbl->set_text(std::string());
    }
    if (m_heightLbl)
    {
        m_heightLbl->set_text(std::string());
    }
    if (m_gainLbl)
    {
        m_gainLbl->set_text(std::string());
    }
}

void MainWindow::onStartClicked()
{
    // Start grab images
    int nRet = MV_CC_StartGrabbing(m_selectedCam);
    if (MV_OK != nRet)
    {
        std::cout << "MV_CC_StartGrabbing fail. Error code: " << nRet << std::endl;
    }
}

void MainWindow::onStopClicked()
{
    int nRet = MV_CC_StopGrabbing(m_selectedCam);
    if (MV_OK != nRet)
    {
        std::cout << "MV_CC_StopGrabbing fail. Error code: " << nRet << std::endl;
    }
}

void MainWindow::onDisconnectClicked()
{
    int nRet = MV_CC_CloseDevice(m_selectedCam);
    if (MV_OK != nRet)
    {
        std::cout << "MV_CC_CloseDevice fail. Error code: " << nRet << std::endl;
    }

    // destroy handle
    nRet = MV_CC_DestroyHandle(m_selectedCam);
    if (MV_OK != nRet)
    {
        std::cout << "MV_CC_DestroyHandle fail. Error code: " << nRet << std::endl;
    }

    m_selectedCam = nullptr;

    clearDeviceSettings();
}

void MainWindow::onFolderSelected() {
    m_folderPath = m_pickerFcb->get_filename();
}

void MainWindow::onCaptureDurationChanged() {
    m_captureDuration = m_captureDurationSb->get_value();
}