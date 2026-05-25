/*
BodySlide and Outfit Studio
See the included LICENSE file
*/

#include "NifBlockInspector.h"

#include "../../lib/nifly/include/BasicTypes.hpp"
#include "../../lib/nifly/include/Shaders.hpp"

#include <wx/sizer.h>
#include <wx/stattext.h>

using namespace nifly;

// ── Event table ─────────────────────────────────────────────────────────────
wxBEGIN_EVENT_TABLE(NifBlockInspector, wxDialog)
	EVT_SEARCH(wxID_ANY,          NifBlockInspector::OnSearch)
	EVT_SEARCH_CANCEL(wxID_ANY,   NifBlockInspector::OnSearch)
	EVT_CHECKBOX(wxID_ANY,        NifBlockInspector::OnRefToggle)
	EVT_CLOSE(                    NifBlockInspector::OnClose)
wxEND_EVENT_TABLE()

// ── Construction ─────────────────────────────────────────────────────────────
NifBlockInspector::NifBlockInspector(wxWindow* parent)
	: wxDialog(parent, wxID_ANY, _("NIF Block Inspector"),
			   wxDefaultPosition, wxSize(520, 680),
			   wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER) {

	// ── Top bar: search + reference toggle ──────────────────────────────────
	auto* topSizer = new wxBoxSizer(wxHORIZONTAL);

	search = new wxSearchCtrl(this, wxID_ANY, wxEmptyString,
							  wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	search->SetDescriptiveText(_("Filter blocks…"));
	search->ShowSearchButton(true);
	search->ShowCancelButton(true);
	topSizer->Add(search, 1, wxEXPAND | wxRIGHT, 4);

	chkRef = new wxCheckBox(this, wxID_ANY, _("Show reference NIF"));
	chkRef->SetValue(true);
	topSizer->Add(chkRef, 0, wxALIGN_CENTER_VERTICAL);

	// ── Tree ────────────────────────────────────────────────────────────────
	tree = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
						  wxTR_LINES_AT_ROOT | wxTR_HAS_BUTTONS |
						  wxTR_HIDE_ROOT | wxTR_SINGLE | wxBORDER_SUNKEN);

	// ── Status bar ──────────────────────────────────────────────────────────
	lblCount = new wxStaticText(this, wxID_ANY, wxEmptyString);

	// ── Layout ──────────────────────────────────────────────────────────────
	auto* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(topSizer, 0, wxEXPAND | wxALL, 4);
	mainSizer->Add(tree,     1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 4);
	mainSizer->Add(lblCount, 0, wxLEFT | wxBOTTOM, 6);
	SetSizerAndFit(mainSizer);

	// Don't call Fit() — the dialog has a fixed default size above.
	SetSize(520, 680);
}

// ── Public API ───────────────────────────────────────────────────────────────
void NifBlockInspector::RefreshNIF(NifFile* workNif, NifFile* refNif) {
	lastWorkNif = workNif;
	lastRefNif  = refNif;
	BuildTree();
}

// ── Private helpers ──────────────────────────────────────────────────────────

// Returns a short human-readable description for a block.
static wxString BlockDetail(NiObject* block, [[maybe_unused]] NifFile* nif) {
	if (!block) return wxEmptyString;

	// BSGeometry: mesh count + internal/external state
	if (auto* bsgeo = dynamic_cast<BSGeometry*>(block)) {
		uint8_t n = bsgeo->MeshCount();
		wxString detail = wxString::Format("Meshes: %u  [%s]",
			(unsigned)n,
			bsgeo->HasInternalGeomData() ? "Internal" : "External");
		return detail;
	}

	// NiAVObject-derived: show translation briefly
	if (auto* av = dynamic_cast<NiAVObject*>(block)) {
		(void)av; // translation access varies; keep it simple
	}

	return wxEmptyString;
}

