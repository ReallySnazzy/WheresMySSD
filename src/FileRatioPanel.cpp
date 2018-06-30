#include "FileRatioPanel.hpp"
#include <wx/dirdlg.h>
#include <wx/wx.h>
#include <stdexcept>
#include <cmath>
#include <cstdlib>

FileRatioPanel::FileRatioPanel(wxFrame *parent) : wxPanel(parent) {
	UpdateDirectory(std::string("."));
}

void FileRatioPanel::PaintNow() {
	wxClientDC dc(this);
	Render(dc);
}

void FileRatioPanel::OnPaintEvent(wxPaintEvent &evt) {
	wxPaintDC dc(this);
	Render(dc);
}

void FileRatioPanel::OnResize(wxSizeEvent &evt) {
	PaintNow();
}

void FileRatioPanel::StoreFileSizes() {
	total_directory_size = 0;
	file_sizes.clear();
	for (File &child : file.Children()) {
		int size = -1;
		try {
			if (child.IsDirectory()) {
				size = child.ChildrenSize();
			}
			else {
				size = child.Size();
			}
		}
		catch (std::exception &ex) {}
		if (size > 0) {
			total_directory_size += size;
		}
		file_sizes.push_back(std::make_tuple(child.Name(), size, GenerateRandomColor()));
	}
	SortBySize();
}

void FileRatioPanel::SortBySize() {
	file_sizes.sort([] (std::tuple<std::string, int, wxColour> left,
		std::tuple<std::string, int, wxColour> right) {
		return std::get<1>(left) > std::get<1>(right);
	});
}

void FileRatioPanel::UpdateDirectory(const File &file) {
	this->file = file;
	StoreFileSizes();
	PaintNow();
}

void FileRatioPanel::Render(wxDC &dc) {
	int width, height, radius;
	GetSize(&width, &height);
	radius = CalculateArcRadius();

	// Background
	dc.SetBrush(*wxBLACK);
	dc.DrawRectangle(0, 0, width, height);

	// Circle
	double angle = 0;
	for (std::tuple<std::string, int, wxColour> &file : file_sizes) {
		if (std::get<1>(file) > 0) {
			dc.SetBrush(std::get<2>(file));
			dc.SetTextForeground(std::get<2>(file));
			double this_angle = static_cast<double>(std::get<1>(file)) / static_cast<double>(total_directory_size) * 360;
			if (this_angle < 1) {
				continue;
			}
			dc.DrawEllipticArc(
				static_cast<wxCoord>(width / 2 - radius / 2), // x = middle of panel x
				static_cast<wxCoord>(height / 2 - radius / 2), // y = middle of panel y
				static_cast<wxCoord>(radius), // width = diameter = min(width,height)*.8
				static_cast<wxCoord>(radius),  // height = diameter = min(width,height)*.8
				angle,  // starting angle
				angle + this_angle); // ending angle
			angle += this_angle;
		}
	}

	// File legend
	wxCoord char_height = dc.GetCharHeight();
	wxCoord padding = 2;
	wxCoord y_pos = 3;
	wxCoord x_pos = 5;
	dc.SetTextForeground(*wxWHITE);
	dc.DrawText(wxString(file.AbsolutePath().c_str()), x_pos, y_pos + padding);
	y_pos += padding + char_height + padding;
	for (std::tuple<std::string, int, wxColour> &file : file_sizes) {
		wxColour &color = std::get<2>(file);
		std::string &name = std::get<0>(file);
		dc.SetBrush(color);
		dc.SetTextForeground(color);
		dc.DrawRectangle(x_pos, y_pos + padding, char_height, char_height);
		dc.DrawText(wxString(std::get<0>(file).c_str()), x_pos + char_height + padding, padding + y_pos);
		y_pos += padding + char_height + padding;
	}
}

wxColour FileRatioPanel::GenerateRandomColor() {
	unsigned char red, green, blue;
	red = static_cast<unsigned char>(rand() % 70 + 256 / 2 - 35);
	green = static_cast<unsigned char>(rand() % 70 + 256 / 2 - 35);
	blue = static_cast<unsigned char>(rand() % 70 + 256 / 2 - 35);
	return wxColour(red, green, blue, static_cast<unsigned char>(250));
}

int FileRatioPanel::CalculateArcRadius() {
	int width, height, min;
	GetSize(&width, &height);
	min = width < height ? width : height;
	return min * 8 / 10;
}

BEGIN_EVENT_TABLE(FileRatioPanel, wxPanel)
	EVT_PAINT(FileRatioPanel::OnPaintEvent)
	EVT_SIZE(FileRatioPanel::OnResize)
END_EVENT_TABLE()