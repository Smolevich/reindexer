#pragma once

#include <string>
#include <vector>

namespace reindexer {
namespace fs {
using std::string;
using std::vector;

struct DirEntry {
	string name;
	bool isDir;
};

enum FileStatus {
	StatError = -1,
	StatFile = 1,
	StatDir = 2,
};

int MkDirAll(const string &path);
int RmDirAll(const string &path);
int ReadFile(const string &path, string &content);
int ReadDir(const string &path, vector<DirEntry> &content);
FileStatus Stat(const string &path);
string GetCwd();
string GetTempDir();
inline static string JoinPath(string base, string name) { return base + ((!base.empty() && base.back() != '/') ? "/" : "") + name; }
}  // namespace fs
}  // namespace reindexer
