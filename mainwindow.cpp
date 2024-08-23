#include "mainwindow.h"
#include <iostream>

void PrintDeviceInfo(MV_CC_DEVICE_INFO *pstMVDevInfo)
{
    if (NULL == pstMVDevInfo)
    {
        std::cout << "The Pointer of pstMVDevInfo is NULL!" << std::endl;
        return;
    }
    if (pstMVDevInfo->nTLayerType == MV_GIGE_DEVICE)
    {
        int nIp1 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0xff000000) >> 24);
        int nIp2 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x00ff0000) >> 16);
        int nIp3 = ((pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x0000ff00) >> 8);
        int nIp4 = (pstMVDevInfo->SpecialInfo.stGigEInfo.nCurrentIp & 0x000000ff);

        // print current ip and user defined name
        std::cout << "Device Model Name: " << pstMVDevInfo->SpecialInfo.stGigEInfo.chModelName << std::endl;
        std::cout << "CurrentIp: " << nIp1 << "." << nIp2 << "." << nIp3 << "." << nIp4 << std::endl;
        std::cout << "UserDefinedName: " << pstMVDevInfo->SpecialInfo.stGigEInfo.chUserDefinedName << std::endl;
    }
    else if (pstMVDevInfo->nTLayerType == MV_USB_DEVICE)
    {
        std::cout << "Device Model Name: " << pstMVDevInfo->SpecialInfo.stUsb3VInfo.chModelName << std::endl;
        std::cout << "UserDefinedName: " << pstMVDevInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName << std::endl;
    }
    else
    {
        std::cout << "Not support" << std::endl;
    }
}

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
    m_builder->get_widget("discover_btn", m_discover_btn);
    m_builder->get_widget("connect_btn", m_connect_btn);
    m_builder->get_widget("start_btn", m_start_btn);
    m_builder->get_widget("stop_btn", m_stop_btn);
    m_builder->get_widget("disconnect_btn", m_disconnect_btn);

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

void MainWindow::on_discover_clicked()
{
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
                PrintDeviceInfo(pDeviceInfo);
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