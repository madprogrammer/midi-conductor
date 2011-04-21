/* Copyright (c) Anoufrienko Sergey aka madpr
 */
#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Common.h"
#include "Stream.h"

class File : public IOStream
{
public:
	File(const std::string & name);
	~File();

	size_t Read (std::vector<char> & buffer, const size_t bytes);
	char   ReadByte ();
	size_t Write (const std::vector<char> & buffer);
	void   WriteByte (const char byte);
	void   Seek(uint32_t offset, int whence);
private:
	FILE * m_File;
	std::string m_Name;
	struct stat m_Stat;
};

