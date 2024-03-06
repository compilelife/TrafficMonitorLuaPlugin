#pragma once
#include <string>
#include <map>
#include <PluginInterface.h>

#define g_data CDataManager::Instance()

struct SettingData
{
    //bool show_second{};
    //bool show_label_text{};
};

class CDataManager
{
private:
    CDataManager();
    ~CDataManager();

public:
    static CDataManager& Instance();
    int DPI(int pixel);
    void LoadConfig(const std::wstring& config_dir);
    void SaveConfig() const;
    const CString& StringRes(UINT id);      //根据资源id获取一个字符串资源
    CString PluginPath();

public:
    SettingData m_setting_data;
    ITMPlugin::MonitorInfo m_monitor_info;

private:
    static CDataManager m_instance;
    std::wstring m_config_path;
    std::map<UINT, CString> m_string_table;
    int m_dpi{ 96 };
    CString mPluginPath;
};
