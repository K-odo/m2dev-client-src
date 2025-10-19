#include "StdAfx.h"
#include "ImGuiManager.h"

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

#ifdef IMGUI_ENABLE_FREETYPE
#include "imgui_freetype.h"
#endif

#include <vector>
#include <fstream>
#include <sstream>
#include <charconv>

// Forward declare message handler from imgui_impl_win32.cpp
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
	// Modern C++20 thread-safe singleton using unique_ptr and magic statics
	// Magic statics guarantee thread-safe initialization since C++11
	static std::unique_ptr<CImGuiManager> instance = std::make_unique<CImGuiManager>();
	return *instance;
}

bool CImGuiManager::Initialize(HWND hWnd, LPDIRECT3DDEVICE9 pDevice)
{
	if (m_bInitialized)
		return true;

	m_hWnd = hWnd;
	m_pDevice = pDevice;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	// Disable ini file
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	// Setup Platform/Renderer backends
	if (!ImGui_ImplWin32_Init(hWnd))
		return false;

	if (!ImGui_ImplDX9_Init(pDevice))
	{
		ImGui_ImplWin32_Shutdown();
		return false;
	}

	// Setup style
	ImGui::StyleColorsDark();

	m_bInitialized = true;

#ifdef IMGUI_ENABLE_FREETYPE
	Tracef("ImGui Manager initialized successfully with FreeType support enabled");
#else
	Tracef("ImGui Manager initialized WITHOUT FreeType support - using default font rasterizer");
#endif

	return true;
}

void CImGuiManager::Shutdown()
{
	if (!m_bInitialized)
		return;

	m_mapFonts.clear();
	m_strActiveFontName.clear();

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	m_bInitialized = false;
}

bool CImGuiManager::__ParseConfigFile(std::string_view configPath, std::vector<SFontConfig>& outConfigs)
{
	// C++20: Use uniform initialization to avoid most vexing parse
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

		// Remove comments (// or #)
		size_t commentPos = line.find("//");
		if (commentPos == std::string::npos)
			commentPos = line.find('#');
		if (commentPos != std::string::npos)
			line = line.substr(0, commentPos);

		// Trim whitespace
		size_t start = line.find_first_not_of(" \t\r\n");
		size_t end = line.find_last_not_of(" \t\r\n");
		if (start == std::string::npos || end == std::string::npos)
			continue; // Empty line

		line = line.substr(start, end - start + 1);

		// Parse key-value pairs
		size_t delimPos = line.find_first_of(" \t");
		if (delimPos == std::string::npos)
		{
			TraceError("ImGuiManager: Invalid syntax at line %d: %s", lineNumber, line.c_str());
			continue;
		}

		std::string key = line.substr(0, delimPos);
		std::string value = line.substr(delimPos + 1);

		// Trim value
		start = value.find_first_not_of(" \t");
		if (start != std::string::npos)
			value = value.substr(start);

		// Process directives
		if (key == "FONT")
		{
			// Save previous font config if valid
			if (hasName && !currentConfig.name.empty())
			{
				outConfigs.push_back(currentConfig);
			}

			// Start new font config
			currentConfig = SFontConfig();
			currentConfig.name = value;
			hasName = true;
		}
		else if (key == "PATH")
		{
			currentConfig.path = value;
		}
		else if (key == "SIZE")
		{
			// C++17/20: std::from_chars for fast and safe string-to-number conversion
			float tempFloat;
			auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), tempFloat);
			if (ec == std::errc())
				currentConfig.size = tempFloat;
		}
		else if (key == "OVERSAMPLE_H")
		{
			int tempInt;
			auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), tempInt);
			if (ec == std::errc())
				currentConfig.oversampleH = tempInt;
		}
		else if (key == "OVERSAMPLE_V")
		{
			int tempInt;
			auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), tempInt);
			if (ec == std::errc())
				currentConfig.oversampleV = tempInt;
		}
		else if (key == "RASTERIZER_MULTIPLY")
		{
			float tempFloat;
			auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), tempFloat);
			if (ec == std::errc())
				currentConfig.rasterizerMultiply = tempFloat;
		}
		else if (key == "ENABLE_OUTLINE")
		{
			int tempInt;
			auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), tempInt);
			if (ec == std::errc())
				currentConfig.enableOutline = (tempInt != 0);
		}
		else if (key == "OUTLINE_THICKNESS")
		{
			int tempInt;
			auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), tempInt);
			if (ec == std::errc())
				currentConfig.outlineThickness = tempInt;
		}
		else if (key == "PIXEL_SNAP_H")
		{
			int tempInt;
			auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), tempInt);
			if (ec == std::errc())
				currentConfig.pixelSnapH = (tempInt != 0);
		}
	}

	// Save last font config
	if (hasName && !currentConfig.name.empty())
	{
		outConfigs.push_back(currentConfig);
	}

	file.close();

	Tracef("ImGuiManager: Loaded %d font configurations from %s", (int)outConfigs.size(), configPath.data());
	return !outConfigs.empty();
}

