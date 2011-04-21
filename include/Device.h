/* Copyright (c) Anoufrienko Sergey aka madpr
 */
#pragma once

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "Common.h"
#include "Stream.h"

class Device : public IOStream
{
public:
	Device(const std::string & name);
	~Device();

	size_t Read (std::vector<char> & buffer, const size_t bytes);
	char   ReadByte ();
	size_t Write (const std::vector<char> & buffer);
	void   WriteByte (const char byte);
private:
	int m_File;
	std::string m_Name;
};

