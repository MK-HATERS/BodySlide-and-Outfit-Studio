/*
BodySlide and Outfit Studio
See the included LICENSE file
*/
#include "BSArtProvider.h"
#include "BSTheme.h"
#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/bitmap.h>
#include <wx/rawbmp.h>

// ── Helper: create a flat icon drawn by a lambda-style function pointer ────────
wxBitmap BSArtProvider::MakeIcon(const wxSize& sz,
                                 void (*draw)(wxDC&, const wxSize&),
                                 const wxColour& fg) {
	wxBitmap bmp(sz.x, sz.y, 32);
	{
		wxMemoryDC dc(bmp);
		dc.SetBackground(wxBrush(gTheme.bgPanel));
		dc.Clear();
		dc.SetPen(wxPen(fg, 1));
		dc.SetBrush(wxBrush(fg));
		draw(dc, sz);
	}
	// Make the bgPanel colour transparent so icons work on any background.
	bmp.UseAlpha();
	return bmp;
}

// ── Icon drawing routines (16x16 logical pixels) ──────────────────────────────

static void DrawBuild(wxDC& dc, const wxSize& sz) {
	// Simple right-pointing triangle (hammer/build symbol)
	int cx = sz.x / 2, cy = sz.y / 2, r = sz.x / 2 - 2;
	wxPoint pts[3] = {
		{cx - r, cy - r},
		{cx - r, cy + r},
		{cx + r, cy    }
	};
	dc.DrawPolygon(3, pts);
}

static void DrawPreview(wxDC& dc, const wxSize& sz) {
	// Eye outline
	int cx = sz.x / 2, cy = sz.y / 2, rx = sz.x / 2 - 2, ry = sz.y / 4;
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.DrawEllipse(cx - rx, cy - ry, rx * 2, ry * 2);
	dc.SetBrush(wxBrush(dc.GetPen().GetColour()));
	dc.DrawCircle(cx, cy, ry - 1);
}

static void DrawSave(wxDC& dc, const wxSize& sz) {
	// Floppy disk outline
	int m = 2;
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.DrawRectangle(m, m, sz.x - 2*m, sz.y - 2*m);
	dc.DrawRectangle(m + 2, m, sz.x - 2*m - 4, sz.y / 3);
	dc.DrawRectangle(m + 2, sz.y / 2, sz.x - 2*m - 4, sz.y / 2 - m);
}

static void DrawSettings(wxDC& dc, const wxSize& sz) {
	// Gear-like circle with spokes
	int cx = sz.x / 2, cy = sz.y / 2, r = sz.x / 2 - 3;
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.DrawCircle(cx, cy, r);
	dc.DrawCircle(cx, cy, r / 2);
	for (int i = 0; i < 4; i++) {
		double a = i * 3.14159 / 4.0;
		int x1 = cx + (int)(r * 0.6 * std::cos(a));
		int y1 = cy + (int)(r * 0.6 * std::sin(a));
		int x2 = cx + (int)(r * 1.1 * std::cos(a));
		int y2 = cy + (int)(r * 1.1 * std::sin(a));
		dc.DrawLine(x1, y1, x2, y2);
	}
}

static void DrawGroup(wxDC& dc, const wxSize& sz) {
	// Two stacked rectangles
	int w = sz.x - 4, h = sz.y / 3;
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.DrawRectangle(2, 1, w, h);
	dc.DrawRectangle(2, 1 + h + 2, w, h);
	dc.DrawRectangle(2, 1 + 2*(h + 2), w, h);
}

static void DrawFavoriteOn(wxDC& dc, const wxSize& sz) {
	// Filled star
	int cx = sz.x / 2, cy = sz.y / 2, r1 = sz.x / 2 - 1, r2 = r1 / 2;
	wxPoint pts[10];
	for (int i = 0; i < 10; i++) {
		double a = i * 3.14159 / 5.0 - 3.14159 / 2.0;
		int r = (i % 2 == 0) ? r1 : r2;
		pts[i] = {cx + (int)(r * std::cos(a)), cy + (int)(r * std::sin(a))};
	}
	dc.DrawPolygon(10, pts);
}

static void DrawFavoriteOff(wxDC& dc, const wxSize& sz) {
	int cx = sz.x / 2, cy = sz.y / 2, r1 = sz.x / 2 - 1, r2 = r1 / 2;
	wxPoint pts[10];
	for (int i = 0; i < 10; i++) {
		double a = i * 3.14159 / 5.0 - 3.14159 / 2.0;
		int r = (i % 2 == 0) ? r1 : r2;
		pts[i] = {cx + (int)(r * std::cos(a)), cy + (int)(r * std::sin(a))};
	}
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.DrawPolygon(10, pts);
}

static void DrawArrowRight(wxDC& dc, const wxSize& sz) {
	int cy = sz.y / 2, m = 3;
	wxPoint pts[3] = {{m, m}, {m, sz.y - m}, {sz.x - m, cy}};
	dc.DrawPolygon(3, pts);
}

static void DrawArrowLeft(wxDC& dc, const wxSize& sz) {
	int cy = sz.y / 2, m = 3;
	wxPoint pts[3] = {{sz.x - m, m}, {sz.x - m, sz.y - m}, {m, cy}};
	dc.DrawPolygon(3, pts);
}

static void DrawClear(wxDC& dc, const wxSize& sz) {
	// X icon
	int m = 3;
	dc.SetPen(wxPen(dc.GetPen().GetColour(), 2));
	dc.DrawLine(m, m, sz.x - m, sz.y - m);
	dc.DrawLine(sz.x - m, m, m, sz.y - m);
}

// ── Main dispatch ─────────────────────────────────────────────────────────────
wxBitmap BSArtProvider::CreateBitmap(const wxArtID&    id,
                                     const wxArtClient& /*client*/,
                                     const wxSize&      requestedSize) {
	wxSize sz = requestedSize.IsFullySpecified() ? requestedSize : wxSize(16, 16);
	wxColour fg = gTheme.fgText;

	if (id == BS_ART_BUILD)        return MakeIcon(sz, DrawBuild,      gTheme.accent);
	if (id == BS_ART_BATCH)        return MakeIcon(sz, DrawBuild,      fg);
	if (id == BS_ART_PREVIEW)      return MakeIcon(sz, DrawPreview,    fg);
	if (id == BS_ART_SAVE)         return MakeIcon(sz, DrawSave,       fg);
	if (id == BS_ART_SAVE_AS)      return MakeIcon(sz, DrawSave,       gTheme.fgMuted);
	if (id == BS_ART_SETTINGS)     return MakeIcon(sz, DrawSettings,   fg);
	if (id == BS_ART_GROUP)        return MakeIcon(sz, DrawGroup,      fg);
	if (id == BS_ART_HIGH_TO_LOW)  return MakeIcon(sz, DrawArrowLeft,  fg);
	if (id == BS_ART_LOW_TO_HIGH)  return MakeIcon(sz, DrawArrowRight, fg);
	if (id == BS_ART_FAVORITE)     return MakeIcon(sz, DrawFavoriteOn, gTheme.colorWarning);
	if (id == BS_ART_FAVORITE_OFF) return MakeIcon(sz, DrawFavoriteOff, gTheme.fgMuted);
	if (id == BS_ART_LOG_CLEAR)    return MakeIcon(sz, DrawClear,      gTheme.colorError);

	return wxNullBitmap; // fall back to other providers
}
