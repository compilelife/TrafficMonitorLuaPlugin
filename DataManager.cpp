#include "pch.h"
#include "DataManager.h"

CDataManager CDataManager::m_instance;

CDataManager::CDataManager()
{
    HDC hDC = ::GetDC(HWND_DESKTOP);
    m_dpi = GetDeviceCaps(hDC, LOGPIXELSY);
    ::ReleaseDC(HWND_DESKTOP, hDC);
#ifdef _DEBUG
    mPluginPath = "C:\\Users\\cy\\personal\\tool\\TrafficMonitor\\plugins";
#else
    TCHAR szBuff[MAX_PATH] = { 0 };
    HMODULE hModuleInst = _AtlBaseModule.GetModuleInstance();
    GetModuleFileName(hModuleInst, szBuff, MAX_PATH);

    CString modulePath(szBuff);
    auto index = modulePath.ReverseFind('\\');
    mPluginPath = modulePath.Left(index + 1);
#endif
}

CDataManager::~CDataManager()
{
    SaveConfig();
}

CDataManager& CDataManager::Instance()
{
    return m_instance;
}

int CDataManager::DPI(int pixel)
{
    return m_dpi * pixel / 96;
}

void CDataManager::LoadConfig(const std::wstring& config_dir)
{
    m_config_path = mPluginPath + L"TrafficMonitorPlugin.ini";
    //m_setting_data.show_second = GetPrivateProfileInt(_T("config"), _T("show_second"), 0, m_config_path.c_str());
    //m_setting_data.show_label_text = GetPrivateProfileInt(_T("config"), _T("show_label_text"), 1, config_path.c_str());
}

static void WritePrivateProfileInt(const wchar_t* app_name, const wchar_t* key_name, int value, const wchar_t* file_path)
{
    wchar_t buff[16];
    swprintf_s(buff, L"%d", value);
    WritePrivateProfileString(app_name, key_name, buff, file_path);
}

void CDataManager::SaveConfig() const
{
    //WritePrivateProfileInt(_T("config"), _T("show_second"), m_setting_data.show_second, m_config_path.c_str());
    //WritePrivateProfileInt(_T("config"), _T("show_label_text"), m_setting_data.show_label_text, config_path.c_str());
}

const CString& CDataManager::StringRes(UINT id)
{
    auto iter = m_string_table.find(id);
    if (iter != m_string_table.end())
    {
        return iter->second;
    }
    else
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        m_string_table[id].LoadString(id);
        return m_string_table[id];
    }
}

CString CDataManager::PluginPath()
{
    return mPluginPath;
}
