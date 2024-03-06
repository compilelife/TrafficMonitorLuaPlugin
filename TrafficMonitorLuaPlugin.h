#pragma once
#include "PluginInterface.h"
#include "LuaScriptManager.h"

class CTrafficMonitorLuaPlugin : public ITMPlugin
{
private:
    CTrafficMonitorLuaPlugin();

public:
    static CTrafficMonitorLuaPlugin& Instance();

    // 通过 ITMPlugin 继承
    virtual IPluginItem* GetItem(int index) override;
    virtual void DataRequired() override;
    virtual const wchar_t* GetInfo(PluginInfoIndex index) override;
    virtual OptionReturn ShowOptionsDialog(void* hParent) override;
    virtual void OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) override;
    virtual void OnMonitorInfo(const MonitorInfo& monitor_info) override;

public:
    void LoadLuaPlugins(CString luaPath);

private:
    LuaScriptManager mlm;

    static CTrafficMonitorLuaPlugin m_instance;
};

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) ITMPlugin* TMPluginGetInstance();

#ifdef __cplusplus
}
#endif
