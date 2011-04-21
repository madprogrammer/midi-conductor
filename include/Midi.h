/* Copyright (c) Anoufrienko Sergey aka madpr
 */
#pragma once

#include "Common.h"

/* MIDI message types */
#define NOTEOFF	0x08	// note, velocity
#define NOTEON	0x09	// note, velocity
#define KEYAFT	0x0A	// note, amount
#define CONTROL	0x0B	// controller, value
#define PROGRAM	0x0C	// program
#define CHNAFT	0x0D	// amount
#define WHEEL	0x0E	// pitch-value
#define SYSTEM	0x0F

/* system message types */
#define SYSEX	0x00	// byte vendor ID + data until EOX
#define MTIME	0x01	// ???
#define SNGPOS	0x02	// LSB, MSB
#define SNGSEL	0x03	// number
#define TUNREQ	0x06
#define EOX	0x07
#define CLOCK	0x08
#define START	0x0A
#define CONT	0x0B
#define STOP	0x0C
#define ACTSNS	0x0E
#define META	0x0F

/* SysEx buffer size */
#define SYSXBUF	1024

/* macro for accessing low/high nibbles */
#define MB(l,h) (((uint8_t)l) | (((uint8_t)h) << 4))
#define LN(s) 	((s) & 15)
#define HN(s) 	((s) >> 4)
#define HIBYTE(w)((uint8_t)(((uint8_t)(w) >> 8) & 0xFF))
#define LOBYTE(w)((uint8_t)(w))

typedef struct _midi_evt
{
	union {
		uint8_t  status;
		struct {
			uint8_t	 chan:4;
			uint8_t  type:4;
		};
	};
	uint8_t	 data[2];
	uint16_t len;
	uint64_t time;
	char*	 payload;
} midi_evt, *pmidi_evt;

#include "Stream.h"

class MidiStream
{
public:
	MidiStream(IOStream & stream, bool file);
	virtual ~MidiStream();

	midi_evt * ReadEvent();
	void WriteEvent(const midi_evt * event);	

	IOStream & GetStream() const;
private:
	IOStream & m_stream;
	bool m_file;
	uint8_t lstatus;
};

#pragma pack(1)
typedef struct _MTHD
{
	char id[4];
	uint32_t len;
	uint16_t fmt;
	uint16_t tracks;
	uint16_t div;
} MTHD, *PMTHD;

typedef struct _MTRK
{
	char id[4];
	uint32_t len;
} MTRK, *PMTRK;

#include <arpa/inet.h>
#include <pthread.h>

void WriteVar(IOStream & stream, register uint32_t value);
uint32_t ReadVar(IOStream & stream);

#define THIS trk->parent->m_this
#define LOCK pthread_mutex_lock(&THIS.lock)
#define UNLOCK pthread_mutex_unlock(&THIS.lock)
#define LOCKED(p) \
	LOCK;	  \
	p; 	  \
	UNLOCK;

typedef std::list<midi_evt*> EventList;

class MidiFile
{
public:
	MidiFile(File & file);
	~MidiFile();

	/* track playing thread uses this */
	struct Private
	{
		MidiStream * stream;
		pthread_mutex_t lock;
		uint64_t time;
		uint16_t tempo;
		uint16_t unit;
		bool	 pause;
		bool	 playing;
		EventList events;
	};

	void Play(MidiStream & out, uint16_t human);	
	Private m_this;
private:
	File & m_File;
};

/* passed to track playing thread */
struct Track
{
	Track(MidiFile *, Memory *, uint16_t);
	MidiFile * parent;
	Memory *   mem;
	bool 	   finished;
	bool	   human;
	uint16_t   idx;
	uint64_t   delta;
};

