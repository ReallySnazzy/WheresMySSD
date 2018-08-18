#ifndef file_hpp
#define file_hpp

#include <string>
#include <vector>
#include <iostream>
#include <cstdint>

class File {
private:
	std::string path_string;
	void ConvertToAbsolutePath();
	bool is_directory;

public:
	File(std::string path) {
		if (path.length() == 2)
		{
			if (path[1] == ':')
			{
				path = path + "/";
			}
		}
		path_string = path;
		ConvertToAbsolutePath();
	}

	/**
	 * Returns true if a file is a directory
	 */
	bool IsDirectory() const;

	/**
	 * Gets all of the descendant files of this directory. Returns
	 * an empty vector in the case of being a file. Caches result.
	 */
	std::vector<File> AllDescendants() const;

	/**
	 * Uses multiple threads to find the size of this folder and all of its descendants.
	 * Returns the same thing as Size() in the case this is not a directory.
	 */
	uint64_t ConcurrentDescendantSize();

	/**
	 * Returns true if the file is not a directory
	 */
	bool IsFile() const;

	/**
	 * Returns the name of the file this object represents.
	 */
	std::string Name() const;

	/**
	 * Returns a string representation of the absolute path this object represents.
	 */
	std::string AbsolutePath() const;

	/**
	 * Gets the File-object representation of the folder containing the current file.
	 */
	File Parent() const;

	bool Exists() const;

	/**
	 * Gets a File-object representation of the named child.
	 */
	File Child(std::string file_name) const;

	/**
	 * Returns a list of all children in the current directory.
	 */
	std::vector<File> Children() const;

	/**
	 * Returns the size of the current file if this is not a directory
	 */
	uint64_t Size() const;

	/**
	 * Returns the sum of all sizes of all files in this directory.
	 */
	uint64_t ChildrenSize() const;
};

inline std::ostream& operator<<(std::ostream &out, const File &file) {
	out << file.AbsolutePath();
	return out;
}

#endif