/* Copyright (c) Anoufrienko Sergey aka madpr
 */
#pragma once

struct IOStream
{
	/// Read bytes from the stream.
	/// Buffer size will be adjusted.
	/// @param buffer Destination buffer.
	/// @param bytes How many bytes to read.
	/// @exception std::logic_error in case of an error.
	/// @return Number of bytes actually read.
	virtual size_t Read(std::vector<char> & buffer, const size_t bytes) = 0;
	virtual char ReadByte () = 0;

	/// Write bytes to the stream.
	/// @param buffer Source buffer.
	/// @exception std::logic_error in case of an error.
	/// @return Number of bytes actually written.
	virtual size_t Write (const std::vector<char> & buffer) = 0;
	virtual void WriteByte (const char byte) = 0;

	virtual ~IOStream() { }
};

#include "File.h"
#include "Memory.h"
