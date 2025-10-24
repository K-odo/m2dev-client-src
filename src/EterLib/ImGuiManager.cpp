#include "StdAfx.h"
#include "ImGuiManager.h"

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "StateManager.h"

#ifdef IMGUI_ENABLE_FREETYPE
#include "imgui_freetype.h"
#endif

#include <vector>
#include <fstream>
#include <sstream>
#include <charconv>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

CImGuiManager::CImGuiManager()
	: m_hWnd(nullptr)
	, m_pDevice(nullptr)
	, m_bInitialized(false)
{
}

CImGuiManager::~CImGuiManager()
{
	Shutdown();
}

CImGuiManager& CImGuiManager::Instance()
{
	static std::unique_ptr<CImGuiManager> instance = std::make_unique<CImGuiManager>();
	return *instance;
}

bool CImGuiManager::Initialize(HWND hWnd, LPDIRECT3DDEVICE9 pDevice)
{
	if (m_bInitialized)
		return true;

	m_hWnd = hWnd;
	m_pDevice = pDevice;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	// Configure ImGui flags for Metin2 integration
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable keyboard navigation
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;   // Don't let ImGui change the mouse cursor (Metin2 manages it)

	if (!ImGui_ImplWin32_Init(hWnd))
		return false;

	if (!ImGui_ImplDX9_Init(pDevice))
	{
		ImGui_ImplWin32_Shutdown();
		return false;
	}

	ImGui::StyleColorsDark();

	m_bInitialized = true;
	return true;
}

void CImGuiManager::Shutdown()
{
	if (!m_bInitialized)
		return;

	// Clear font resources before destroying ImGui context
	m_mapFonts.clear();
	m_strActiveFontName.clear();

	// Shutdown ImGui implementations in reverse order of initialization
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// Reset all member variables to safe defaults
	m_hWnd = nullptr;
	m_pDevice = nullptr;
	m_bInitialized = false;
}

bool CImGuiManager::__ParseConfigFile(std::string_view configPath, std::vector<SFontConfig>& outConfigs)
{
	std::ifstream file{std::string(configPath)};
	if (!file.is_open())
	{
		TraceError("ImGuiManager: Failed to open font config file: %s", configPath.data());
		return false;
	}

	outConfigs.clear();
	SFontConfig currentConfig;
	bool hasName = false;
	std::string line;
	int lineNumber = 0;

	while (std::getline(file, line))
	{
		++lineNumber;

		size_t commentPos = line.find("//");
		if (commentPos == std::string::npos) commentPos = line.find('#');
		if (commentPos != std::string::npos) line = line.substr(0, commentPos);

		size_t start = line.find_first_not_of(" \t\r\n");
		size_t end = line.find_last_not_of(" \t\r\n");
		if (start == std::string::npos) continue;

		line = line.substr(start, end - start + 1);
		size_t delimPos = line.find_first_of(" \t");
		if (delimPos == std::string::npos) continue;

		std::string key = line.substr(0, delimPos);
		std::string value = line.substr(delimPos + 1);
		start = value.find_first_not_of(" \t");
		if (start != std::string::npos) value = value.substr(start);

		if (key == "FONT")
		{
			if (hasName && !currentConfig.name.empty()) outConfigs.push_back(currentConfig);
			currentConfig = SFontConfig();
			currentConfig.name = value;
			hasName = true;
		}
		else if (key == "PATH") currentConfig.path = value;
		else if (key == "SIZE")
		{
			float tempFloat;
			if (std::from_chars(value.data(), value.data() + value.size(), tempFloat).ec == std::errc())
				currentConfig.size = tempFloat;
		}
		else if (key == "ENABLE_OUTLINE")
		{
			int tempInt;
			if (std::from_chars(value.data(), value.data() + value.size(), tempInt).ec == std::errc())
				currentConfig.enableOutline = (tempInt != 0);
		}
	}

	if (hasName && !currentConfig.name.empty()) outConfigs.push_back(currentConfig);
	file.close();
	return !outConfigs.empty();
}

