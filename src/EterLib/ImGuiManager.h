#pragma once

#include <d3d9.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <string_view>
#include <span>

// Forward declarations
struct ImFont;
struct ImDrawList;

// C++20: FreeType rendering quality flags
enum class EFreeTypeQuality : unsigned int
{
	Default      = 0,                         // Default rendering
	NoHinting    = 1 << 0,                    // Disable hinting for pixel-perfect rendering
	NoAutoHint   = 1 << 1,                    // Disable auto-hinter
	ForceAutoHint = 1 << 2,                   // Force auto-hinter
	LightHinting = 1 << 3,                    // Light hinting for better readability
	MonoHinting  = 1 << 4,                    // Mono hinting
	Bold         = 1 << 5,                    // Simulate bold (FreeType emboldening)
	Oblique      = 1 << 6,                    // Simulate italic oblique
	Monochrome   = 1 << 7,                    // Render as 1-bit monochrome
	LoadColor    = 1 << 8,                    // Load color bitmaps/emojis

	// Recommended presets
	HighQuality  = NoHinting | LightHinting,  // Best quality for most fonts
	Sharp        = NoHinting | NoAutoHint,    // Sharpest rendering
	Smooth       = LightHinting,              // Smoothest rendering
};

// Font configuration structure
// C++20: Simplified config - only manual size and outline on/off
// All other parameters are auto-calculated based on size for optimal quality
struct SFontConfig
{
	std::string name;              // Font name (UI_DEF_FONT, UI_DEF_FONT_LARGE, etc.)
	std::string path;              // Path to TTF file
	float size;                    // Font size in pixels (MANUAL - user controls this)
	bool enableOutline;            // Enable outline rendering (MANUAL - on/off only)

	// Auto-adjusted parameters (calculated internally based on size)
	int oversampleH;               // Horizontal oversampling (auto: 3 for small, 2 for large)
	int oversampleV;               // Vertical oversampling (auto: 3 for small, 2 for large)
	float rasterizerMultiply;      // Brightness multiplier (auto: higher for small fonts)
	int outlineThickness;          // Outline thickness (auto: ~5% of size if enabled)
	bool pixelSnapH;               // Snap horizontally to pixel grid (auto: on for small fonts)
	EFreeTypeQuality quality;      // FreeType rendering quality (auto: based on size)

	SFontConfig()
		: size(14.0f)
		, enableOutline(false)
		, oversampleH(4)           // Will be auto-adjusted in __LoadFont
		, oversampleV(4)           // Will be auto-adjusted in __LoadFont
		, rasterizerMultiply(1.2f) // Will be auto-adjusted in __LoadFont
		, outlineThickness(0)      // Will be auto-calculated if enableOutline=true
		, pixelSnapH(false)        // Will be auto-adjusted in __LoadFont
		, quality(EFreeTypeQuality::HighQuality)
	{}
};

// C++20: Font entry (aggregate type for designated initializers)
struct SFontEntry
{
	ImFont* pFont = nullptr;
	float fontSize = 14.0f;
	int outlineThickness = 1;  // From OUTLINE_THICKNESS in FreeTypeFont.txt
};

// C++20: Transparent hash for heterogeneous lookup (zero-allocation string_view→string)
struct StringHash
{
	using is_transparent = void; // Enable heterogeneous lookup
	using hash_type = std::hash<std::string_view>;

	size_t operator()(std::string_view sv) const noexcept { return hash_type{}(sv); }
	size_t operator()(const std::string& s) const noexcept { return hash_type{}(s); }
	size_t operator()(const char* s) const noexcept { return hash_type{}(s); }
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
	[[nodiscard]] ImFont* GetFont(std::string_view fontName = "") const;

	// Frame lifecycle
	void BeginFrame() noexcept;
	void EndFrame() noexcept;
	void Render() noexcept;

	// Flush and restart frame (for rendering windows in correct z-order)
	void FlushAndRestart() noexcept;

	// C++20: Render layer control for z-ordering
	enum class ERenderLayer : int
	{
		Background = 0,  // Behind UI elements (player names, damage text, etc.)
		Foreground = 1,  // On top of everything (UI text)
	};

	// Text rendering functions
	void RenderText(std::string_view text, float x, float y, unsigned long color, bool shadow = false, ERenderLayer layer = ERenderLayer::Background);
	void RenderTextW(std::wstring_view text, float x, float y, unsigned long color, bool shadow = false, ERenderLayer layer = ERenderLayer::Background);
	void RenderTextWithOutline(std::string_view text, float x, float y, unsigned long textColor, unsigned long outlineColor, ERenderLayer layer = ERenderLayer::Background);
	void RenderTextWithOutlineW(std::wstring_view text, float x, float y, unsigned long textColor, unsigned long outlineColor, ERenderLayer layer = ERenderLayer::Background);

	// Advanced rendering with specific font
	void RenderTextEx(std::string_view fontName, std::string_view text, float x, float y, unsigned long color, bool shadow = false);
	void RenderTextWithOutlineEx(std::string_view fontName, std::string_view text, float x, float y, unsigned long textColor, unsigned long outlineColor);

	// Text measurement
	void GetTextExtent(std::string_view text, int* width, int* height, std::string_view fontName = "") const;
	void GetTextExtentW(std::wstring_view text, int* width, int* height, std::string_view fontName = "") const;

	// C++20: UTF-8 conversion helper (made public for GrpTextInstance)
	[[nodiscard]] static std::string WideToUtf8(std::wstring_view text);

	// C++20: DirectX rendering support for z-buffer (glyph data structure)
	struct SGlyphInfo
	{
		float x0, y0, x1, y1;  // Screen coordinates
		float u0, v0, u1, v1;  // Texture UV coordinates
		float advanceX;        // Horizontal advance
	};

	// Get glyph data for DirectX rendering with z-buffer support
	[[nodiscard]] bool GetGlyphInfo(wchar_t character, SGlyphInfo& outInfo, std::string_view fontName = "") const;
	[[nodiscard]] LPDIRECT3DTEXTURE9 GetFontTexture(std::string_view fontName = "") const;

	// Device management
	void OnLostDevice() noexcept;
	void OnResetDevice() noexcept;

	// Accessors
	[[nodiscard]] bool IsInitialized() const noexcept { return m_bInitialized; }

	// Singleton access (modern C++20 thread-safe implementation)
	static CImGuiManager& Instance();

private:
	bool __LoadFont(const SFontConfig& config);
	bool __ParseConfigFile(std::string_view configPath, std::vector<SFontConfig>& outConfigs);
	[[nodiscard]] static constexpr unsigned int __ConvertD3DColorToImGuiColor(unsigned long d3dColor) noexcept;

#ifdef IMGUI_ENABLE_FREETYPE
	// C++20: constexpr conversion from our enum to ImGui FreeType flags
	[[nodiscard]] static constexpr unsigned int __ConvertToFreeTypeFlags(EFreeTypeQuality quality) noexcept;
#endif

	HWND m_hWnd;
	LPDIRECT3DDEVICE9 m_pDevice;
	bool m_bInitialized;

	// C++20: Heterogeneous lookup - allows find() with string_view without allocation
	std::unordered_map<std::string, SFontEntry, StringHash, std::equal_to<>> m_mapFonts;
	std::string m_strActiveFontName;
};
