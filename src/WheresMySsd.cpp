#include "WheresMySsd.hpp"
#include <cstdlib>
#include <cmath>
#include <cstdint>
#include <thread>
#include <future>
#include <wx/msgdlg.h>

IMPLEMENT_APP(WheresMySsd)

void SsdFinderWindow::InitWindow() {
	// create elements
	list_wrapper = new wxPanel(this);
	list_file_listings = new wxListCtrl(list_wrapper, LIST_FILE_LISTINGS_ID, wxDefaultPosition, wxDefaultSize, wxLC_REPORT);
	list_file_listings->InsertColumn(0, wxT("File name"), wxLIST_FORMAT_LEFT, 50);
	list_file_listings->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
	list_file_listings->InsertColumn(1, wxT("File size"), wxLIST_FORMAT_LEFT, 50);
	list_file_listings->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);
	list_file_listings->SetForegroundColour(*wxBLACK);
	list_file_listings->SetBackgroundColour(*wxWHITE);
	list_file_listings->SetAutoLayout(true);
	button_browse_directory = new wxButton(this, BUTTON_BROWSE_DIRECTORY_ID, wxT("Browse directory"), wxDefaultPosition, wxDefaultSize, 0);
	button_open_folder = new wxButton(this, BUTTON_OPEN_FOLDER_ID, wxT("Open folder"), wxDefaultPosition, wxDefaultSize, 0);
	button_show_parent = new wxButton(this, BUTTON_SHOW_PARENT_ID, wxT("Show parent"), wxDefaultPosition, wxDefaultSize, 0);
	fileratiopanel_file_viewer = new FileRatioPanel(this);

	// make list stretch
	wxBoxSizer *list_sizer = new wxBoxSizer(wxVERTICAL);
	list_sizer->Add(list_file_listings, 1, wxEXPAND | wxALL, 5);
	list_wrapper->SetSizer(list_sizer);

	// create main divisions
	wxBoxSizer *panel_canvas_separator = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *list_buttons_separator = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *open_parent_buttons_separator = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *interface_browser_separator = new wxBoxSizer(wxVERTICAL);
	open_parent_buttons_separator->Add(button_open_folder, 1, wxEXPAND | wxALL, 2);
	open_parent_buttons_separator->Add(button_show_parent, 1, wxEXPAND | wxALL, 2);
	list_buttons_separator->Add(list_wrapper, 1, wxEXPAND | wxALL, 2);
	list_buttons_separator->Add(open_parent_buttons_separator, 0, wxEXPAND | wxALL, 2);
	panel_canvas_separator->Add(list_buttons_separator, 0, wxEXPAND | wxALL, 2);
	panel_canvas_separator->Add(fileratiopanel_file_viewer, 1, wxEXPAND | wxALL, 2);
	interface_browser_separator->Add(panel_canvas_separator, 1, wxEXPAND | wxALL, 0);
	interface_browser_separator->Add(button_browse_directory, 0, wxEXPAND | wxALL, 2);

	// finish window
	this->SetSizer(interface_browser_separator);
}

void SsdFinderWindow::OnOpenFolderClicked(wxCommandEvent &evt) {
	long index = list_file_listings->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	wxString file_name_wxstring = list_file_listings->GetItemText(index);
	std::string file_name = file_name_wxstring.ToStdString();
	File file = current_directory.Child(file_name);
	if (file.IsDirectory()) {
		SwitchDirectory(file);
	}
	else {
		wxMessageBox("Selected item not a directory", "Error", wxOK | wxCENTRE, this);
	}
}

void SsdFinderWindow::OnShowParentClicked(wxCommandEvent &evt) {
	SwitchDirectory(current_directory.Parent());
}

void SsdFinderWindow::SwitchDirectory(const File &new_directory) {
	current_directory = new_directory;
	RefreshListing();
	fileratiopanel_file_viewer->UpdateDirectory(current_directory);
}

int SsdFinderWindow::ClearOldListings() {
	int old_count = list_file_listings->GetTopItem();
	list_file_listings->DeleteAllItems();
	list_file_listings->RefreshItems(0, old_count);
	return old_count;
}

int SsdFinderWindow::LoadListings() {
	int index = 0;
	for (File &child : current_directory.Children()) {
		std::string size_string;
		try {
			size_string = std::to_string(child.ConcurrentDescendantSize());
		}
		catch (std::exception &ex) {
			size_string = std::string("0");
		}
		std::string name = child.Name();
		index = list_file_listings->InsertItem(index, wxString(name.c_str()), 0);
		list_file_listings->SetItem(index, 1, wxString(size_string.c_str()), 0);
		list_file_listings->RefreshItem(index);
		index++;
	}
	list_file_listings->RefreshItems(0, index);
	return index;
}

void SsdFinderWindow::RefreshListing() {
	ClearOldListings();
	LoadListings();
	list_file_listings->FitInside();
	list_wrapper->Refresh();
	Refresh();
	Update();
}

void SsdFinderWindow::OnBrowseFolderClicked(wxCommandEvent &evt) {
	wxString path = wxDirSelector("Choose input directory", "",
		wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST | wxDD_CHANGE_DIR, wxDefaultPosition, this);
	if (!path.empty()) {
		File new_directory = File(path.ToStdString());
		if (new_directory.Exists()) {
			SwitchDirectory(new_directory);
		}
	}
}

bool WheresMySsd::OnInit() {
	try {
		SsdFinderWindow *window = new SsdFinderWindow;
		window->Show();
		window->RefreshListing();
	}
	catch (std::exception &ex) {
		std::string narrowEx = ex.what();
		std::wstring wideEx(narrowEx.begin(), narrowEx.end());
		MessageBox(nullptr, wideEx.c_str(), L"exception", MB_OK);
	}
	return true;
}

BEGIN_EVENT_TABLE(SsdFinderWindow, wxFrame)
	EVT_BUTTON(BUTTON_OPEN_FOLDER_ID, SsdFinderWindow::OnOpenFolderClicked)
	EVT_BUTTON(BUTTON_SHOW_PARENT_ID, SsdFinderWindow::OnShowParentClicked)
	EVT_BUTTON(BUTTON_BROWSE_DIRECTORY_ID, SsdFinderWindow::OnBrowseFolderClicked)
END_EVENT_TABLE()