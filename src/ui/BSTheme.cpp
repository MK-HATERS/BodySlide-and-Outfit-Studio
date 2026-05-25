/*
BodySlide and Outfit Studio
See the included LICENSE file
*/
#include "BSTheme.h"
#include <wx/window.h>

BSTheme gTheme;

void BSTheme_InitDark() {
	// ── Backgrounds ──────────────────────────────────────────────────────────
	gTheme.bgMain      = wxColour( 30,  30,  30);
	gTheme.bgPanel     = wxColour( 37,  37,  38);
	gTheme.bgAlt       = wxColour( 45,  45,  48);
	gTheme.bgHover     = wxColour( 62,  62,  66);
	gTheme.bgSelected  = wxColour(  9,  71, 113);

	// ── Foregrounds ──────────────────────────────────────────────────────────
	gTheme.fgText      = wxColour(212, 212, 212);
	gTheme.fgMuted     = wxColour(133, 133, 133);
	gTheme.fgDisabled  = wxColour( 85,  85,  85);

	// ── Accent ───────────────────────────────────────────────────────────────
	gTheme.accent      = wxColour(  0, 120, 212);
	gTheme.accentHover = wxColour( 16, 132, 216);

	// ── Borders ──────────────────────────────────────────────────────────────
	gTheme.border      = wxColour( 63,  63,  70);
	gTheme.borderStrong= wxColour(107, 107, 107);

	// ── Slider ───────────────────────────────────────────────────────────────
	gTheme.sliderTrack  = wxColour( 63,  63,  70);
	gTheme.sliderFill   = wxColour(  0, 120, 212);
	gTheme.sliderHandle = wxColour(212, 212, 212);

	// ── Status ───────────────────────────────────────────────────────────────
	gTheme.colorSuccess = wxColour( 78, 201, 176);
	gTheme.colorWarning = wxColour(220, 220,   0);
	gTheme.colorError   = wxColour(244,  71,  71);

	// ── Fonts ─────────────────────────────────────────────────────────────────
	wxFont base(9, wxFONTFAMILY_DEFAULT,
	            wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
	            false, "Segoe UI");
	if (!base.IsOk())
		base = *wxNORMAL_FONT;

	gTheme.fontMain = base;
	gTheme.fontBold = base;
	gTheme.fontBold.SetWeight(wxFONTWEIGHT_BOLD);

	gTheme.fontMono = wxFont(9, wxFONTFAMILY_TELETYPE,
	                         wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL,
	                         false, "Consolas");
	if (!gTheme.fontMono.IsOk())
		gTheme.fontMono = *wxNORMAL_FONT;
}

void BSTheme_Apply(wxWindow* w) {
	if (!w) return;
	w->SetBackgroundColour(gTheme.bgPanel);
	w->SetForegroundColour(gTheme.fgText);
	w->SetFont(gTheme.fontMain);
}
