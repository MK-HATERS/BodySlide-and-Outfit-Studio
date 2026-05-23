/*
BodySlide and Outfit Studio
See the included LICENSE file
*/

#include "ConvertBodyReferenceDialog.h"

#include <NifUtil.hpp>

#include "../utils/ConfigDialogUtil.h"
#include "../utils/ConfigurationManager.h"
#include "../utils/PlatformUtil.h"

#include <fstream>
#include <regex>
#ifdef _WIN32
#include <windows.h>
#endif

#include "OutfitStudio.h"

using namespace std;
class RefTemplate;
extern ConfigurationManager Config;

wxBEGIN_EVENT_TABLE(ConvertBodyReferenceDialog, wxWizard)

wxEND_EVENT_TABLE()

const char* CONVERT_SLIDER_PREFIX = "Convert";
const char* SLIDER_SET_PREFIX = "Sliders";


ConvertBodyReferenceDialog::ConvertBodyReferenceDialog(OutfitStudioFrame* outfitStudio,
													   OutfitProject* project,
													   ConfigurationManager& config,
													   const std::vector<RefTemplate>& refTemplates)
	: outfitStudio(outfitStudio)
	, project(project)
	, config(config)
	, refTemplates(refTemplates)
	, pg1(nullptr)
	, pg2(nullptr) {
	wxXmlResource* xrc = wxXmlResource::Get();
	xrc->Load(wxString::FromUTF8(Config["AppDir"]) + "/res/xrc/ConvertBodyReference.xrc");
	xrc->LoadObject(this, outfitStudio, "wizConvertBodyRef", "wxWizard");

	pg1 = (wxWizardPage*)XRCCTRL(*this, "wizpgConvertBodyRef1", wxWizardPageSimple);
	pg2 = (wxWizardPage*)XRCCTRL(*this, "wizpgConvertBodyRef2", wxWizardPageSimple);

	npConvRefChoice = XRCCTRL((*this), "npConvRefChoice", wxChoice);
	npConvRefChoice->Append("None");

	chkConformSliders = XRCCTRL((*this), "chkConformSliders", wxCheckBox);
	chkSkipConformPopup = XRCCTRL((*this), "chkSkipConformPopup", wxCheckBox);
	chkCopyBoneWeights = XRCCTRL((*this), "chkCopyBoneWeights", wxCheckBox);
	chkSkipCopyBonesPopup = XRCCTRL((*this), "chkSkipCopyBonesPopup", wxCheckBox);

	std::vector<RefTemplate> converters = refTemplates;
	std::vector<RefTemplate> bodies = refTemplates;

	auto sortTemplates = [&](const RefTemplate& first, const RefTemplate& second, vector<string> prioritizeNames, bool ascendingPriority) {
		const bool firstHasPriorityName = std::any_of(prioritizeNames.begin(), prioritizeNames.end(), [first](const string& name) {
			return strstr(first.GetName().c_str(), name.c_str());
		});
		const bool secondHasPriorityName = std::any_of(prioritizeNames.begin(), prioritizeNames.end(), [second](const string& name) {
			return strstr(second.GetName().c_str(), name.c_str());
		});
		if (firstHasPriorityName == secondHasPriorityName)
			return first.GetName().compare(second.GetName()) <= 0;
		return static_cast<bool>((!firstHasPriorityName && secondHasPriorityName) ^ ascendingPriority);
	};


	std::sort(converters.begin(), converters.end(), [&](const RefTemplate& first, const RefTemplate& second) {
		return sortTemplates(first, second, {CONVERT_SLIDER_PREFIX}, true);
	});
	std::sort(bodies.begin(), bodies.end(), [&](const RefTemplate& first, const RefTemplate& second) {
		return sortTemplates(first, second, {CONVERT_SLIDER_PREFIX, SLIDER_SET_PREFIX}, false);
	});

	ConfigDialogUtil::LoadDialogChoices(config, (*this), "ConvertBodyReference", "npConvRefChoice", converters);
	ConfigDialogUtil::LoadDialogChoices(config, (*this), "ConvertBodyReference", "npNewRefChoice", bodies);
	ConfigDialogUtil::LoadDialogText(config, (*this), "ConvertBodyReference", "npRemoveText");
	ConfigDialogUtil::LoadDialogText(config, (*this), "ConvertBodyReference", "npAppendText");
	ConfigDialogUtil::LoadDialogText(config, (*this), "ConvertBodyReference", "npDeleteShapesText");
	ConfigDialogUtil::LoadDialogText(config, (*this), "ConvertBodyReference", "npAddBonesText");
	ConfigDialogUtil::LoadDialogCheckBox(config, (*this), "ConvertBodyReference", "chkConvertMergeSliders");
	ConfigDialogUtil::LoadDialogCheckBox(config, (*this), "ConvertBodyReference", "chkConvertMergeZaps");
	ConfigDialogUtil::LoadDialogCheckBox(config, (*this), "ConvertBodyReference", "chkConformSliders");
	ConfigDialogUtil::LoadDialogCheckBox(config, (*this), "ConvertBodyReference", "chkSkipConformPopup");
	ConfigDialogUtil::LoadDialogCheckBox(config, (*this), "ConvertBodyReference", "chkCopyBoneWeights");
	ConfigDialogUtil::LoadDialogCheckBox(config, (*this), "ConvertBodyReference", "chkSkipCopyBonesPopup");
	ConfigDialogUtil::LoadDialogCheckBox(config, (*this), "ConvertBodyReference", "chkDeleteReferenceOnComplete");

	if (!chkConformSliders->IsChecked())
		chkSkipConformPopup->Disable();

	if (!chkCopyBoneWeights->IsChecked())
		chkSkipCopyBonesPopup->Disable();

	chkConformSliders->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& event) {
		bool checked = event.IsChecked();
		chkSkipConformPopup->Enable(checked);
	});

	chkCopyBoneWeights->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& event) {
		bool checked = event.IsChecked();
		chkSkipCopyBonesPopup->Enable(checked);
	});

	SetDoubleBuffered(true);
	CenterOnParent();
}

