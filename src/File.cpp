/* Copyright (c) Anoufrienko Sergey aka madpr
 */

#include <iostream>
#include <stdio.h>
#include "File.h"

File::File(const std::string & name)
{
	m_File = fopen(name.c_str(), "r+b");
	CHECK(m_File != 0, "Failed to open the file");
	CHECK(stat(name.c_str(), &m_Stat) == 0, "Could not stat the file");
}

size_t File::Read (std::vector<char> & buffer, const size_t bytes)
{
	buffer.resize(bytes);
        size_t res = fread(&buffer[0], sizeof(char), bytes, m_File);
        CHECK(ferror(m_File) == 0, "Failed to read the file");
	CHECK(feof(m_File) == 0, "End of file reached");
        return res;
}

char File::ReadByte ()
{
	std::vector<char> buffer;
	CHECK(1 == Read(buffer, 1), "Failed to read byte");
	return buffer[0];
}

size_t File::Write (const std::vector<char> & buffer)
{
	size_t res = fwrite(&buffer[0], sizeof(char), buffer.size(), m_File);
	CHECK(ferror(m_File) == 0, "Failed to write the file");
	fflush(m_File);
	return res;
}

void File::WriteByte (const char byte)
{
	std::vector<char> buffer;
	buffer.push_back(byte);
	CHECK(1 == Write(buffer), "Failed to write byte");
}

void File::Seek(uint32_t offset, int whence)
{
	fseek(m_File, offset, whence);
}

File::~File()
{
	CHECK(m_File != 0, "File is not opened");
	fclose(m_File);
}