bool CImGuiManager::__LoadFont(const SFontConfig& config)
{
	if (!m_bInitialized) return false;

	// CHANGE: Check if the font file exists
	std::ifstream testFile(config.path);
    if (!testFile.good())
    {
        TraceError("ImGuiManager: Font file not found at path '%s'", config.path.c_str());
        return false;
    }
    testFile.close();

	ImGuiIO& io = ImGui::GetIO();
	SFontConfig adjustedConfig = config;

	if (adjustedConfig.enableOutline)
		adjustedConfig.outlineThickness = std::max(1, static_cast<int>(adjustedConfig.size * 0.05f));
	else
		adjustedConfig.outlineThickness = 0;

	// CHANGE: Use standard ImGuiFreeType flags instead of a custom enum
	unsigned int buildFlags = ImGuiFreeTypeBuilderFlags_LoadColor; // Always enable support for color glyphs/emoji

	if (adjustedConfig.size < 12.0f)
	{
		adjustedConfig.oversampleH = 3;
		adjustedConfig.oversampleV = 3;
		adjustedConfig.rasterizerMultiply = 1.3f;
		adjustedConfig.pixelSnapH = true;
		buildFlags |= ImGuiFreeTypeBuilderFlags_LightHinting;
	}
	else if (adjustedConfig.size > 24.0f)
	{
		adjustedConfig.oversampleH = 2;
		adjustedConfig.oversampleV = 2;
		adjustedConfig.rasterizerMultiply = 1.0f;
		adjustedConfig.pixelSnapH = false;
		buildFlags |= ImGuiFreeTypeBuilderFlags_NoHinting;
	}
	else
	{
		adjustedConfig.oversampleH = 4;
		adjustedConfig.oversampleV = 4;
		adjustedConfig.rasterizerMultiply = 1.2f;
		adjustedConfig.pixelSnapH = false;
		// For medium sizes, default flags (or light hinting) are OK
		buildFlags |= ImGuiFreeTypeBuilderFlags_LightHinting;
	}

	ImFontConfig imConfig;
	imConfig.OversampleH = adjustedConfig.oversampleH;
	imConfig.OversampleV = adjustedConfig.oversampleV;
	imConfig.PixelSnapH = adjustedConfig.pixelSnapH;
	imConfig.RasterizerMultiply = adjustedConfig.rasterizerMultiply;
#ifdef IMGUI_ENABLE_FREETYPE
	imConfig.FontLoaderFlags = buildFlags; // Set flags for the specific font
#endif

	static const ImWchar ranges[] = {
		0x0020, 0x00FF, 0x0100, 0x017F, 0x0180, 0x024F, 0x0400, 0x052F,
		0x2000, 0x206F, 0x3000, 0x30FF, 0x4E00, 0x9FAF, 0xAC00, 0xD7A3, 0,
	};

	ImFont* pFont = io.Fonts->AddFontFromFileTTF(config.path.c_str(), config.size, &imConfig, ranges);
	if (!pFont)
	{
		TraceError("ImGuiManager: Failed to load font '%s' from path '%s'", config.name.c_str(), config.path.c_str());
		return false;
	}

	SFontEntry entry{pFont, adjustedConfig.size, adjustedConfig.outlineThickness};
	m_mapFonts[config.name] = entry;

	if (m_strActiveFontName.empty()) m_strActiveFontName = config.name;

	return true;
}

bool CImGuiManager::LoadFontsFromConfig(std::string_view configPath)
{
	if (!m_bInitialized) return false;

	std::vector<SFontConfig> configs;
	if (!__ParseConfigFile(configPath, configs)) return false;

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Clear();

	for (const SFontConfig& config : configs)
	{
		__LoadFont(config);
	}

	if (m_mapFonts.empty())
	{
		TraceError("ImGuiManager: No fonts were loaded successfully");
		return false;
	}

	// Build the atlas. Flags are already set per-font in __LoadFont.
	if (!io.Fonts->Build())
	{
		TraceError("ImGuiManager: Failed to build font atlas!");
		return false;
	}

	ImGui_ImplDX9_InvalidateDeviceObjects();
	ImGui_ImplDX9_CreateDeviceObjects();
	return true;
}

bool CImGuiManager::LoadFont(std::string_view fontName, std::string_view fontPath, float fontSize, bool enableOutline)
{
	if (!m_bInitialized) return false;

	SFontConfig config;
	config.name = std::string(fontName);
	config.path = std::string(fontPath);
	config.size = fontSize;
	config.enableOutline = enableOutline;

	ImGuiIO& io = ImGui::GetIO();
	if (m_mapFonts.empty()) io.Fonts->Clear();

	if (__LoadFont(config))
	{
		io.Fonts->Build();
		ImGui_ImplDX9_InvalidateDeviceObjects();
		ImGui_ImplDX9_CreateDeviceObjects();
		return true;
	}
	return false;
}