// Append one NifFile's blocks under a parent tree node, applying optional
// case-insensitive text filter.
void NifBlockInspector::AppendNifBlocks(const wxTreeItemId& root,
										NifFile* nif,
										const wxString& rootLabel,
										const wxString& filterText) {
	if (!nif || !nif->IsValid())
		return;

	auto& hdr = nif->GetHeader();
	const uint32_t numBlocks = hdr.GetNumBlocks();

	wxTreeItemId nifRoot = tree->AppendItem(root,
		rootLabel + wxString::Format("  (%u blocks)", numBlocks));

	bool anyVisible = false;
	const bool filtering = !filterText.IsEmpty();

	for (uint32_t i = 0; i < numBlocks; i++) {
		auto* block = hdr.GetBlock<NiObject>(i);
		if (!block) continue;

		wxString typeName   = wxString::FromUTF8(block->GetBlockName());
		wxString blockName;
		if (auto* named = dynamic_cast<NiObjectNET*>(block))
			blockName = wxString::FromUTF8(named->name.get());

		wxString detail = BlockDetail(block, nif);

		// Build display label
		wxString nameStr = typeName + (blockName.IsEmpty() ? wxString() : "  \"" + blockName + "\"");
		wxString label = wxString::Format("[%03u]  ", i) + nameStr + "  " + detail;

		// Filter
		if (filtering) {
			wxString lower = label.Lower();
			if (!lower.Contains(filterText.Lower()))
				continue;
		}

		wxTreeItemId blockNode = tree->AppendItem(nifRoot, label);
		anyVisible = true;

		// ── BSGeometry: expand mesh slots ────────────────────────────────────
		if (auto* bsgeo = dynamic_cast<BSGeometry*>(block)) {
			for (uint8_t m = 0; m < bsgeo->MeshCount(); m++) {
				BSGeometryMesh* mesh = bsgeo->SelectMesh(m);
				wxString meshPath   = mesh ? wxString::FromUTF8(mesh->meshName.get()) : wxEmptyString;
				bsgeo->ReleaseMesh();

				const char* lodTag =
					m == 0 ? "LOD0 (main)" :
					m == 1 ? "LOD1"        :
					m == 2 ? "LOD2"        : "LOD3";

				if (meshPath.IsEmpty())
					meshPath = wxString::Format("<slot %u - no path>", (unsigned)m);

				tree->AppendItem(blockNode,
					wxString::Format("  Mesh[%u]  ", (unsigned)m) + meshPath + "  (" + lodTag + ")");
			}
		}

		// ── NiNode: show child count ──────────────────────────────────────────
		if (auto* node = dynamic_cast<NiNode*>(block)) {
			std::vector<NiRef*> childRefs;
			std::set<NiRef*> childRefSet;
			node->GetChildRefs(childRefSet);
			childRefs.assign(childRefSet.begin(), childRefSet.end());
			if (!childRefs.empty())
				tree->AppendItem(blockNode,
					wxString::Format("  Children: %zu", childRefs.size()));
		}

		// ── BSLightingShaderProperty: shader type ─────────────────────────────
		if (auto* shader = dynamic_cast<BSLightingShaderProperty*>(block)) {
			tree->AppendItem(blockNode,
				wxString::Format("  Shader type: %u", shader->GetShaderType()));
		}
	}

	if (!anyVisible && filtering)
		tree->AppendItem(nifRoot, _("  (no blocks match filter)"));

	tree->Expand(nifRoot);
}

void NifBlockInspector::BuildTree() {
	tree->DeleteAllItems();
	wxTreeItemId hiddenRoot = tree->AddRoot(wxEmptyString);

	const wxString filterText = search ? search->GetValue() : wxEmptyString;

	uint32_t totalBlocks = 0;

	if (lastWorkNif && lastWorkNif->IsValid()) {
		AppendNifBlocks(hiddenRoot, lastWorkNif, _("Work NIF"), filterText);
		totalBlocks += lastWorkNif->GetHeader().GetNumBlocks();
	}

	if (chkRef && chkRef->GetValue() && lastRefNif && lastRefNif->IsValid()) {
		AppendNifBlocks(hiddenRoot, lastRefNif, _("Reference NIF"), filterText);
		totalBlocks += lastRefNif->GetHeader().GetNumBlocks();
	}

	if (lblCount)
		lblCount->SetLabel(wxString::Format(_("Total blocks shown: %u"), totalBlocks));
}

// ── Event handlers ───────────────────────────────────────────────────────────
void NifBlockInspector::OnSearch(wxCommandEvent&) {
	BuildTree();
}

void NifBlockInspector::OnRefToggle(wxCommandEvent&) {
	BuildTree();
}

void NifBlockInspector::OnClose(wxCloseEvent& event) {
	// Hide instead of destroy so OutfitStudio can re-show it cheaply.
	Hide();
	event.Veto();
}
