#include <windows.h>
#include <string>
#include <vector>
#include <Shlwapi.h>
#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <future>
#include <thread>
#include <queue>
#include "../file.hpp"

#define FULL_PATH_MAX_LENGTH 4096

static bool IsSpaceCharacter(char character) {
	return character == ' ' || character == '\n' || character == '\t' || character == '\r';
}

// Trims leading and trailing whitespace
static std::string TrimString(std::string input) {
	int string_length = input.length();
	int first = 0;
	while (first < string_length && IsSpaceCharacter(input[first])) {
		first++;
	}
	int last = string_length - 1;
	while (last >= first && IsSpaceCharacter(input[last])) {
		last--;
	}
	if (last < first) {
		return std::string();
	}
	return input.substr(first, last-first+1);
}

// expects the path to be a valid path, only possibly missing a trailing /
static std::string EnsureTrailingSlash(std::string path) {
	std::string trimmed_path = TrimString(path);
	char last_char = trimmed_path[trimmed_path.length()-1];
	if (last_char == '\\' || last_char == '/') {
		return trimmed_path;
	} else {
		return trimmed_path + "\\";
	}
}

File File::Child(std::string child_name) const {
	File result = AbsolutePath() + child_name;
	return result;
}

bool File::Exists() const {
	return PathFileExistsA(AbsolutePath().c_str());
}

void File::ConvertToAbsolutePath() {
	char path_buffer[FULL_PATH_MAX_LENGTH];
	GetFullPathName(path_string.c_str(), FULL_PATH_MAX_LENGTH, path_buffer, nullptr);
	path_string = TrimString(std::string(path_buffer));
}

std::string File::AbsolutePath() const {
	if (IsDirectory()) {
		return EnsureTrailingSlash(path_string);
	}
	else {
		return path_string;
	}
}

std::string File::Name() const {
	const char *file_name_chars = PathFindFileName(path_string.c_str());
	return std::string(file_name_chars);
}

File File::Parent() const {
	using namespace std::string_literals;
	size_t path_length = path_string.length();
	int new_path_ending = -1;
	for (int i = path_length-1; i >= 0; i--) {
		if (path_string[i] == '\\' || path_string[i] == '/') {
			new_path_ending = i;
			break;
		}
	}
	if (new_path_ending > 0) {
		return File(path_string.substr(0, new_path_ending));
	} else {
		return File(path_string);
	}
}

uint64_t File::ChildrenSize() const {
	if (!IsDirectory()) {
		throw std::logic_error("Cannot get the size of a file's children. Files have no children.");
	}
	try {
		size_t file_size = 0;
		for (File &file : Children()) {
			if (file.IsDirectory()) {
				file_size += file.ChildrenSize();
			}
			else {
				file_size += file.Size();
			}
		}
		return file_size;
	}
	catch (std::exception &ex) {
		return 0;
	}
}

uint64_t File::Size() const {
	if (IsDirectory()) {
		throw std::logic_error("Cannot get the file size of a directory. Directories contain multiple files.");
	}
	uint64_t size = 0;
    HANDLE file_handle = CreateFile(path_string.c_str(), GENERIC_READ, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 
			FILE_ATTRIBUTE_NORMAL, NULL);
	if (file_handle == INVALID_HANDLE_VALUE) {
		CloseHandle(file_handle);
		throw std::runtime_error("Unable to get file size");
	}
	size = GetFileSize(file_handle, nullptr);
	CloseHandle(file_handle);
	if (size == INVALID_FILE_SIZE) {
		throw std::runtime_error("Unable to get file size");
	}
	return size;
}

std::vector<File> File::AllDescendants() const {
	if (IsFile()) {
		return std::vector<File>();
	}
	std::queue<File> search_queue;
	for (File &file : Children()) {
		search_queue.push(file);
	}
	std::vector<File> all_files;
	while (!search_queue.empty()) {
		File front = search_queue.front();
		if (front.IsDirectory()) {
			for (File &child : front.Children()) {
				search_queue.push(child);
			}
		}
		all_files.push_back(front);
		search_queue.pop();
	}
	return all_files;
}

std::vector<File> File::Children() const {
	if (IsFile()) {
		return std::vector<File>();
	} 
	else {
		WIN32_FIND_DATA file_data;
		std::vector<File> result;
		std::string new_path = EnsureTrailingSlash(path_string);
		std::string path_search = new_path + "*";
		HANDLE file_finder_handle = FindFirstFile(path_search.c_str(), &file_data);
		if (file_finder_handle != INVALID_HANDLE_VALUE) {
			do {
				std::string next_file_name = file_data.cFileName;
				if (next_file_name != "." && next_file_name != "..") {
					result.push_back(File(new_path + next_file_name));
				}
			} while (FindNextFile(file_finder_handle, &file_data));
			FindClose(file_finder_handle);
		}
		return result;
	}
}

bool File::IsDirectory() const {
	return PathIsDirectory(path_string.c_str());
}

bool File::IsFile() const {
	return !IsDirectory();
}

// CONCURRENT FILE SIZE 

typedef std::shared_ptr<std::future<uint64_t>> FuturePtr;

static uint64_t FastFileSizeProcedure(std::vector<File> files, int start, int finish) {
	if (finish > files.size()) {
		throw std::invalid_argument("Finish is out of range");
	}
	if (start < 0) {
		throw std::invalid_argument("Start is out of range");
	}
	uint64_t size = 0;
	for (int i = start; i < finish; i++) {
		if (files[i].IsFile()) {
			size += files[i].Size();
		}
	}
	return size;
}

static FuturePtr FastFileSizeThread(std::vector<File> files, int start, int finish) {
	FuturePtr ptr = std::make_shared<std::future<uint64_t>>(std::async(std::launch::async, [=] {
		return FastFileSizeProcedure(files, start, finish);
	}));
	return ptr;
}

uint64_t File::ConcurrentDescendantSize() {
	if (IsFile()) {
		return Size();
	}
	std::vector<FuturePtr> futures;
	std::vector<File> descendants = AllDescendants();
	unsigned int thread_count = std::thread::hardware_concurrency();
	unsigned int work_per_thread = descendants.size() / thread_count;
	if (descendants.size() <= thread_count) {
		return ChildrenSize();
	}
	uint64_t total_size = 0;
	for (int i = 0; i < thread_count; i++) {
		if (i == thread_count - 1) {
			total_size += FastFileSizeProcedure(descendants, i*work_per_thread, descendants.size());
		}
		else {
			futures.push_back(FastFileSizeThread(descendants, i*work_per_thread, i*work_per_thread + work_per_thread));
		}
	}
	for (FuturePtr future : futures) {
		total_size += future->get();
	}
	return total_size;
}