bool CImGuiManager::LoadFontFromMemory(std::string_view fontName, std::span<const std::byte> fontData, float fontSize, bool enableOutline)
{
    if (!m_bInitialized || fontData.empty()) 
        return false;

    ImGuiIO& io = ImGui::GetIO();
    
    // CRITICAL: Allocate persistent memory for font data
    // ImGui will own this memory and free it when the atlas is cleared
    void* fontDataCopy = ImGui::MemAlloc(fontData.size());
    if (!fontDataCopy)
        return false;
    
    std::memcpy(fontDataCopy, fontData.data(), fontData.size());

    // Configure font with same settings as LoadFont
    SFontConfig fontConfig;
    fontConfig.name = std::string(fontName);
    fontConfig.size = fontSize;
    fontConfig.enableOutline = enableOutline;

    if (enableOutline)
        fontConfig.outlineThickness = std::max(1, static_cast<int>(fontSize * 0.05f));
    else
        fontConfig.outlineThickness = 0;

    // Auto-adjust parameters based on font size
    unsigned int buildFlags = ImGuiFreeTypeBuilderFlags_LoadColor;

    if (fontSize < 12.0f)
    {
        fontConfig.oversampleH = 3;
        fontConfig.oversampleV = 3;
        fontConfig.rasterizerMultiply = 1.3f;
        fontConfig.pixelSnapH = true;
        buildFlags |= ImGuiFreeTypeBuilderFlags_LightHinting;
    }
    else if (fontSize > 24.0f)
    {
        fontConfig.oversampleH = 2;
        fontConfig.oversampleV = 2;
        fontConfig.rasterizerMultiply = 1.0f;
        fontConfig.pixelSnapH = false;
        buildFlags |= ImGuiFreeTypeBuilderFlags_NoHinting;
    }
    else
    {
        fontConfig.oversampleH = 4;
        fontConfig.oversampleV = 4;
        fontConfig.rasterizerMultiply = 1.2f;
        fontConfig.pixelSnapH = false;
        buildFlags |= ImGuiFreeTypeBuilderFlags_LightHinting;
    }

    ImFontConfig imConfig;
    imConfig.FontDataOwnedByAtlas = true; // ImGui will free fontDataCopy
    imConfig.OversampleH = fontConfig.oversampleH;
    imConfig.OversampleV = fontConfig.oversampleV;
    imConfig.PixelSnapH = fontConfig.pixelSnapH;
    imConfig.RasterizerMultiply = fontConfig.rasterizerMultiply;
    
#ifdef IMGUI_ENABLE_FREETYPE
    imConfig.FontLoaderFlags = buildFlags;
#endif

    static const ImWchar ranges[] = {
        0x0020, 0x00FF, 0x0100, 0x017F, 0x0180, 0x024F, 0x0400, 0x052F,
        0x2000, 0x206F, 0x3000, 0x30FF, 0x4E00, 0x9FAF, 0xAC00, 0xD7A3, 0,
    };

    // Clear fonts if this is the first font being loaded
    if (m_mapFonts.empty()) 
        io.Fonts->Clear();

    ImFont* pFont = io.Fonts->AddFontFromMemoryTTF(
        fontDataCopy,
        static_cast<int>(fontData.size()),
        fontSize,
        &imConfig,
        ranges);

    if (!pFont)
    {
        ImGui::MemFree(fontDataCopy); // Clean up on failure
        TraceError("ImGuiManager: Failed to load font '%s' from memory", fontName.data());
        return false;
    }

    SFontEntry entry{pFont, fontSize, fontConfig.outlineThickness};
    m_mapFonts[std::string(fontName)] = entry;

    if (m_strActiveFontName.empty()) 
        m_strActiveFontName = std::string(fontName);

    // Build and recreate device objects
    if (!io.Fonts->Build())
    {
        TraceError("ImGuiManager: Failed to build font atlas!");
        return false;
    }

    ImGui_ImplDX9_InvalidateDeviceObjects();
    ImGui_ImplDX9_CreateDeviceObjects();
    
    return true;
}

