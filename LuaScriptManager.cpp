#include "pch.h"
#include "LuaScriptManager.h"
#include "DataManager.h"

#include "lua.hpp"
#include "LuaBridge/LuaBridge.h"

#include <windows.h>
#include <string>

#include <regex>
#include <map>

using namespace luabridge;

string runCmdLine(string cmd)
{
	/*
		╔══════════════════╗                ╔══════════════════╗
		║  Parent Process  ║                ║  Child Process   ║
		╠══════════════════╣                ╠══════════════════╣
		║                  ║                ║                  ║
		║ g_hChildStd_IN_Wr╟───────────── > ║g_hChildStd_IN_Rd ║
		║                  ║                ║                  ║
		║g_hChildStd_OUT_Rd║ < ─────────────╢g_hChildStd_OUT_Wr║
		║                  ║                ║                  ║
		╚══════════════════╝                ╚══════════════════╝
	*/
	HANDLE g_hChildStd_IN_Rd = NULL;
	HANDLE g_hChildStd_IN_Wr = NULL;
	HANDLE g_hChildStd_OUT_Rd = NULL;
	HANDLE g_hChildStd_OUT_Wr = NULL;
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = true;
	::CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &sa, 0);
	::CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &sa, 0);

	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.hStdError = g_hChildStd_OUT_Wr;
	si.hStdOutput = g_hChildStd_OUT_Wr;
	si.hStdInput = g_hChildStd_IN_Rd;
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	string output;

	PROCESS_INFORMATION pi;
	if (CreateProcess(NULL, CString(cmd.c_str()).GetBuffer(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi))
	{
		//父进程关闭不再需要的pipe端
		CloseHandle(g_hChildStd_IN_Rd);
		CloseHandle(g_hChildStd_OUT_Wr);

		// 读取输出
		const int N = 4096;
		char buffer[N] = { 0 };
		DWORD bytesRead = 0;
		while (true)
		{
			if (!ReadFile(g_hChildStd_OUT_Rd, buffer, N, &bytesRead, NULL) || !bytesRead)
				break;

			output.append(buffer, bytesRead);
		}

		// 等待子进程退出
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
	}
	else {
		lg << L"failed run cmdline: " << CString(cmd.c_str()).GetBuffer() <<":" << GetLastError() << endl;
	}

	CloseHandle(g_hChildStd_IN_Wr);
	CloseHandle(g_hChildStd_OUT_Rd);

	return output;
}

void registerLuaExtension(lua_State* L) {
	getGlobalNamespace(L)
		.beginNamespace("tf")
		.addFunction("runCmdLine", runCmdLine)
		.addProperty("monitorInfo", &CDataManager::Instance().m_monitor_info)
		.beginClass<ITMPlugin::MonitorInfo>("MonitorInfo")
			.addProperty("up_speed", &ITMPlugin::MonitorInfo::up_speed, false)
			.addProperty("down_speed", &ITMPlugin::MonitorInfo::down_speed, false)
			.addProperty("cpu_usage", &ITMPlugin::MonitorInfo::cpu_usage, false)
			.addProperty("memory_usage", &ITMPlugin::MonitorInfo::memory_usage, false)
			.addProperty("gpu_usage", &ITMPlugin::MonitorInfo::gpu_usage, false)
			.addProperty("hdd_usage", &ITMPlugin::MonitorInfo::hdd_usage, false)
			.addProperty("cpu_temperature", &ITMPlugin::MonitorInfo::cpu_temperature, false)
			.addProperty("gpu_temperature", &ITMPlugin::MonitorInfo::gpu_temperature, false)
			.addProperty("hdd_temperature", &ITMPlugin::MonitorInfo::hdd_temperature, false)
			.addProperty("main_board_temperature", &ITMPlugin::MonitorInfo::main_board_temperature, false)
			.addProperty("cpu_freq", &ITMPlugin::MonitorInfo::cpu_freq)
		.endClass()
		.endNamespace();
}



//不要忘记在使用完wchar_t*后delete[]释放内存
wchar_t* multiByteToWideChar(const string& pKey)
{
	char* pCStrKey = (char*)(pKey.c_str());
	//第一次调用返回转换后的字符串长度，用于确认为wchar_t*开辟多大的内存空间
	int pSize = MultiByteToWideChar(CP_OEMCP, 0, pCStrKey, strlen(pCStrKey) + 1, NULL, 0);
	wchar_t* pWCStrKey = new wchar_t[pSize];
	//第二次调用将单字节字符串转换成双字节字符串
	MultiByteToWideChar(CP_OEMCP, 0, pCStrKey, strlen(pCStrKey) + 1, pWCStrKey, pSize);
	return pWCStrKey;
}

void LuaScriptManager::init(CString path)
{
	CFileFind finder;
	auto found = finder.FindFile(path+"\\lua\\*");
	while (found)
	{
		found = finder.FindNextFile();
		auto name = finder.GetFileName();
		if (name == "." || name == "..")
			continue;

		auto path = finder.GetFilePath();
		if (finder.IsDirectory()) {
			path += "\\main.lua";
		}
		auto item = createItem(path, name);
		if (item) {
			mItems.push_back(item);
			lg << "load success:" << name.GetString() << endl;
		}
		else {
			lg << "load failed:" << name.GetString() << endl;
		}
	}
}
void LuaScriptManager::DataRequired()
{
	ULONGLONG curTick = GetTickCount64();
	for (auto item : mItems) {
		if (item->getInterval() <= 0) {
			continue;
		}
		auto diff = curTick - item->mLastTick;
		if (diff >= item->getInterval()) {
			item->onUpdate();
			item->mLastTick = curTick;
		}
	}
}

