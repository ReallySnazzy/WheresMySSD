#ifndef FileRatioPanel_hpp
#define FileRatioPanel_hpp

#include <list>
#include <tuple>
#include <wx/wx.h>
#include "file.hpp"

class FileRatioPanel : public wxPanel {
private:
	/**
	 * Current file to display in the panel.
	 */
	File file = { "." };

	/**
	 * Forces a frame update.
	 */
	void PaintNow();

	std::list<std::tuple<std::string, int, wxColour>> file_sizes;

	long long int total_directory_size;

	void StoreFileSizes();

	/**
	 * min(width, height)*.8
	 */
	int CalculateArcRadius();

	/**
	 * Generates a dull color that looks good in the graph
	 */
	wxColour GenerateRandomColor();

	/**
	 * Sorts the currently stored files by size.
	 */
	void SortBySize();

public:
	void OnPaintEvent(wxPaintEvent &evt);

	/**
	 * Repaints on resize.
	 */
	void OnResize(wxSizeEvent &evt);
	
	/**
	 * Renders the current panel to the specified DC.
	 */
	void Render(wxDC &dc);

	/**
	 * Sets the internal directory to the specified directory, also repaints this frame.
	 */
	void UpdateDirectory(const File &file);

	FileRatioPanel(wxFrame *parent);

	DECLARE_EVENT_TABLE()
};

#endif