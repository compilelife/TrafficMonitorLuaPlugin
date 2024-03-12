#include "pch.h"
#include "TrafficMonitorLuaPlugin.h"
#include "DataManager.h"
#include "OptionsDlg.h"
#include "LuaScriptManager.h"
#include "log.h"
#include <fstream>

CTrafficMonitorLuaPlugin CTrafficMonitorLuaPlugin::m_instance;

CTrafficMonitorLuaPlugin::CTrafficMonitorLuaPlugin()
{
}

CTrafficMonitorLuaPlugin& CTrafficMonitorLuaPlugin::Instance()
{
    return m_instance;
}

IPluginItem* CTrafficMonitorLuaPlugin::GetItem(int index)
{
    auto luas = mlm.items();
    if (index < luas.size()) {
        return luas[index];
    }

    return nullptr;
}

void CTrafficMonitorLuaPlugin::DataRequired()
{
    mlm.DataRequired();
}

const wchar_t* CTrafficMonitorLuaPlugin::GetInfo(PluginInfoIndex index)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    static CString str;
    switch (index)
    {
    case TMI_NAME:
        str.LoadString(IDS_PLUGIN_NAME);
        return str.GetString();
    case TMI_DESCRIPTION:
        str.LoadString(IDS_PLUGIN_DESCRIPTION);
        return str.GetString();
    case TMI_AUTHOR:
        return L"compilelife";
    case TMI_COPYRIGHT:
        return L"Copyright (C) by compilelife 2024";
    case TMI_VERSION:
        return L"1.0";
    case ITMPlugin::TMI_URL:
        return L"https://github.com/compilelife/TrafficMonitorLuaPlugin";
        break;
    default:
        break;
    }
    return L"";
}

ITMPlugin::OptionReturn CTrafficMonitorLuaPlugin::ShowOptionsDialog(void* hParent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    COptionsDlg dlg(CWnd::FromHandle((HWND)hParent));
    dlg.m_data = CDataManager::Instance().m_setting_data;
    if (dlg.DoModal() == IDOK)
    {
        CDataManager::Instance().m_setting_data = dlg.m_data;
        return ITMPlugin::OR_OPTION_CHANGED;
    }
    return ITMPlugin::OR_OPTION_UNCHANGED;
}


void CTrafficMonitorLuaPlugin::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data)
{
    switch (index)
    {
    case ITMPlugin::EI_CONFIG_DIR:
        //从配置文件读取配置
        g_data.LoadConfig(std::wstring(data));

        break;
    default:
        break;
    }
}

void CTrafficMonitorLuaPlugin::OnMonitorInfo(const MonitorInfo& monitor_info)
{
    CDataManager::Instance().m_monitor_info = monitor_info;
}

void CTrafficMonitorLuaPlugin::LoadLuaPlugins(CString luaPath)
{
    mlm.init(luaPath);
}

void appInit();
ITMPlugin* TMPluginGetInstance()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    static bool first = true;
    auto thiz = &CTrafficMonitorLuaPlugin::Instance();
    if (first) {
        lg << "app begin init" << endl;
        appInit();
        thiz->LoadLuaPlugins(CDataManager::Instance().PluginPath());
        lg << "app inited" << endl;
        first = false;
    }
    return thiz;
}

#if _DEBUG //配合 设置=>常规=>配置类型
void test() {
    auto plugin = (CTrafficMonitorLuaPlugin*)TMPluginGetInstance();
    
    plugin->DataRequired();

    HDC hDC = ::GetDC(HWND_DESKTOP);
    auto width = plugin->GetItem(1)->GetItemWidthEx(hDC);
    plugin->GetItem(1)->DrawItem(hDC, 50, 50, width, 16, false);
    ::ReleaseDC(HWND_DESKTOP, hDC);
}

int AFXAPI AfxWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    _In_ LPTSTR lpCmdLine, int nCmdShow) {
    AllocConsole();//控制台调试窗口开启  
    FILE* s;
    freopen_s(&s, "CONOUT$", "w", stdout);//开启中文控制台输出支持 

    test();

    system("pause");
    FreeConsole(); // 释放控制台资源  
    return 0;
}

void appInit() {
    wcout.imbue(locale("zh_CN"));
    dbgBackend = &wcout;
}
#else
void appInit() {
    wofstream* log = new wofstream(CDataManager::Instance().PluginPath() + "\\luaplugin.txt");
    log->imbue(locale("zh_CN"));
    dbgBackend = log;
}
#endif