bool CImGuiManager::SetActiveFont(std::string_view fontName)
{
	if (!m_mapFonts.contains(fontName)) return false;
	m_strActiveFontName = fontName;
	return true;
}

ImFont* CImGuiManager::GetFont(std::string_view fontName) const noexcept
{
	if (fontName.empty()) fontName = m_strActiveFontName;
	if (auto it = m_mapFonts.find(fontName); it != m_mapFonts.end())
		return it->second.pFont;
	return nullptr;
}

void CImGuiManager::BeginFrame() noexcept
{
	if (m_bInitialized)
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}
}

void CImGuiManager::EndFrame() noexcept
{
	if (m_bInitialized)
		ImGui::EndFrame();
}

void CImGuiManager::Render() noexcept
{
	if (!m_bInitialized)
		return;

	// Save current DirectX9 render states to prevent interference with game rendering
	const DWORD savedZEnable = STATEMANAGER.GetRenderState(D3DRS_ZENABLE);
	const DWORD savedAlphaBlendEnable = STATEMANAGER.GetRenderState(D3DRS_ALPHABLENDENABLE);
	const DWORD savedScissorTestEnable = STATEMANAGER.GetRenderState(D3DRS_SCISSORTESTENABLE);

	// Set render states for ImGui rendering (2D overlay, no depth testing)
	STATEMANAGER.SetRenderState(D3DRS_ZENABLE, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	STATEMANAGER.SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	// Render ImGui draw data
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	// Restore original render states for game rendering
	STATEMANAGER.SetRenderState(D3DRS_ZENABLE, savedZEnable);
	STATEMANAGER.SetRenderState(D3DRS_ALPHABLENDENABLE, savedAlphaBlendEnable);
	STATEMANAGER.SetRenderState(D3DRS_SCISSORTESTENABLE, savedScissorTestEnable);
}

void CImGuiManager::FlushAndRestart() noexcept
{
	if (m_bInitialized)
	{
		EndFrame();
		Render();
		BeginFrame();
	}
}

ImDrawList* CImGuiManager::GetDrawListForLayer(ERenderLayer layer) noexcept
{
	// Map layer enum to appropriate ImGui DrawList with z-order consideration
	// Background and UI layers use Background DrawList
	// Foreground uses Foreground DrawList

	const int layerValue = static_cast<int>(layer);

	if (layerValue >= static_cast<int>(ERenderLayer::Foreground))
	{
		return ImGui::GetForegroundDrawList();
	}
	else
	{
		return ImGui::GetBackgroundDrawList();
	}
}

void CImGuiManager::RenderText(std::string_view text, float x, float y, unsigned long color, ERenderLayer layer)
{
	if (!m_bInitialized || text.empty()) [[unlikely]]
		return;

	auto it = m_mapFonts.find(m_strActiveFontName);
	if (it == m_mapFonts.end()) [[unlikely]]
		return;

	ImDrawList* drawList = GetDrawListForLayer(layer);
	drawList->AddText(it->second.pFont, it->second.fontSize, ImVec2(x, y),
		__ConvertD3DColorToImGuiColor(color), text.data(), text.data() + text.size());
}

std::string CImGuiManager::WideToUtf8(std::wstring_view text)
{
	if (text.empty()) return {};
	int size = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
	if (size <= 0) return {};
	std::string result(size, '\0');
	WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), size, nullptr, nullptr);
	return result;
}

void CImGuiManager::RenderTextW(std::wstring_view text, float x, float y, unsigned long color, ERenderLayer layer)
{
	if (text.empty()) return;
	RenderText(WideToUtf8(text), x, y, color, layer);
}

