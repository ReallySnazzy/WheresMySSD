#ifndef WHERESMYSSD_HPP
#define WHERESMYSSD_HPP

#include <wx/wx.h>
#include <wx/sysopt.h>
#include <wx/sizer.h>
#include <wx/listctrl.h>
#include <vector>
#include "file.hpp"
#include "FileRatioPanel.hpp"

class WheresMySsd : public wxApp
{
public:
	virtual bool OnInit();
};
 
DECLARE_APP(WheresMySsd)

class SsdFinderWindow : public wxFrame {
private:
	/**
	 * Contains the ID for all of the controls
	 */
	enum ElementIds {
		BUTTON_OPEN_FOLDER_ID = wxID_HIGHEST + 1,
		BUTTON_SHOW_PARENT_ID,
		CANVAS_DISPLAY_GRAPH_ID,
		LIST_FILE_LISTINGS_ID,
		BUTTON_BROWSE_DIRECTORY_ID,
		FILERATIOPANEL_FILE_VIEWER_ID
	};

	/**
	 * Button that opens the folder selected in the list control
	 */
	wxButton *button_open_folder;

	/**
	 * Button that shows the parent of the current folder
	 */
	wxButton *button_show_parent;

	/**
	 * Button used to select a new directory.
	 */
	wxButton *button_browse_directory;

	/**
	 * List that shows file names and their sizes
	 */
	wxListCtrl *list_file_listings;

	/**
	 * File listing wrapper
	 */
	wxPanel *list_wrapper;

	/**
	 * File ratio graph control. Displays visual file size comparions.
	 */
	FileRatioPanel *fileratiopanel_file_viewer;

	/**
	 * Initializes components for the window
	 */
	void InitWindow();

	/**
	 * File representation of the current directory
	 */
	File current_directory = {std::string(".")};

	/**
	 * Clears old listings and refreshes the internal data of wxWidgets. Returns old item count.
	 */
	int ClearOldListings();
	
	/**
	 * Loads all children in the selected directory and refreshes wxWidgets internals. Returns new item count.
	 */
	int LoadListings();

	void SwitchDirectory(const File &new_directory);

public:
	SsdFinderWindow() : wxFrame((wxFrame*)NULL, -1, wxT("Where'd My SSD Go?"), wxDefaultPosition, wxSize(640, 480)) {
		InitWindow();
	}

	void OnOpenFolderClicked(wxCommandEvent &evt);
	void OnShowParentClicked(wxCommandEvent &evt);
	void RefreshListing();
	void OnBrowseFolderClicked(wxCommandEvent &evt);

	DECLARE_EVENT_TABLE()
};

#endif // WHERESMYSSD_HPP