bool CImGuiManager::__LoadFont(const SFontConfig& config)
{
	if (!m_bInitialized)
		return false;

	ImGuiIO& io = ImGui::GetIO();

	// Configure font
	ImFontConfig imConfig;
	imConfig.OversampleH = config.oversampleH;
	imConfig.OversampleV = config.oversampleV;
	imConfig.PixelSnapH = config.pixelSnapH;
	imConfig.RasterizerMultiply = config.rasterizerMultiply;

	// Unicode ranges (same as before)
	static const ImWchar ranges[] = {
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0100, 0x017F, // Latin Extended-A
		0x0180, 0x024F, // Latin Extended-B
		0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
		0x2000, 0x206F, // General Punctuation
		0x3000, 0x30FF, // CJK Symbols and Punctuation, Hiragana, Katakana
		0x4E00, 0x9FAF, // CJK Ideograms
		0xAC00, 0xD7A3, // Hangul Syllables
		0,
	};

	// Load main font
	ImFont* pFont = io.Fonts->AddFontFromFileTTF(config.path.c_str(), config.size, &imConfig, ranges);
	if (!pFont)
	{
		TraceError("ImGuiManager: Failed to load font '%s' from path '%s'", config.name.c_str(), config.path.c_str());
		return false;
	}

	// C++20 designated initializers for font entry
	// outlineThickness comes from OUTLINE_THICKNESS in FreeTypeFont.txt
	SFontEntry entry{
		.pFont = pFont,
		.fontSize = config.size,
		.outlineThickness = config.outlineThickness
	};

	m_mapFonts[config.name] = entry;

	// Set as active if it's the first font
	if (m_strActiveFontName.empty())
		m_strActiveFontName = config.name;

#ifdef IMGUI_ENABLE_FREETYPE
	Tracef("ImGuiManager: Loaded font '%s' (size: %.1f, outline: %s, thickness: %d, FreeType: enabled)",
		config.name.c_str(), config.size, config.enableOutline ? "yes" : "no", config.outlineThickness);
#else
	Tracef("ImGuiManager: Loaded font '%s' (size: %.1f, outline: %s, thickness: %d, FreeType: DISABLED)",
		config.name.c_str(), config.size, config.enableOutline ? "yes" : "no", config.outlineThickness);
#endif

	return true;
}

bool CImGuiManager::LoadFontsFromConfig(std::string_view configPath)
{
	if (!m_bInitialized)
		return false;

	std::vector<SFontConfig> configs;
	if (!__ParseConfigFile(configPath, configs))
		return false;

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Clear();

	// Load all fonts
	bool anySuccess = false;
	for (const SFontConfig& config : configs)
	{
		if (__LoadFont(config))
			anySuccess = true;
	}

	if (!anySuccess)
	{
		TraceError("ImGuiManager: No fonts were loaded successfully");
		return false;
	}

	// Build atlas with FreeType for superior rendering quality
#ifdef IMGUI_ENABLE_FREETYPE
	// C++20: Use our strongly-typed enum for FreeType quality settings
	// Set global FontLoader flags for FreeType (NoHinting | LightHinting = HighQuality)
	constexpr auto quality = EFreeTypeQuality::HighQuality;
	io.Fonts->FontLoaderFlags = __ConvertToFreeTypeFlags(quality);

	if (!io.Fonts->Build())
	{
		TraceError("ImGuiManager: Failed to build font atlas with FreeType!");
		return false;
	}

	Tracef("ImGuiManager: Font atlas built successfully with FreeType rasterizer (quality: HighQuality, flags: 0x%X)",
		io.Fonts->FontLoaderFlags);
#else
	if (!io.Fonts->Build())
	{
		TraceError("ImGuiManager: Failed to build font atlas!");
		return false;
	}
	Tracef("ImGuiManager: Font atlas built with default rasterizer (FreeType NOT enabled!)");
#endif

	// Rebuild device objects
	ImGui_ImplDX9_InvalidateDeviceObjects();
	ImGui_ImplDX9_CreateDeviceObjects();

	return true;
}