LuaItem* LuaScriptManager::createItem(CString path, CString name)
{
	USES_CONVERSION;
	auto cpath = T2A(path);

	auto L = luaL_newstate();
	luaL_openlibs(L);
	registerLuaExtension(L);
	
	if (luaL_loadfile(L, cpath) != 0) {
		lg << "load failed: " << lua_tostring(L, -1) << endl;
		return nullptr;
	}
	if (lua_pcall(L, 0, 0, -2) != 0) {
		lg << "pcall lua failed: " << lua_tostring(L, -1) << endl;
		return nullptr;
	}

	auto item = new LuaItem(L, name);
	item->mImgDir = path.Left(path.ReverseFind('\\'));
	
	//执行一次数据获取工作，确保interval为0的可以有数据
	item->onUpdate();

	return item;
}

LuaItem::LuaItem(lua_State* l, CString filename)
{
	L = l;
	auto name = getGlobal(L, "name").cast<string>();
	if (name.empty())
		mName = filename.AllocSysString();
	else
		mName = multiByteToWideChar(name);

	mId = filename.AllocSysString();

	auto sample = getGlobal(L, "sample").cast<string>();
	if (sample.empty())
		mSample = filename.AllocSysString();
	else
		mSample = multiByteToWideChar(sample);

	mInterval = getGlobal(L, "interval").cast<int>() * 1000;
}

const wchar_t* LuaItem::GetItemName() const
{
	return mName;
}

const wchar_t* LuaItem::GetItemId() const
{
	return mId;
}

const wchar_t* LuaItem::GetItemValueSampleText() const
{
	return mSample;
}

void LuaItem::onUpdate()
{
	auto value = getGlobal(L, "onUpdate")().cast<string>();
	mSegs = parseValue(value);
}

int LuaItem::getInterval()
{
	return mInterval;
}

bool LuaItem::IsCustomDraw() const
{
	return true;
}

int LuaItem::GetItemWidthEx(void* hDC) const
{
	auto sample = getGlobal(L, "sample").cast<string>();
	if (sample.empty()) {
		return 100;
	}
	
	auto segs = parseValue(sample);
	int width = 0;
	auto pDC = CDC::FromHandle((HDC)hDC);
	for (auto seg : segs) {
		if (seg.hasImage()) {
			width += estimateImageWidth();
		}
		else {
			width += estimateTextWidth(pDC, seg.text);
		}
	}

	return width;
}

void LuaItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
	CDC* pDC = CDC::FromHandle((HDC)hDC);
	CRect rect(CPoint(x, y), CSize(w, h));

	auto& gData = CDataManager::Instance();

	auto onlyText = true;
	for (auto seg : mSegs) {
		if (seg.hasImage()) {
			onlyText = false;
			break;
		}
	}

	if (onlyText) {
		string text;
		for (auto seg : mSegs) {
			text += seg.text;
		}
		auto multiLine = (h > g_data.DPI(30));
		UINT flag = multiLine ? (DT_NOPREFIX | DT_WORDBREAK) : (DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
		pDC->DrawText(text.c_str(), rect, flag);
	}
	else {
		for (auto seg : mSegs) {
			if (seg.hasImage()) {
				auto hicon = findOrLoadImage(seg.image);
				if (!hicon)
					continue;
				auto iconSize = gData.DPI(IMAGE_SIZE);
				::DrawIconEx(pDC->GetSafeHdc(), x, y, hicon, iconSize, iconSize, 0, NULL, DI_NORMAL);
				x += iconSize;
			}
			else {
				CRect textRect = rect;
				textRect.left = x;
				pDC->DrawText(seg.text.c_str(), textRect, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
				x += estimateTextWidth(pDC, seg.text);
			}
		}
	}
}

int LuaItem::OnMouseEvent(MouseEventType type, int x, int y, void* hWnd, int flag)
{
	if (type == MouseEventType::MT_LCLICKED) {
		auto value = getGlobal(L, "onClick")().cast<string>();
		if (!value.empty())
			mSegs = parseValue(value);
		return 1;
	}
	return 0;
}

HICON LuaItem::findOrLoadImage(string name) {
	auto it = mCache.find(name);
	if (it != mCache.end()) {
		return (*it).second;
	}
	else {
		auto path = mImgDir + "\\" + CString(name.c_str()) + ".ico";
		auto ret = (HICON)LoadImage(NULL, path, IMAGE_ICON, IMAGE_SIZE, IMAGE_SIZE, LR_LOADFROMFILE);
		if (ret) {
			mCache[name] = ret;
		}
		return ret;
	}
}

vector<segment> LuaItem::parseValue(string value) const
{
	//按[]分割图片和文字
	vector<segment> ret;

	std::regex pattern("\\[(.*?)\\]|(\\w+)");
	
	std::sregex_token_iterator iter(value.begin(), value.end(), pattern, { -1, 0 });
	std::sregex_token_iterator end;

	while (iter != end) {
		auto submatch = (*iter++);
		auto s = submatch.str();
		if (s.empty())
			continue;

		if (s[0] == '[') {
			ret.push_back({ "", s.substr(1, s.length()-2) });
		}
		else {
			ret.push_back({ s, "" });
		}
	}

	return ret;
}

int LuaItem::estimateImageWidth() const
{
	return CDataManager::Instance().DPI(IMAGE_SIZE);
}

int LuaItem::estimateTextWidth(CDC* pDC, string text) const
{
	return pDC->GetTextExtent(text.c_str()).cx;
}