ConvertBodyReferenceDialog::~ConvertBodyReferenceDialog() {
	wxXmlResource::Get()->Unload(wxString::FromUTF8(Config["AppDir"]) + "/res/xrc/ConvertBodyReferenceDialog.xrc");
}

bool ConvertBodyReferenceDialog::Load() {
	FitToPage(pg1);
	return RunWizard(pg1);
}

std::string ConvertBodyReferenceDialog::GetNifPathFromTemplate(const wxString& templateName) const {
	std::string tmplName{templateName.ToUTF8()};
	auto tmpl = find_if(refTemplates.begin(), refTemplates.end(), [&tmplName](const RefTemplate& rt) { return rt.GetName() == tmplName; });
	if (tmpl == refTemplates.end())
		return {};

	std::string sourcePath;
	if (wxFileName(wxString::FromUTF8(tmpl->GetSource())).IsRelative())
		sourcePath = GetProjectPath() + PathSepStr + tmpl->GetSource();
	else
		sourcePath = tmpl->GetSource();

	SliderSetFile sset(sourcePath);
	if (sset.fail())
		return {};

	SliderSet refSet;
	if (sset.GetSet(tmpl->GetSetName(), refSet))
		return {};

	refSet.SetBaseDataPath(GetProjectPath() + PathSepStr + "ShapeData");
	return refSet.GetInputFileName();
}

std::string ConvertBodyReferenceDialog::GetShapeNameFromTemplate(const wxString& templateName) const {
	std::string tmplName{templateName.ToUTF8()};
	auto tmpl = find_if(refTemplates.begin(), refTemplates.end(), [&tmplName](const RefTemplate& rt) { return rt.GetName() == tmplName; });
	if (tmpl == refTemplates.end())
		return {};

	return tmpl->GetShape();
}