bool CImGuiManager::LoadFont(std::string_view fontName, std::string_view fontPath, float fontSize, bool enableOutline)
{
	if (!m_bInitialized)
		return false;

	SFontConfig config;
	config.name = std::string(fontName);
	config.path = std::string(fontPath);
	config.size = fontSize;
	config.enableOutline = enableOutline;

	ImGuiIO& io = ImGui::GetIO();

	// If this is the first font, clear the atlas
	if (m_mapFonts.empty())
		io.Fonts->Clear();

	bool result = __LoadFont(config);

	if (result)
	{
		// Build with FreeType
#ifdef IMGUI_ENABLE_FREETYPE
		constexpr auto quality = EFreeTypeQuality::HighQuality;
		io.Fonts->FontLoaderFlags = __ConvertToFreeTypeFlags(quality);
#endif
		io.Fonts->Build();
		ImGui_ImplDX9_InvalidateDeviceObjects();
		ImGui_ImplDX9_CreateDeviceObjects();
	}

	return result;
}

bool CImGuiManager::LoadFontFromMemory(std::string_view fontName, std::span<const std::byte> fontData, float fontSize, bool enableOutline)
{
	if (!m_bInitialized)
		return false;

	ImGuiIO& io = ImGui::GetIO();

	ImFontConfig config;
	// CRITICAL FIX: Let ImGui own the font data to avoid dangling pointer
	// ImGui will internally copy the data, so it's safe even if fontData is temporary
	config.FontDataOwnedByAtlas = true;
	config.OversampleH = 4;
	config.OversampleV = 4;
	config.PixelSnapH = false;
	config.RasterizerMultiply = 1.2f;

	// std::span provides safe access to data and size
	ImFont* pFont = io.Fonts->AddFontFromMemoryTTF(
		const_cast<void*>(static_cast<const void*>(fontData.data())),
		static_cast<int>(fontData.size()),
		fontSize,
		&config);
	if (!pFont)
		return false;

	SFontEntry entry;
	entry.pFont = pFont;
	entry.fontSize = fontSize;
	entry.outlineThickness = enableOutline ? 1 : 0;

	m_mapFonts[std::string(fontName)] = entry;

	if (m_strActiveFontName.empty())
		m_strActiveFontName = std::string(fontName);

	// Build with FreeType
#ifdef IMGUI_ENABLE_FREETYPE
	constexpr auto quality = EFreeTypeQuality::HighQuality;
	io.Fonts->FontLoaderFlags = __ConvertToFreeTypeFlags(quality);
#endif
	io.Fonts->Build();
	ImGui_ImplDX9_InvalidateDeviceObjects();
	ImGui_ImplDX9_CreateDeviceObjects();

	return true;
}

bool CImGuiManager::SetActiveFont(std::string_view fontName)
{
	// C++20: Heterogeneous lookup - no allocation needed!
	auto it = m_mapFonts.find(fontName);
	if (it == m_mapFonts.end())
	{
		TraceError("ImGuiManager: Font '%s' not found", fontName.data());
		return false;
	}

	m_strActiveFontName = fontName;
	return true;
}

ImFont* CImGuiManager::GetFont(std::string_view fontName) const
{
	if (fontName.empty())
		fontName = m_strActiveFontName;

	// C++20: Heterogeneous lookup - no allocation needed!
	auto it = m_mapFonts.find(fontName);
	if (it != m_mapFonts.end())
		return it->second.pFont;

	return nullptr;
}

