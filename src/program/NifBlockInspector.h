/*
BodySlide and Outfit Studio
See the included LICENSE file
*/

#pragma once

#include "../../lib/nifly/include/NifFile.hpp"
#include "../../lib/nifly/include/Geometry.hpp"
#include "../../lib/nifly/include/Objects.hpp"

#include <wx/dialog.h>
#include <wx/treectrl.h>
#include <wx/srchctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/checkbox.h>
#include <wx/button.h>

// ── NifBlockInspector ────────────────────────────────────────────────────────
// A non-modal floating dialog that enumerates every block in the work NIF
// (and optionally the reference NIF) and shows it as an expandable tree —
// similar to NifSkope's block list panel.
//
// Usage (from OutfitStudio):
//   if (!blockInspector)
//       blockInspector = new NifBlockInspector(this);
//   blockInspector->RefreshNIF(workNif, refNif);
//   blockInspector->Show();
//   blockInspector->Raise();
//
// The dialog destroys itself when closed (wxCLOSE_BOX); the owner should
// null the pointer via the EVT_CLOSE handler forwarded to it.

class NifBlockInspector : public wxDialog {
public:
	NifBlockInspector(wxWindow* parent);

	// Populate (or repopulate) the tree from the supplied NIFs.
	// Pass nullptr for either argument to skip that NIF.
	void RefreshNIF(nifly::NifFile* workNif, nifly::NifFile* refNif = nullptr);

private:
	wxTreeCtrl*   tree   = nullptr;
	wxSearchCtrl* search = nullptr;
	wxCheckBox*   chkRef = nullptr;
	wxStaticText* lblCount = nullptr;

	// Raw pointers — owned by OutfitProject, not by this dialog.
	nifly::NifFile* lastWorkNif = nullptr;
	nifly::NifFile* lastRefNif  = nullptr;

	void BuildTree();
	void AppendNifBlocks(const wxTreeItemId& root,
						 nifly::NifFile* nif,
						 const wxString& rootLabel,
						 const wxString& filterText);

	void OnSearch(wxCommandEvent&);
	void OnRefToggle(wxCommandEvent&);
	void OnClose(wxCloseEvent&);

	wxDECLARE_EVENT_TABLE();
};
