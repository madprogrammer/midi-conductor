/* Copyright (c) Anoufrienko Sergey aka madpr
 */
#include <iostream>
#include <string.h>
#include "Memory.h"

Memory::Memory(uint32_t size)
{
	m_buffer = malloc(size);
	m_ptr = m_buffer;
}

size_t Memory::Read (std::vector<char> & buffer, const size_t bytes)
{
	buffer.resize(bytes);
        memcpy(&buffer[0], m_ptr, bytes);
	m_ptr = (void*)((char*)m_ptr + bytes);
	return bytes;
}

char Memory::ReadByte ()
{
	std::vector<char> buffer(1);
	CHECK(1 == Read(buffer, 1), "Failed to read byte");
	return buffer[0];
}

size_t Memory::Write (const std::vector<char> & buffer)
{
	uint32_t len = buffer.size();
	memcpy(m_ptr, &buffer[0], len);
	m_ptr = (void*)((char*)m_ptr + len);
	return len;
}

void Memory::WriteByte (const char byte)
{
	std::vector<char> buffer;
	buffer.push_back(byte);
	CHECK(1 == Write(buffer), "Failed to write byte");
}

void Memory::Seek(uint32_t offset, int whence)
{
	m_ptr = (void*)((char*)m_ptr + offset);
}

void * Memory::Buffer()
{
	return m_buffer;
}

Memory::~Memory()
{
	free(m_buffer);
}