void CImGuiManager::BeginFrame() noexcept
{
	if (!m_bInitialized)
		return;

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void CImGuiManager::EndFrame() noexcept
{
	if (!m_bInitialized)
		return;

	ImGui::EndFrame();
}

void CImGuiManager::Render() noexcept
{
	if (!m_bInitialized)
		return;

	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void CImGuiManager::FlushAndRestart() noexcept
{
	if (!m_bInitialized)
		return;

	// Finish current frame and render accumulated draw commands
	EndFrame();
	Render();

	// Start new frame for next window layer
	BeginFrame();
}

void CImGuiManager::RenderText(std::string_view text, float x, float y, unsigned long color, bool shadow, ERenderLayer layer)
{
	if (!m_bInitialized || m_mapFonts.empty() || text.empty())
		return;

	auto it = m_mapFonts.find(m_strActiveFontName);
	if (it == m_mapFonts.end())
		return;

	ImFont* pFont = it->second.pFont;
	float fontSize = it->second.fontSize;

	ImU32 imColor = __ConvertD3DColorToImGuiColor(color);

	// C++20: Select draw list based on render layer for proper z-ordering
	ImDrawList* drawList = (layer == ERenderLayer::Foreground)
		? ImGui::GetForegroundDrawList()
		: ImGui::GetBackgroundDrawList();

	if (shadow)
	{
		drawList->AddText(pFont, fontSize, ImVec2(x + 1.0f, y + 1.0f), IM_COL32(0, 0, 0, 128), text.data(), text.data() + text.size());
	}

	drawList->AddText(pFont, fontSize, ImVec2(x, y), imColor, text.data(), text.data() + text.size());
}

std::string CImGuiManager::WideToUtf8(std::wstring_view text)
{
	if (text.empty())
		return {};

	// Calculate required buffer size
	const int size = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
	if (size <= 0)
		return {};

	// C++20: Direct string construction with size
	std::string result(size, '\0');
	WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), size, nullptr, nullptr);

	return result;
}

void CImGuiManager::RenderTextW(std::wstring_view text, float x, float y, unsigned long color, bool shadow, ERenderLayer layer)
{
	if (text.empty())
		return;

	// C++20: Use helper function to reduce code duplication
	const std::string utf8Text = WideToUtf8(text);
	if (!utf8Text.empty())
		RenderText(utf8Text, x, y, color, shadow, layer);
}

void CImGuiManager::RenderTextWithOutline(std::string_view text, float x, float y, unsigned long textColor, unsigned long outlineColor, ERenderLayer layer)
{
	if (!m_bInitialized || m_mapFonts.empty() || text.empty())
		return;

	auto it = m_mapFonts.find(m_strActiveFontName);
	if (it == m_mapFonts.end())
		return;

	ImFont* pFont = it->second.pFont;
	float fontSize = it->second.fontSize;
	int thickness = it->second.outlineThickness;

	ImU32 imOutlineColor = __ConvertD3DColorToImGuiColor(outlineColor);
	ImU32 imTextColor = __ConvertD3DColorToImGuiColor(textColor);

	// C++20: Select draw list based on render layer for proper z-ordering
	ImDrawList* drawList = (layer == ERenderLayer::Foreground)
		? ImGui::GetForegroundDrawList()
		: ImGui::GetBackgroundDrawList();

	const char* textStart = text.data();
	const char* textEnd = text.data() + text.size();

	// High-quality outline rendering using 8-directional offsets
	// Uses OUTLINE_THICKNESS from FreeTypeFont.txt config file
	// This creates a proper outline around text, not a shadow
	if (thickness > 0)
	{
		// 8-direction pattern: N, NE, E, SE, S, SW, W, NW
		// This ensures outline is CENTERED around text
		static constexpr float offsetsX[] = { 0.0f,  1.0f,  1.0f,  1.0f,  0.0f, -1.0f, -1.0f, -1.0f };
		static constexpr float offsetsY[] = {-1.0f, -1.0f,  0.0f,  1.0f,  1.0f,  1.0f,  0.0f, -1.0f };
		static_assert(std::size(offsetsX) == std::size(offsetsY), "Offset arrays must match");

		const float offset = static_cast<float>(thickness);

		for (size_t i = 0; i < std::size(offsetsX); ++i)
		{
			const float ox = offsetsX[i] * offset;
			const float oy = offsetsY[i] * offset;
			drawList->AddText(pFont, fontSize, ImVec2(x + ox, y + oy), imOutlineColor, textStart, textEnd);
		}
	}

	// Draw main text on top (centered - no offset)
	drawList->AddText(pFont, fontSize, ImVec2(x, y), imTextColor, textStart, textEnd);
}

