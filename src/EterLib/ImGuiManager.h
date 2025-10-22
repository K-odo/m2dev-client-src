#pragma once

#include <d3d9.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <string_view>
#include <span>

// Forward declarations
struct ImFont;
struct ImDrawList;

// Font configuration structure
struct SFontConfig
{
	std::string name;
	std::string path;
	float size;
	bool enableOutline;

	// Auto-adjusted parameters
	int oversampleH;
	int oversampleV;
	float rasterizerMultiply;
	int outlineThickness;
	bool pixelSnapH;

	SFontConfig()
		: size(14.0f)
		, enableOutline(false)
		, oversampleH(4)
		, oversampleV(4)
		, rasterizerMultiply(1.2f)
		, outlineThickness(0)
		, pixelSnapH(false)
	{}
};

// Font entry structure
struct SFontEntry
{
	ImFont* pFont = nullptr;
	float fontSize = 14.0f;
	int outlineThickness = 1;
};

class CImGuiManager
{
public:
	CImGuiManager();
	~CImGuiManager();

	// Initialization and shutdown
	bool Initialize(HWND hWnd, LPDIRECT3DDEVICE9 pDevice);
	void Shutdown();

	// Font management
	[[nodiscard]] bool LoadFontsFromConfig(std::string_view configPath);
	[[nodiscard]] bool LoadFont(std::string_view fontName, std::string_view fontPath, float fontSize, bool enableOutline = false);
	[[nodiscard]] bool LoadFontFromMemory(std::string_view fontName, std::span<const std::byte> fontData, float fontSize, bool enableOutline = false);
	[[nodiscard]] bool SetActiveFont(std::string_view fontName);

	// Legacy compatibility
	[[nodiscard]] ImFont* GetFont(std::string_view fontName = "") const noexcept;

	// Frame lifecycle
	void BeginFrame() noexcept;
	void EndFrame() noexcept;
	void Render() noexcept;

	// Flush and restart frame
	void FlushAndRestart() noexcept;

	// Render layer control
	enum class ERenderLayer : int
	{
		Background = 0,
		Foreground = 1,
	};

	// Text rendering functions
	void RenderText(std::string_view text, float x, float y, unsigned long color, ERenderLayer layer = ERenderLayer::Background);
	void RenderTextW(std::wstring_view text, float x, float y, unsigned long color, ERenderLayer layer = ERenderLayer::Background);
	void RenderTextWithOutline(std::string_view text, float x, float y, unsigned long textColor, unsigned long outlineColor, ERenderLayer layer = ERenderLayer::Background);
	void RenderTextWithOutlineW(std::wstring_view text, float x, float y, unsigned long textColor, unsigned long outlineColor, ERenderLayer layer = ERenderLayer::Background);

	// Advanced rendering
	void RenderTextEx(std::string_view fontName, std::string_view text, float x, float y, unsigned long color);
	void RenderTextWithOutlineEx(std::string_view fontName, std::string_view text, float x, float y, unsigned long textColor, unsigned long outlineColor);

	// Text measurement
	void GetTextExtent(std::string_view text, int* width, int* height, std::string_view fontName = "") const;
	void GetTextExtentW(std::wstring_view text, int* width, int* height, std::string_view fontName = "") const;

	// UTF-8 conversion helper
	[[nodiscard]] static std::string WideToUtf8(std::wstring_view text);

	// Device management
	void OnLostDevice() noexcept;
	void OnResetDevice() noexcept;

	// Accessors
	[[nodiscard]] bool IsInitialized() const noexcept { return m_bInitialized; }

	// Singleton access
	static CImGuiManager& Instance();

private:
	bool __LoadFont(const SFontConfig& config);
	bool __ParseConfigFile(std::string_view configPath, std::vector<SFontConfig>& outConfigs);
	[[nodiscard]] static constexpr unsigned int __ConvertD3DColorToImGuiColor(unsigned long d3dColor) noexcept;

	// Transparent hash for zero-allocation lookup
	struct StringHash {
		using is_transparent = void;
		[[nodiscard]] size_t operator()(std::string_view sv) const noexcept { 
			return std::hash<std::string_view>{}(sv); 
		}
		[[nodiscard]] size_t operator()(const std::string& s) const noexcept { 
			return std::hash<std::string>{}(s); 
		}
	};

	HWND m_hWnd;
	LPDIRECT3DDEVICE9 m_pDevice;
	bool m_bInitialized;

	// OPTIMIZED: unordered_map with transparent hash for zero-allocation lookup
	std::unordered_map<std::string, SFontEntry, StringHash, std::equal_to<>> m_mapFonts;
	std::string m_strActiveFontName;
};