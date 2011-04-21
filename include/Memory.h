/* Copyright (c) Anoufrienko Sergey aka madpr
 */
#pragma once

#include <sys/types.h>
#include <unistd.h>
#include "Common.h"
#include "Stream.h"

class Memory : public IOStream
{
public:
	Memory(uint32_t size);
	~Memory();

	size_t Read (std::vector<char> & buffer, const size_t bytes);
	char   ReadByte ();
	size_t Write (const std::vector<char> & buffer);
	void   WriteByte (const char byte);
	void   Seek(uint32_t offset, int whence);
	void * Buffer();
private:
	void * m_buffer;
	void * m_ptr;
};

