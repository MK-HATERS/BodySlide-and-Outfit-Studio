/*
BodySlide and Outfit Studio
See the included LICENSE file
*/

#pragma once

#include <wx/wx.h>
#include <wx/xrc/xmlres.h>

#include <vector>

#include "OutfitProject.h"

class OutfitStudioFrame;
class ConfigurationManager;
class RefTemplate;

class ConvertBodyReferenceDialog : public wxWizard {
public:
	ConvertBodyReferenceDialog(OutfitStudioFrame* outfitStudio, OutfitProject* project, ConfigurationManager& config, const std::vector<RefTemplate>& refTemplates);
	~ConvertBodyReferenceDialog() override;

	bool Load();
	void ConvertBodyReference() const;

private:
	OutfitStudioFrame* outfitStudio = nullptr;
	OutfitProject* project = nullptr;
	ConfigurationManager& config;
	const std::vector<RefTemplate>& refTemplates;
	wxWizardPage* pg1 = nullptr;
	wxWizardPage* pg2 = nullptr;

	wxChoice* npConvRefChoice = nullptr;
	wxCheckBox* chkConformSliders = nullptr;
	wxCheckBox* chkSkipConformPopup = nullptr;
	wxCheckBox* chkCopyBoneWeights = nullptr;
	wxCheckBox* chkSkipCopyBonesPopup = nullptr;

	int LoadReferenceTemplate(const wxString& refTemplate, bool mergeSliders, bool mergeZaps) const;
	bool AlertProgressError(int error, const wxString& title, const wxString& message) const;

	// Resolves the input NIF file path for a given reference template name.
	std::string GetNifPathFromTemplate(const wxString& templateName) const;
	// Returns the shape name (e.g. "Naked_M:0") stored in a reference template.
	std::string GetShapeNameFromTemplate(const wxString& templateName) const;

	wxDECLARE_EVENT_TABLE();
};
