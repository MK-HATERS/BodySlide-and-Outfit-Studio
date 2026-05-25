/*
BodySlide and Outfit Studio
See the included LICENSE file
*/
#include "BSLogPanel.h"
#include "BSTheme.h"
#include "BSArtProvider.h"

#include <wx/artprov.h>
#include <wx/datetime.h>

BSLogPanel::BSLogPanel(wxWindow* parent)
	: wxPanel(parent, wxID_ANY) {

	SetBackgroundColour(gTheme.bgMain);
	SetForegroundColour(gTheme.fgText);
	SetFont(gTheme.fontMono);

	// ── Header row ────────────────────────────────────────────────────────────
	auto* header = new wxPanel(this, wxID_ANY);
	header->SetBackgroundColour(gTheme.bgPanel);

	auto* lblLog = new wxStaticText(header, wxID_ANY, "Build Log");
	lblLog->SetFont(gTheme.fontBold);
	lblLog->SetForegroundColour(gTheme.fgText);

	m_clear = new wxButton(header, wxID_ANY, wxEmptyString,
	                       wxDefaultPosition, wxSize(22, 22), wxBORDER_NONE);
	m_clear->SetBitmap(wxArtProvider::GetBitmap(BS_ART_LOG_CLEAR,
	                                             wxART_BUTTON, wxSize(14, 14)));
	m_clear->SetBackgroundColour(gTheme.bgPanel);
	m_clear->SetToolTip("Clear log");
	m_clear->Bind(wxEVT_BUTTON, [this](wxCommandEvent&) { Clear(); });

	auto* hdr = new wxBoxSizer(wxHORIZONTAL);
	hdr->Add(lblLog,  0, wxALIGN_CENTER_VERTICAL | wxLEFT, 6);
	hdr->AddStretchSpacer();
	hdr->Add(m_clear, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 4);
	header->SetSizer(hdr);

	// ── Text area ─────────────────────────────────────────────────────────────
	m_text = new wxTextCtrl(this, wxID_ANY, wxEmptyString,
	                        wxDefaultPosition, wxDefaultSize,
	                        wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2 |
	                        wxNO_BORDER | wxHSCROLL);
	m_text->SetBackgroundColour(gTheme.bgMain);
	m_text->SetForegroundColour(gTheme.fgText);
	m_text->SetFont(gTheme.fontMono);

	// ── Layout ───────────────────────────────────────────────────────────────
	auto* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(header, 0, wxEXPAND);
	sizer->Add(m_text, 1, wxEXPAND);
	SetSizer(sizer);

	// ── Logger ───────────────────────────────────────────────────────────────
	m_logger = new Logger(this);
}

void BSLogPanel::AppendMessage(const wxString& msg, const wxColour& colour) {
	wxTextAttr attr;
	attr.SetTextColour(colour);
	attr.SetFont(gTheme.fontMono);

	// Timestamp prefix in muted colour
	wxTextAttr tsAttr;
	tsAttr.SetTextColour(gTheme.fgMuted);
	tsAttr.SetFont(gTheme.fontMono);

	wxString ts = wxDateTime::Now().Format("[%H:%M:%S] ");
	m_text->SetDefaultStyle(tsAttr);
	m_text->AppendText(ts);

	m_text->SetDefaultStyle(attr);
	m_text->AppendText(msg + "\n");

	// Auto-scroll to bottom
	m_text->ShowPosition(m_text->GetLastPosition());
}

void BSLogPanel::AppendInfo   (const wxString& m) { AppendMessage(m, gTheme.fgText);       }
void BSLogPanel::AppendWarning(const wxString& m) { AppendMessage(m, gTheme.colorWarning);  }
void BSLogPanel::AppendError  (const wxString& m) { AppendMessage(m, gTheme.colorError);    }

void BSLogPanel::Clear() {
	m_text->Clear();
}

// ── Logger implementation ─────────────────────────────────────────────────────
void BSLogPanel::Logger::DoLogRecord(wxLogLevel level,
                                     const wxString& msg,
                                     const wxLogRecordInfo&) {
	if (!m_panel) return;
	switch (level) {
		case wxLOG_Error:
		case wxLOG_FatalError:
			m_panel->AppendError(msg);
			break;
		case wxLOG_Warning:
			m_panel->AppendWarning(msg);
			break;
		default:
			m_panel->AppendInfo(msg);
			break;
	}
}