void CImGuiManager::RenderTextWithOutline(std::string_view text, float x, float y, unsigned long textColor, unsigned long outlineColor, ERenderLayer layer)
{
	if (!m_bInitialized || text.empty()) [[unlikely]]
		return;

	auto it = m_mapFonts.find(m_strActiveFontName);
	if (it == m_mapFonts.end()) [[unlikely]]
		return;

	const int thickness = it->second.outlineThickness;
	if (thickness <= 0) [[unlikely]]
	{
		RenderText(text, x, y, textColor, layer);
		return;
	}

	ImDrawList* drawList = GetDrawListForLayer(layer);
	const ImU32 imOutlineColor = __ConvertD3DColorToImGuiColor(outlineColor);
	const ImU32 imTextColor = __ConvertD3DColorToImGuiColor(textColor);
	const char* const textStart = text.data();
	const char* const textEnd = textStart + text.size();
	const float offset = static_cast<float>(thickness);
	ImFont* const font = it->second.pFont;
	const float fontSize = it->second.fontSize;

	// 8-directional outline for better quality
	drawList->AddText(font, fontSize, ImVec2(x - offset, y - offset), imOutlineColor, textStart, textEnd);
	drawList->AddText(font, fontSize, ImVec2(x,          y - offset), imOutlineColor, textStart, textEnd);
	drawList->AddText(font, fontSize, ImVec2(x + offset, y - offset), imOutlineColor, textStart, textEnd);
	drawList->AddText(font, fontSize, ImVec2(x - offset, y         ), imOutlineColor, textStart, textEnd);
	drawList->AddText(font, fontSize, ImVec2(x + offset, y         ), imOutlineColor, textStart, textEnd);
	drawList->AddText(font, fontSize, ImVec2(x - offset, y + offset), imOutlineColor, textStart, textEnd);
	drawList->AddText(font, fontSize, ImVec2(x,          y + offset), imOutlineColor, textStart, textEnd);
	drawList->AddText(font, fontSize, ImVec2(x + offset, y + offset), imOutlineColor, textStart, textEnd);

	// Main text on top
	drawList->AddText(font, fontSize, ImVec2(x, y), imTextColor, textStart, textEnd);
}

void CImGuiManager::RenderTextWithOutlineW(std::wstring_view text, float x, float y, unsigned long textColor, unsigned long outlineColor, ERenderLayer layer)
{
	if (text.empty()) return;
	RenderTextWithOutline(WideToUtf8(text), x, y, textColor, outlineColor, layer);
}

void CImGuiManager::GetTextExtent(std::string_view text, int* width, int* height,
	std::string_view fontName) const
{
	if (width) *width = 0;
	if (height) *height = 0;

	if (!m_bInitialized || text.empty()) [[unlikely]]
		return;

	auto it = fontName.empty()
		? m_mapFonts.find(m_strActiveFontName)
		: m_mapFonts.find(fontName);

	if (it == m_mapFonts.end()) [[unlikely]]
		return;

	const ImVec2 size = it->second.pFont->CalcTextSizeA(
		it->second.fontSize, FLT_MAX, 0.0f,
		text.data(), text.data() + text.size());

	if (width) *width = static_cast<int>(size.x);
	if (height) *height = static_cast<int>(size.y);
}

void CImGuiManager::GetTextExtentW(std::wstring_view text, int* width, int* height, std::string_view fontName) const
{
	if (text.empty())
	{
		if (width) *width = 0;
		if (height) *height = 0;
		return;
	}
	GetTextExtent(WideToUtf8(text), width, height, fontName);
}

void CImGuiManager::OnLostDevice() noexcept { if (m_bInitialized) ImGui_ImplDX9_InvalidateDeviceObjects(); }
void CImGuiManager::OnResetDevice() noexcept { if (m_bInitialized) ImGui_ImplDX9_CreateDeviceObjects(); }

bool CImGuiManager::WantCaptureKeyboard() const noexcept
{
	// Early exit if not initialized - avoids ImGui::GetIO() call
	if (!m_bInitialized) [[unlikely]]
		return false;

	// Cache IO reference to avoid multiple function calls
	const ImGuiIO& io = ImGui::GetIO();
	return io.WantCaptureKeyboard;
}

bool CImGuiManager::WantCaptureMouse() const noexcept
{
	// Early exit if not initialized - avoids ImGui::GetIO() call
	if (!m_bInitialized) [[unlikely]]
		return false;

	// Cache IO reference to avoid multiple function calls
	const ImGuiIO& io = ImGui::GetIO();
	return io.WantCaptureMouse;
}

constexpr unsigned int CImGuiManager::__ConvertD3DColorToImGuiColor(unsigned long d3dColor) noexcept
{
	const unsigned char a = (d3dColor >> 24) & 0xFF;
	const unsigned char r = (d3dColor >> 16) & 0xFF;
	const unsigned char g = (d3dColor >> 8) & 0xFF;
	const unsigned char b = d3dColor & 0xFF;
	return (unsigned int)a << 24 | (unsigned int)b << 16 | (unsigned int)g << 8 | (unsigned int)r;
}