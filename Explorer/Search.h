#pragma once

class Finder
{
	HANDLE m_Data;
	WIN32_FIND_DATA m_File;
public:
	Finder();

	bool find(const std::string &);
	bool go_next();
	bool is_file() const;
	bool not_hidden() const;

	WIN32_FIND_DATA get() const;
	~Finder();
};