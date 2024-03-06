#pragma once
#include <vector>
#include <string.h>
#include "PluginInterface.h"
#include "log.h"

using namespace std;

struct lua_State;

struct segment {
	string text;
	string image;
	bool hasImage() {
		return text.empty();
	}
};

#define IMAGE_SIZE 16

//doc:https://github.com/zhongyang219/TrafficMonitor/wiki/%E6%8F%92%E4%BB%B6%E5%BC%80%E5%8F%91%E6%8C%87%E5%8D%97
//按照自定义绘制的流程
class LuaItem : public IPluginItem {
public:
	LuaItem(lua_State* l, CString filename);
	void onUpdate();
	int getInterval();
	// 通过 IPluginItem 继承
	virtual const wchar_t* GetItemLableText() const { return L""; }
	virtual const wchar_t* GetItemValueText() const { return L""; }
	virtual const wchar_t* GetItemName() const override;
	virtual const wchar_t* GetItemId() const override;
	virtual const wchar_t* GetItemValueSampleText() const override;
	virtual bool IsCustomDraw() const override;
	virtual int GetItemWidthEx(void* hDC) const override;
	virtual void DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) override;
	virtual int OnMouseEvent(MouseEventType type, int x, int y, void* hWnd, int flag) override;
	ULONGLONG mLastTick{ 0 };
protected:
	lua_State* L{ nullptr };
private:
	vector<segment> parseValue(string value) const;
	int estimateImageWidth() const;
	int estimateTextWidth(CDC* pDC, string text) const;
	int mInterval{ 1 };
	wchar_t* mName{ nullptr };
	wchar_t* mId{ nullptr };
	wchar_t* mSample{ nullptr };
	vector<segment> mSegs;
};

class LuaScriptManager
{
public:
	void init(CString path);
	const vector<LuaItem*>& items() { return mItems; }
	void DataRequired();
private:
	LuaItem* createItem(CString path, CString name);
private:
	vector<LuaItem*> mItems;
};