void CImGuiManager::RenderTextWithOutlineW(std::wstring_view text, float x, float y, unsigned long textColor, unsigned long outlineColor, ERenderLayer layer)
{
	if (text.empty())
		return;

	// C++20: Use helper function to reduce code duplication
	const std::string utf8Text = WideToUtf8(text);
	if (!utf8Text.empty())
		RenderTextWithOutline(utf8Text, x, y, textColor, outlineColor, layer);
}

void CImGuiManager::RenderTextEx(std::string_view fontName, std::string_view text, float x, float y, unsigned long color, bool shadow)
{
	if (!m_bInitialized || text.empty() || fontName.empty())
		return;

	// C++20: Heterogeneous lookup - no allocation needed!
	auto it = m_mapFonts.find(fontName);
	if (it == m_mapFonts.end())
		return;

	ImFont* pFont = it->second.pFont;
	float fontSize = it->second.fontSize;

	ImU32 imColor = __ConvertD3DColorToImGuiColor(color);
	ImDrawList* drawList = ImGui::GetForegroundDrawList();

	const char* textStart = text.data();
	const char* textEnd = text.data() + text.size();

	if (shadow)
	{
		drawList->AddText(pFont, fontSize, ImVec2(x + 1.0f, y + 1.0f), IM_COL32(0, 0, 0, 128), textStart, textEnd);
	}

	drawList->AddText(pFont, fontSize, ImVec2(x, y), imColor, textStart, textEnd);
}

void CImGuiManager::RenderTextWithOutlineEx(std::string_view fontName, std::string_view text, float x, float y, unsigned long textColor, unsigned long outlineColor)
{
	if (!m_bInitialized || text.empty() || fontName.empty())
		return;

	// C++20: Heterogeneous lookup - no allocation needed!
	auto it = m_mapFonts.find(fontName);
	if (it == m_mapFonts.end())
		return;

	ImFont* pFont = it->second.pFont;
	float fontSize = it->second.fontSize;
	int thickness = it->second.outlineThickness;

	ImU32 imOutlineColor = __ConvertD3DColorToImGuiColor(outlineColor);
	ImU32 imTextColor = __ConvertD3DColorToImGuiColor(textColor);
	ImDrawList* drawList = ImGui::GetForegroundDrawList();

	const char* textStart = text.data();
	const char* textEnd = text.data() + text.size();

	// Outline rendering using config thickness
	if (thickness > 0)
	{
		static constexpr float offsetsX[] = { 0.0f,  1.0f,  1.0f,  1.0f,  0.0f, -1.0f, -1.0f, -1.0f };
		static constexpr float offsetsY[] = {-1.0f, -1.0f,  0.0f,  1.0f,  1.0f,  1.0f,  0.0f, -1.0f };
		static_assert(std::size(offsetsX) == std::size(offsetsY), "Offset arrays must match");

		const float offset = static_cast<float>(thickness);

		for (size_t i = 0; i < std::size(offsetsX); ++i)
		{
			const float ox = offsetsX[i] * offset;
			const float oy = offsetsY[i] * offset;
			drawList->AddText(pFont, fontSize, ImVec2(x + ox, y + oy), imOutlineColor, textStart, textEnd);
		}
	}

	// Draw main text
	drawList->AddText(pFont, fontSize, ImVec2(x, y), imTextColor, textStart, textEnd);
}

void CImGuiManager::GetTextExtent(std::string_view text, int* width, int* height, std::string_view fontName) const
{
	if (!m_bInitialized || text.empty())
	{
		if (width) *width = 0;
		if (height) *height = 0;
		return;
	}

	if (fontName.empty())
		fontName = m_strActiveFontName;

	// C++20: Heterogeneous lookup - no allocation needed!
	auto it = m_mapFonts.find(fontName);
	if (it == m_mapFonts.end())
	{
		if (width) *width = 0;
		if (height) *height = 0;
		return;
	}

	ImFont* pFont = it->second.pFont;
	float fontSize = it->second.fontSize;

	ImVec2 size = pFont->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text.data(), text.data() + text.size());

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

	// C++20: Use helper function to reduce code duplication
	const std::string utf8Text = WideToUtf8(text);
	GetTextExtent(utf8Text, width, height, fontName);
}

