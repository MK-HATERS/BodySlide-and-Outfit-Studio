/*
BodySlide and Outfit Studio
See the included LICENSE file
*/
#pragma once

#include <wx/log.h>
#include <wx/panel.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

// ── BSLogPanel ────────────────────────────────────────────────────────────────
// Dockable bottom pane that captures wxLog output for BodySlide builds.
// Drop-in: after construction call wxLog::SetActiveTarget(panel->GetLogger()).
class BSLogPanel : public wxPanel {
public:
	explicit BSLogPanel(wxWindow* parent);

	// Append a line of text. Colour is applied per-character via attributes.
	void AppendMessage(const wxString& msg, const wxColour& colour);
	void AppendInfo   (const wxString& msg);
	void AppendWarning(const wxString& msg);
	void AppendError  (const wxString& msg);

	// Hook into wxLog routing
	class Logger : public wxLog {
	public:
		explicit Logger(BSLogPanel* panel) : m_panel(panel) {}
	protected:
		void DoLogRecord(wxLogLevel level, const wxString& msg, const wxLogRecordInfo&) override;
	private:
		BSLogPanel* m_panel;
	};
	Logger* GetLogger() { return m_logger; }

	void Clear();

private:
	wxTextCtrl* m_text  = nullptr;
	wxButton*   m_clear = nullptr;
	Logger*     m_logger= nullptr;
};
