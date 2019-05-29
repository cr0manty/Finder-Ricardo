#include "System.h"

Finder::Finder() :
	m_Data(NULL)
{
}

bool Finder::find(const std::string &_name)
{
	m_Data = FindFirstFile(_name.c_str(), &m_File);
	return m_Data != INVALID_HANDLE_VALUE;
}

bool Finder::go_next()
{
	return FindNextFile(m_Data, &m_File);
}

bool Finder::is_file() const
{
	return m_File.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE;
}

bool Finder::not_hidden() const
{
	return !(m_File.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) &&
		!(m_File.dwFileAttributes &  FILE_ATTRIBUTE_REPARSE_POINT) &&
		lstrcmp(m_File.cFileName, "..") && lstrcmp(m_File.cFileName, ".");
}

WIN32_FIND_DATA Finder::get() const
{
	return m_File;
}

Finder::~Finder()
{
	if (m_Data)
		FindClose(m_Data);
}
