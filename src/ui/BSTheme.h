/*
BodySlide and Outfit Studio
See the included LICENSE file
*/
#pragma once

#include <wx/colour.h>
#include <wx/font.h>

class wxWindow; // forward-declare to avoid pulling in the full hierarchy

// ── Global dark theme ─────────────────────────────────────────────────────────
// All colours are #RRGGBB equivalents of the VS Code Dark+ palette.
// Apply with panel->SetBackgroundColour(gTheme.bgPanel) etc.
struct BSTheme {
	// Backgrounds
	wxColour bgMain;        // #1e1e1e  frame / outermost
	wxColour bgPanel;       // #252526  most panels
	wxColour bgAlt;         // #2d2d30  alternating rows, input fields
	wxColour bgHover;       // #3e3e42  mouse-over highlight
	wxColour bgSelected;    // #094771  selected item

	// Foregrounds
	wxColour fgText;        // #d4d4d4  primary text
	wxColour fgMuted;       // #858585  secondary / labels
	wxColour fgDisabled;    // #555555  disabled controls

	// Accent
	wxColour accent;        // #0078d4  buttons, slider fill, links
	wxColour accentHover;   // #1084d8

	// Borders
	wxColour border;        // #3f3f46  subtle panel separator
	wxColour borderStrong;  // #6b6b6b  visible border

	// Slider-specific
	wxColour sliderTrack;   // #3f3f46
	wxColour sliderFill;    // #0078d4
	wxColour sliderHandle;  // #d4d4d4

	// Status colours
	wxColour colorSuccess;  // #4ec9b0
	wxColour colorWarning;  // #dcdcaa
	wxColour colorError;    // #f44747

	// Fonts
	wxFont fontMain;        // Segoe UI 9 (Windows) / system sans
	wxFont fontBold;        // same, Bold
	wxFont fontMono;        // Consolas 9 / monospace
};

// Global singleton — initialised by BSTheme_InitDark() at app startup.
extern BSTheme gTheme;

// Populate gTheme with dark values and resolve fonts.
void BSTheme_InitDark();

// Convenience: apply background + foreground to any window.
void BSTheme_Apply(wxWindow* w);