bool CImGuiManager::GetGlyphInfo(wchar_t character, SGlyphInfo& outInfo, std::string_view fontName) const
{
	if (!m_bInitialized || m_mapFonts.empty())
		return false;

	// Get active font if no font name specified
	if (fontName.empty())
		fontName = m_strActiveFontName;

	// C++20: Heterogeneous lookup - no allocation needed!
	auto it = m_mapFonts.find(fontName);
	if (it == m_mapFonts.end())
		return false;

	ImFont* pFont = it->second.pFont;
	float fontSize = it->second.fontSize;
	if (!pFont)
	{
		TraceError("ImGuiManager::GetGlyphInfo: Font pointer is null for '%s'", fontName.data());
		return false;
	}

	// ImGui 1.92+: Use GetFontBaked() to get baked data for given size
	ImFontBaked* pBaked = pFont->GetFontBaked(fontSize);
	if (!pBaked)
	{
		TraceError("ImGuiManager::GetGlyphInfo: GetFontBaked failed for font '%s' size %.1f - atlas may not be built or font not loaded",
			fontName.data(), fontSize);
		return false;
	}

	// Find glyph for this character
	const ImFontGlyph* pGlyph = pBaked->FindGlyph(static_cast<ImWchar>(character));
	if (!pGlyph || !pGlyph->Visible)
		return false;

	// Fill glyph info structure
	outInfo.x0 = pGlyph->X0;
	outInfo.y0 = pGlyph->Y0;
	outInfo.x1 = pGlyph->X1;
	outInfo.y1 = pGlyph->Y1;
	outInfo.u0 = pGlyph->U0;
	outInfo.v0 = pGlyph->V0;
	outInfo.u1 = pGlyph->U1;
	outInfo.v1 = pGlyph->V1;
	outInfo.advanceX = pGlyph->AdvanceX;

	return true;
}

LPDIRECT3DTEXTURE9 CImGuiManager::GetFontTexture(std::string_view fontName) const
{
	if (!m_bInitialized || m_mapFonts.empty())
		return nullptr;

	// Get active font if no font name specified
	if (fontName.empty())
		fontName = m_strActiveFontName;

	// C++20: Heterogeneous lookup - no allocation needed!
	auto it = m_mapFonts.find(fontName);
	if (it == m_mapFonts.end())
		return nullptr;

	ImFont* pFont = it->second.pFont;
	if (!pFont || !pFont->ContainerAtlas)
		return nullptr;

	// ImGui 1.92+: Use TexRef.GetTexID() instead of TexID
	ImTextureID texID = pFont->ContainerAtlas->TexRef.GetTexID();
	return reinterpret_cast<LPDIRECT3DTEXTURE9>(texID);
}

void CImGuiManager::OnLostDevice() noexcept
{
	if (m_bInitialized)
		ImGui_ImplDX9_InvalidateDeviceObjects();
}

void CImGuiManager::OnResetDevice() noexcept
{
	if (m_bInitialized)
		ImGui_ImplDX9_CreateDeviceObjects();
}

constexpr unsigned int CImGuiManager::__ConvertD3DColorToImGuiColor(unsigned long d3dColor) noexcept
{
	// D3DCOLOR format: ARGB (0xAARRGGBB)
	// ImGui format: ABGR (0xAABBGGRR)
	// C++20: constexpr bitwise operations for compile-time color conversion

	const unsigned char a = static_cast<unsigned char>((d3dColor >> 24) & 0xFF);
	const unsigned char r = static_cast<unsigned char>((d3dColor >> 16) & 0xFF);
	const unsigned char g = static_cast<unsigned char>((d3dColor >> 8) & 0xFF);
	const unsigned char b = static_cast<unsigned char>((d3dColor >> 0) & 0xFF);

	return (static_cast<unsigned int>(a) << 24) |
	       (static_cast<unsigned int>(b) << 16) |
	       (static_cast<unsigned int>(g) << 8) |
	       (static_cast<unsigned int>(r) << 0);
}

#ifdef IMGUI_ENABLE_FREETYPE
constexpr unsigned int CImGuiManager::__ConvertToFreeTypeFlags(EFreeTypeQuality quality) noexcept
{
	// C++20: constexpr conversion from our strongly-typed enum to ImGui flags
	// This allows compile-time validation and type safety
	return static_cast<unsigned int>(quality);
}
#endif