void ConvertBodyReferenceDialog::ConvertBodyReference() const {
	try {

#define CBR_LOG(msg, ...) do { wxLogMessage("[CBR] " msg, ##__VA_ARGS__); wxLog::FlushActive(); } while(0)
// Heap-validation helper — remove once CBR crash is fixed.
#ifdef _WIN32
#define CBR_HV(label) do { \
	BOOL _hv = HeapValidate(GetProcessHeap(), 0, NULL); \
	wxLogMessage("[HV] CBR/" label ": heap %s", _hv ? wxString("OK") : wxString("CORRUPTED")); \
	wxLog::FlushActive(); \
} while(0)
#else
#define CBR_HV(label) do {} while(0)
#endif

	CBR_LOG("ConvertBodyReference: started");
	CBR_HV("wizard entry");
	outfitStudio->StartProgress(_("Starting conversion..."));

	bool mergeSliders = ConfigDialogUtil::SetBoolFromDialogCheckbox(config, (*this), "ConvertBodyReference", "chkConvertMergeSliders");
	bool mergeZaps = ConfigDialogUtil::SetBoolFromDialogCheckbox(config, (*this), "ConvertBodyReference", "chkConvertMergeZaps");
	bool conformSliders = ConfigDialogUtil::SetBoolFromDialogCheckbox(config, (*this), "ConvertBodyReference", "chkConformSliders");
	bool skipConformPopup = ConfigDialogUtil::SetBoolFromDialogCheckbox(config, (*this), "ConvertBodyReference", "chkSkipConformPopup");
	bool copyBoneWeights = ConfigDialogUtil::SetBoolFromDialogCheckbox(config, (*this), "ConvertBodyReference", "chkCopyBoneWeights");
	bool skipCopyBonesPopup = ConfigDialogUtil::SetBoolFromDialogCheckbox(config, (*this), "ConvertBodyReference", "chkSkipCopyBonesPopup");
	bool deleteReferenceOnCompleted = ConfigDialogUtil::SetBoolFromDialogCheckbox(config, (*this), "ConvertBodyReference", "chkDeleteReferenceOnComplete");
	auto conversionRefTemplate = ConfigDialogUtil::SetStringFromDialogChoice(config, (*this), "ConvertBodyReference", "npConvRefChoice");
	auto newRefTemplate = ConfigDialogUtil::SetStringFromDialogChoice(config, (*this), "ConvertBodyReference", "npNewRefChoice");
	auto removeFromProjectText = ConfigDialogUtil::SetStringFromDialogTextControl(config, (*this), "ConvertBodyReference", "npRemoveText");
	auto appendToProjectText = ConfigDialogUtil::SetStringFromDialogTextControl(config, (*this), "ConvertBodyReference", "npAppendText");
	auto deleteShapesText = ConfigDialogUtil::SetStringFromDialogTextControl(config, (*this), "ConvertBodyReference", "npDeleteShapesText");
	auto addBonesText = ConfigDialogUtil::SetStringFromDialogTextControl(config, (*this), "ConvertBodyReference", "npAddBonesText");

	CBR_LOG("settings read: convRef='%s' newRef='%s' mergeSliders=%d mergeZaps=%d conform=%d copyBones=%d deleteRefOnComplete=%d",
			conversionRefTemplate, newRefTemplate, (int)mergeSliders, (int)mergeZaps, (int)conformSliders, (int)copyBoneWeights, (int)deleteReferenceOnCompleted);

	Config.SaveConfig(Config["AppDir"] + "/Config.xml");

	outfitStudio->UpdateProgress(1, _("Updating Project Output Settings"));

	if (!removeFromProjectText.IsEmpty()) {
		wxStringTokenizer tkz(removeFromProjectText, wxT(","));
		bool modifiedName = false;
		while (tkz.HasMoreTokens()) {
			wxString token = tkz.GetNextToken();
			if (!modifiedName && !appendToProjectText.IsEmpty() && project->mFileName.Contains(token)) {
				project->mFileName.Replace(token, appendToProjectText);
				modifiedName = true;
			}
			else {
				project->mFileName.Replace(token, "");
			}
			project->mOutfitName.Replace(token, "");
			project->mDataDir.Replace(token, "");
			project->mBaseFile.Replace(token, "");
		}
	}
	if (!appendToProjectText.IsEmpty()) {
		if (project->mOutfitName[0] != ' ')
			project->mOutfitName.Prepend(' ');
		if (project->mDataDir[0] != ' ')
			project->mDataDir.Prepend(' ');
		if (project->mBaseFile[0] != ' ')
			project->mBaseFile.Prepend(' ');
		project->mOutfitName.Prepend(appendToProjectText);
		project->mDataDir.Prepend(appendToProjectText);
		project->mBaseFile.Prepend(appendToProjectText);

		project->outfitName = project->mOutfitName.ToStdString();
		outfitStudio->UpdateTitle();
	}

	CBR_LOG("calling DeleteSliders (mergeSliders=%d mergeZaps=%d)", (int)mergeSliders, (int)mergeZaps);
	outfitStudio->DeleteSliders(mergeSliders, mergeZaps); // we need to do this first so we can clear any broken sliders

	CBR_LOG("calling ResetTransforms");
	project->ResetTransforms();

	CBR_LOG("calling GetWorkNif()->GetShapes() for original shapes");
	auto workNif = project->GetWorkNif();
	if (!workNif) {
		wxLogError("[CBR] GetWorkNif() returned null — aborting");
		wxLog::FlushActive();
		outfitStudio->EndProgress("", true);
		return;
	}
	auto originalShapes = workNif->GetShapes(); // get outfit shapes
	CBR_LOG("got %zu original shapes", originalShapes.size());

	if (!deleteShapesText.IsEmpty()) {
		outfitStudio->UpdateProgress(5, _("Deleting Shapes..."));
		wxStringTokenizer tkz(deleteShapesText, wxT(","));

		while (tkz.HasMoreTokens()) {
			wxString token = tkz.GetNextToken();
			for (auto& shape : originalShapes) {
				if (shape == nullptr)
					continue;
				auto shapeName = wxString(shape->name.get().c_str());
				if (shapeName.Contains(token)) {
					CBR_LOG("deleting shape '%s' (matched token '%s')", shapeName, token);
					project->DeleteShape(shape);
					shape = nullptr;
				}
			}
		}
	}

	CBR_LOG("deleting base shape");
	project->DeleteShape(project->GetBaseShape());

	// getOutfitShapes() — returns fresh NiShape* for every shape that is NOT the
	// current base/reference shape.  MUST be called AFTER each LoadReferenceTemplate
	// so the base pointer is up-to-date.
	// Rationale: LoadReferenceTemplate can call DeleteShape on an existing shape that
	// shares a name with the incoming reference, then clone a fresh copy.  Any
	// pointer captured before that call becomes dangling; dereferencing it (even to
	// read the name) is undefined behaviour and causes an access-violation that
	// bypasses all C++ catch blocks (Windows SEH → OnFatalException).
	auto getOutfitShapes = [&]() -> std::vector<NiShape*> {
		std::vector<NiShape*> outfits;
		auto* nif = project->GetWorkNif();
		if (!nif)
			return outfits;
		for (auto* s : nif->GetShapes())
			if (s && !project->IsBaseShape(s))
				outfits.push_back(s);
		return outfits;
	};

	{
		auto pre = project->GetWorkNif() ? project->GetWorkNif()->GetShapes() : std::vector<NiShape*>{};
		CBR_LOG("shapes in NIF before conversion step: %zu", pre.size());
		for (auto* s : pre)
			CBR_LOG("  '%s' (isBase=%d)", wxString(s->name.get().c_str()), (int)project->IsBaseShape(s));
	}

	// Suppress "deleted shapes" modal dialogs for the duration of the entire wizard.
	// LoadReference shows a wxMessageBox when a shape is deleted as a duplicate.
	// That modal's message pump can re-enter the GL render path while the scene is
	// partially built (new shape cloned into workNif but no GL mesh yet), corrupting
	// the heap.  During the wizard these deletions are always intentional, so we log
	// them instead of showing a modal.
	project->bSuppressLoadWarnings = true;

	if (conversionRefTemplate != "None") {
		outfitStudio->UpdateProgress(5, _("Loading conversion reference..."));
		outfitStudio->StartSubProgress(5, 10);
		CBR_LOG("loading conversion reference template: '%s'", conversionRefTemplate);
		if (AlertProgressError(LoadReferenceTemplate(conversionRefTemplate, mergeSliders, mergeZaps),
							   _("Load Error"), "Failed to load conversion reference"))
			return;
		CBR_HV("after first LoadReferenceTemplate");
		outfitStudio->EndProgress();

		// Derive a FRESH list of outfit shapes now that the conversion reference has
		// been loaded.  Old pointers (captured before LoadReferenceTemplate) may be
		// dangling.
		//
		// IMPORTANT: evaluate BEFORE calling CreateSetSliders / RefreshGUIFromProj.
		// RefreshGUIFromProj triggers MeshesFromProj() which rebuilds GL meshes;
		// in certain configurations (pure body project, reference replaced the only
		// shape) this can corrupt the heap.  When there are no outfit shapes to
		// conform or deform, skip the entire heavy GL rebuild — it serves no purpose.
		auto shapesToConform = getOutfitShapes();
		CBR_LOG("outfit shapes after loading conversion ref: %zu", shapesToConform.size());

		if (!shapesToConform.empty()) {
			CBR_LOG("has outfit shapes — running CreateSetSliders + RefreshGUIFromProj + conform");
			outfitStudio->StartSubProgress(10, 20);
			outfitStudio->CreateSetSliders();
			outfitStudio->RefreshGUIFromProj();

			outfitStudio->UpdateProgress(20, _("Conforming outfit parts..."));
			outfitStudio->StartSubProgress(20, 35);

			// A correct conversion reference should always conform accurately, so
			// skip the default-conform fallback popup.
			CBR_LOG("calling ConformShapes (%zu shapes)", shapesToConform.size());
			if (AlertProgressError(outfitStudio->ConformShapes(shapesToConform, true),
								   _("Conform Error"), "Failed to conform shapes"))
				return;

			outfitStudio->UpdateProgress(35, _("Updating conversion Slider..."));
			// Applying the conversion slider at 100% deforms outfit shapes to the
			// converted position.
			{
				size_t activeSetSz = project->activeSet.size();
				if (activeSetSz > 0) {
					CBR_LOG("calling SetSliderValue(%lu, 100) + ApplySliders", (unsigned long)(activeSetSz - 1));
					outfitStudio->SetSliderValue(activeSetSz - 1, 100);
					outfitStudio->ApplySliders();
					CBR_LOG("SetSliderValue + ApplySliders done");
				}
			}
		}
		else {
			CBR_LOG("no outfit shapes — skipping CreateSetSliders/RefreshGUIFromProj/conform/apply");
		}

		outfitStudio->UpdateProgress(40, _("Setting the base shape and removing the conversion reference"));

		if (!shapesToConform.empty()) {
			// Outfit shapes were conformed/deformed — bake the morphed state into the NIF.
			wxLogMessage("[CBR] calling SetBaseShape (outfit shapes were deformed)");
			wxLog::FlushActive();
			outfitStudio->SetBaseShape();
			wxLogMessage("[CBR] SetBaseShape done");
			wxLog::FlushActive();
		}
		else {
			// No outfit shapes — nothing was deformed, nothing to bake.
			// Skip SetBaseShape entirely to avoid GL/heap corruption from
			// operating on a project with no GL meshes.
			CBR_LOG("no outfit shapes — skipping SetBaseShape (nothing was deformed)");
		}

		wxLogMessage("[CBR] calling DeleteShape(baseShape)");
		wxLog::FlushActive();
		CBR_HV("before DeleteShape(convRef baseShape)");
		project->DeleteShape(project->GetBaseShape());
		CBR_HV("after DeleteShape(convRef baseShape)");

		wxLogMessage("[CBR] DeleteShape done — calling DeleteSliders");
		wxLog::FlushActive();
		outfitStudio->DeleteSliders(mergeSliders, mergeZaps);
		CBR_HV("after DeleteSliders");

		wxLogMessage("[CBR] DeleteSliders done — calling WorkAnim->Clear");
		wxLog::FlushActive();
		project->GetWorkAnim()->Clear();
		CBR_HV("after WorkAnim->Clear");

		wxLogMessage("[CBR] WorkAnim->Clear done");
		wxLog::FlushActive();
	}
	else {
		CBR_LOG("conversionRefTemplate is 'None' — skipping conversion ref step");
		outfitStudio->UpdateProgress(5, _("Skipping conversion reference..."));
	}

	CBR_LOG("RefreshGUIFromProj before loading new reference");
	CBR_HV("before RefreshGUIFromProj (pre-newRef)");
	outfitStudio->RefreshGUIFromProj();
	CBR_HV("after RefreshGUIFromProj (pre-newRef)");

	outfitStudio->UpdateProgress(50, _("Loading new reference..."));
	outfitStudio->StartSubProgress(50, 55);
	CBR_LOG("loading new reference template: '%s'", newRefTemplate);
	if (AlertProgressError(LoadReferenceTemplate(newRefTemplate, mergeSliders, mergeZaps), _("Load Error"), "Failed to load new reference"))
		return;
	outfitStudio->EndProgress();

	CBR_LOG("CreateSetSliders + RefreshGUIFromProj after new ref load");
	outfitStudio->StartSubProgress(55, 65);
	outfitStudio->CreateSetSliders();
	outfitStudio->RefreshGUIFromProj();

	CBR_LOG("checking GetBaseShape() != nullptr");
	if (AlertProgressError(project->GetBaseShape() == nullptr, _("Missing Base Shape"), "The loaded reference does not contain a base shape"))
		return;

	// Refresh outfit shape list after new reference is loaded.
	auto outfitShapesForNewRef = getOutfitShapes();
	CBR_LOG("outfit shapes for new-reference operations: %zu", outfitShapesForNewRef.size());

	if (copyBoneWeights) {
		if (!outfitShapesForNewRef.empty()) {
			outfitStudio->UpdateProgress(65, _("Copying bones..."));
			outfitStudio->StartSubProgress(65, 85);
			CBR_LOG("CopyBoneWeightForShapes (%zu shapes, skipPopup=%d)", outfitShapesForNewRef.size(), (int)skipCopyBonesPopup);
			if (AlertProgressError(outfitStudio->CopyBoneWeightForShapes(outfitShapesForNewRef, skipCopyBonesPopup), _("Copy Bone Weights Error"), "Failed to copy bone weights"))
				return;
		}
		else {
			CBR_LOG("no outfit shapes for new reference — skipping CopyBoneWeightForShapes");
		}
	}

	if (conformSliders) {
		if (!outfitShapesForNewRef.empty()) {
			outfitStudio->UpdateProgress(85, _("Conforming outfit parts..."));
			outfitStudio->StartSubProgress(85, 100);
			CBR_LOG("ConformShapes for new reference (%zu shapes, skipPopup=%d)", outfitShapesForNewRef.size(), (int)skipConformPopup);
			if (AlertProgressError(outfitStudio->ConformShapes(outfitShapesForNewRef, skipConformPopup), _("Conform Error"), "Failed to conform shapes"))
				return;
		}
		else {
			CBR_LOG("no outfit shapes for new reference — skipping ConformShapes");
		}
	}

	if (!addBonesText.IsEmpty()) {
		outfitStudio->UpdateProgress(100, _("Adding Bones..."));
		wxStringTokenizer tkz(addBonesText, wxT(","));
		while (tkz.HasMoreTokens()) {
			wxString token = tkz.GetNextToken();
			CBR_LOG("AddBoneRef: '%s'", token);
			project->AddBoneRef(token.ToStdString());
		}
	}

	if (deleteReferenceOnCompleted) {
		// Delete any shape that is currently the base/reference, keeping outfit shapes.
		// Using getOutfitShapes() avoids comparing against stale pre-load pointers.
		//
		// GUARD: if there are no outfit shapes, deleting "non-outfit" shapes would delete
		// everything in the project — including the new reference body itself.  That makes
		// no sense for a pure body conversion, so skip the deletion entirely.
		auto freshOutfits = getOutfitShapes();
		if (freshOutfits.empty()) {
			CBR_LOG("deleteReferenceOnCompleted: skipping — no outfit shapes present; deleting would remove the new reference body");
		}
		else {
			CBR_LOG("deleteReferenceOnCompleted: removing reference shapes from NIF (%zu outfit shapes kept)", freshOutfits.size());
			for (auto* s : project->GetWorkNif()->GetShapes()) {
				if (std::find(freshOutfits.begin(), freshOutfits.end(), s) == freshOutfits.end()) {
					CBR_LOG("  deleting reference shape '%s'", wxString(s->name.get().c_str()));
					project->DeleteShape(s);
				}
			}
		}
	}

	CBR_LOG("DeleteUnreferencedNodes");
	int deletionCount = 0;
	auto workNif2 = project->GetWorkNif();
	if (workNif2)
		workNif2->DeleteUnreferencedNodes(&deletionCount);

	CBR_LOG("RecalcNormals for all shapes");
	auto allShapes = project->GetWorkNif()->GetShapes();
	for (auto& s : allShapes)
		outfitStudio->glView->RecalcNormals(s->name.get());

	CBR_LOG("final RefreshGUIFromProj + ApplySliders");
	outfitStudio->RefreshGUIFromProj();
	outfitStudio->ApplySliders();

	project->bSuppressLoadWarnings = false;
	CBR_LOG("Conversion finished successfully");
	wxLogMessage("Conversion finished.");
	wxLog::FlushActive();
	outfitStudio->EndProgress(_("Conversion finished."));

	} // end try
	catch (const std::bad_alloc& e) {
		project->bSuppressLoadWarnings = false;
		wxLogError("ConvertBodyReference: out of memory — %s", e.what());
		wxLog::FlushActive();
		wxMessageBox("Out of memory during conversion.\nThe project data may be too large or corrupt.", _("Conversion Error"), wxICON_ERROR);
		outfitStudio->EndProgress("", true);
		outfitStudio->RefreshGUIFromProj();
	}
	catch (const std::exception& e) {
		project->bSuppressLoadWarnings = false;
		wxLogError("ConvertBodyReference: exception — %s", e.what());
		wxLog::FlushActive();
		wxMessageBox(wxString::Format("Conversion failed:\n%s", e.what()), _("Conversion Error"), wxICON_ERROR);
		outfitStudio->EndProgress("", true);
		outfitStudio->RefreshGUIFromProj();
	}
	catch (...) {
		project->bSuppressLoadWarnings = false;
		wxLogError("ConvertBodyReference: unknown exception (possibly an access violation or noexcept violation)");
		wxLog::FlushActive();
		wxMessageBox("Conversion failed with an unknown error.\nCheck the log for details.", _("Conversion Error"), wxICON_ERROR);
		outfitStudio->EndProgress("", true);
		outfitStudio->RefreshGUIFromProj();
	}
}

