/* Copyright (c) Anoufrienko Sergey aka madpr
 */
#include <iostream>
#include "Device.h"

Device::Device(const std::string & name)
{
	m_File = open(name.c_str(), O_RDWR | O_NONBLOCK);
	CHECK(m_File != 0, "Failed to open the file");
}

size_t Device::Read (std::vector<char> & buffer, const size_t bytes)
{
	buffer.resize(bytes);
        size_t res = read(m_File, &buffer[0], bytes);
        CHECK(res != 0, "Failed to read from device");
        return res;
}

char Device::ReadByte ()
{
	std::vector<char> buffer;
	CHECK(1 == Read(buffer, 1), "Failed to read byte");
	return buffer[0];
}

size_t Device::Write (const std::vector<char> & buffer)
{
	size_t res = write(m_File, &buffer[0], buffer.size());
	CHECK(res != 0, "Failed to write to device");
	return res;
}

void Device::WriteByte (const char byte)
{
	std::vector<char> buffer;
	buffer.push_back(byte);
	CHECK(1 == Write(buffer), "Failed to write byte");
}

Device::~Device()
{
	CHECK(m_File != 0, "File is not opened");
	close(m_File);
}

