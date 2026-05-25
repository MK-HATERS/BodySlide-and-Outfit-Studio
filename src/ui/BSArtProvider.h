/*
BodySlide and Outfit Studio
See the included LICENSE file
*/
#pragma once

#include <wx/artprov.h>

// ── BodySlide art IDs ─────────────────────────────────────────────────────────
// Use these with wxArtProvider::GetBitmap(BS_ART_BUILD, wxART_BUTTON, sz).
#define BS_ART_BUILD        wxART_MAKE_ART_ID(BS_BUILD)
#define BS_ART_BATCH        wxART_MAKE_ART_ID(BS_BATCH)
#define BS_ART_PREVIEW      wxART_MAKE_ART_ID(BS_PREVIEW)
#define BS_ART_SAVE         wxART_MAKE_ART_ID(BS_SAVE)
#define BS_ART_SAVE_AS      wxART_MAKE_ART_ID(BS_SAVE_AS)
#define BS_ART_SETTINGS     wxART_MAKE_ART_ID(BS_SETTINGS)
#define BS_ART_GROUP        wxART_MAKE_ART_ID(BS_GROUP)
#define BS_ART_OUTFIT_STUDIO wxART_MAKE_ART_ID(BS_OUTFIT_STUDIO)
#define BS_ART_HIGH_TO_LOW  wxART_MAKE_ART_ID(BS_HIGH_TO_LOW)
#define BS_ART_LOW_TO_HIGH  wxART_MAKE_ART_ID(BS_LOW_TO_HIGH)
#define BS_ART_FAVORITE     wxART_MAKE_ART_ID(BS_FAVORITE)
#define BS_ART_FAVORITE_OFF wxART_MAKE_ART_ID(BS_FAVORITE_OFF)
#define BS_ART_LOG_CLEAR    wxART_MAKE_ART_ID(BS_LOG_CLEAR)

// Flat monochrome icon provider drawn with wxDC.
// Register once at app startup:
//   wxArtProvider::Push(new BSArtProvider());
class BSArtProvider : public wxArtProvider {
protected:
	wxBitmap CreateBitmap(const wxArtID&    id,
	                      const wxArtClient& client,
	                      const wxSize&      size) override;

private:
	static wxBitmap MakeIcon(const wxSize& sz,
	                         void (*draw)(wxDC&, const wxSize&),
	                         const wxColour& fg);
};