int ConvertBodyReferenceDialog::LoadReferenceTemplate(const wxString& refTemplate, bool mergeSliders, bool mergeZaps) const {
	nifly::NiShape* baseShape = project->GetBaseShape();
	if (baseShape)
		outfitStudio->glView->DeleteMesh(baseShape->name.get());

	int error;
	wxLogMessage("Loading reference template '%s'...", refTemplate);
	std::string tmplName{refTemplate.ToUTF8()};
	auto tmpl = find_if(refTemplates.begin(), refTemplates.end(), [&tmplName](const RefTemplate& rt) { return rt.GetName() == tmplName; });
	if (tmpl != refTemplates.end()) {
		try {
			if (wxFileName(wxString::FromUTF8(tmpl->GetSource())).IsRelative())
				error = project->LoadReferenceTemplate(GetProjectPath() + PathSepStr + tmpl->GetSource(),
													   tmpl->GetSetName(),
													   tmpl->GetShape(),
													   tmpl->GetLoadAll(),
													   mergeSliders,
													   mergeZaps);
			else
				error = project->LoadReferenceTemplate(tmpl->GetSource(), tmpl->GetSetName(), tmpl->GetShape(), tmpl->GetLoadAll(), mergeSliders, mergeZaps);
		}
		catch (const std::bad_alloc&) {
			wxLogError("LoadReferenceTemplate: memory allocation failed while loading '%s'. The NIF or OSD data may be corrupt or too large.", refTemplate);
			error = -1;
		}
		catch (const std::exception& e) {
			wxLogError("LoadReferenceTemplate: exception loading '%s': %s", refTemplate, e.what());
			error = -1;
		}
	}
	else
		error = 1;

	if (!error) {
		// since some reference files have multiple shapes, just update all textures
		for (auto& s : project->GetWorkNif()->GetShapes())
			project->SetTextures(s);
	}

	return error;
}

bool ConvertBodyReferenceDialog::AlertProgressError(int error, const wxString& title, const wxString& message) const {
	if (error == 0)
		return false;

	// Always reset the suppress flag before showing any error UI — the wizard is
	// aborting, and subsequent project operations should use normal behaviour.
	project->bSuppressLoadWarnings = false;

	wxLogError(message);
	wxMessageBox(message, title, wxICON_ERROR);
	outfitStudio->EndProgress("", true);
	outfitStudio->RefreshGUIFromProj();
	return true;
}
