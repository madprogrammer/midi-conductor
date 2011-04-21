/* Copyright (c) Anoufrienko Sergey aka madpr
 */
#include "Midi.h"
#include <math.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>

static uint8_t DataLenMap[16] = 
	{0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 1, 1, 2, 0};

MidiStream::MidiStream(IOStream & stream, bool file) :
	m_stream(stream),
	m_file(file),
	lstatus(0)
{
}

MidiStream::~MidiStream()
{
}

midi_evt * MidiStream::ReadEvent()
{
	midi_evt * me = new midi_evt();
	/* get the status byte */
	me->status = m_stream.ReadByte();
	/* determine the length of data (for simple msgs only) */
	uint8_t dlen = DataLenMap[me->type];
	/* check for possibly running status */
	if(!(me->status & 1 << 7))
	{
		dlen = DataLenMap[HN(lstatus)];
		if (dlen == 1)
			me->data[0] = me->status;
		else
		{
			me->data[0] = me->status;
			me->data[1] = m_stream.ReadByte();
			me->status = lstatus;
		}
	}
	/* if successful, read the data byte(s) */
	else if(dlen)
	{
		std::vector<char> data(dlen);
		CHECK(dlen == m_stream.Read(data, data.size()), "Failed to read data");
		memcpy(&me->data, &data[0], data.size());
		/* set event length */
		me->len = dlen;
	}
	else if(me->type == SYSTEM)
	{
		/* system subtype */
		switch(me->chan)
		{
		case SYSEX:
		{
			if(!m_file)
			{
				/* initial buffer */
				char * buffer = (char*)malloc(SYSXBUF);
				/* get the vendor ID byte */
				m_stream.ReadByte();
				/* index into the buffer */
				uint16_t idx = 0;
				for (uint8_t c = 0; 
				     c != MB(EOX, SYSTEM); 
				     c = m_stream.ReadByte())
				{	
					if ((idx % SYSXBUF == 0) && idx) {
					   buffer = (char*)realloc(buffer, SYSXBUF*ceilf((float)(idx+1) / (float)SYSXBUF));
					   CHECK(buffer, "Failed to realloc");
					}
					buffer[idx] = c;
					idx++;
				}
				/* set sysex packet data */
				me->len = idx + 1;
				me->payload = buffer;
				break;
			}
			else
			{
                                /* read the variable length */
				me->len = ReadVar(m_stream);

				if(me->len)
				{
					char * buffer = (char*)malloc(me->len);
					/* read the data */
					std::vector<char> data;
					m_stream.Read(data, me->len);
					memcpy(buffer, &data[0], me->len);
					/* set buffer */
					me->payload = buffer;
				}
			}
			break;
		}
		case META:
		{
			if(m_file)
			{
				/* read the second status byte */
				me->data[0] = m_stream.ReadByte();
				/* read the variable length */
				me->len = ReadVar(m_stream);
				char * buffer = (char*)malloc(me->len);

				if(me->len)
				{
					/* read the data */
					std::vector<char> data;
					m_stream.Read(data, me->len);
					memcpy(buffer, &data[0], me->len);
					/* set buffer */
					me->payload = buffer;
				}
			}
			break;
		}
		default:
			/* process one byte events */
			me->len = 1;
		}
	}
	else
	{
		CHECK(false, "Invalid MIDI message received");
	}

	lstatus = me->status;
	return me;
}

void MidiStream::WriteEvent(const midi_evt * event)
{
	uint8_t dlen = DataLenMap[event->type];

	if(!dlen && event->type != SYSTEM)
		CHECK(false, "Invalid MIDI message sent");

	/* write the status byte */
	m_stream.WriteByte(event->status);
	
	if (event->type != SYSTEM)
	{
		/* write the data byte(s) */
		std::vector<char> data(dlen);
		memcpy(&data[0], &event->data, dlen);
		CHECK(dlen == m_stream.Write(data), "Failed to write data");
	}
	else
	{
		switch(event->chan)
		{
		case SYSEX:
		{
			std::vector<char> data(event->len - 1);
			/* write the vendor byte */
			m_stream.WriteByte(0x41);
			/* write sysex data */
			memcpy(&data[0], &event->payload, data.size());
			CHECK(data.size() == m_stream.Write(data),
				"Failed to send SysEx data");
			/* write end of sysex */
			m_stream.WriteByte(MB(EOX, SYSTEM));
			break;
		}
		case META:
			/* ignore meta events */
			break;
		default:
			/* process one byte events */
			m_stream.WriteByte(event->status);
		}
	}
}

IOStream & MidiStream::GetStream() const
{
	return m_stream;
}

/***************************************************************************/

void WriteVar(IOStream & stream, register uint32_t value)
{
	register uint32_t buffer;
	buffer = value & 0x7F;
	while ( (value >>= 7) )
	{
		buffer <<= 8;
		buffer |= ((value & 0x7F) | 0x80);
	}
	while (true)
	{
		stream.WriteByte(buffer);
		if (buffer & 0x80)
			buffer >>= 8;
		else
			break;
	}
}

uint32_t ReadVar(IOStream & stream)
{
	register uint32_t value;
	register uint8_t c;

	if ( (value = stream.ReadByte()) & 0x80 )
	{
		value &= 0x7F;
		do
		{
			value = (value << 7) + ((c = stream.ReadByte()) & 0x7F);
		} while (c & 0x80);
	}
	return(value);
}

/***************************************************************************/
/* MIDI File								   */
/***************************************************************************/

MidiFile::MidiFile(File & file) :
	m_File(file)
{
}

MidiFile::~MidiFile()
{
}

void * PlayThread(void * arg)
{
	Track * trk = (Track*)arg;
	MidiStream in = MidiStream(*trk->mem, true);

	while(true)
	{
		uint32_t delta = ReadVar(*trk->mem);
		uint64_t now = THIS.time;
		/* wait for the next event */
		while(THIS.time < now + delta)
			sched_yield();
		/* start timer */	
		trk->delta = 0;
		/* read next event from memory */
		midi_evt * e = in.ReadEvent();
		e->time  = THIS.time;

		/* handle some important meta events */
		if(e->type == SYSTEM && e->chan == META) {
			if(e->data[0] == 0x2F) // end of track
				break;
			else if(e->data[0] == 0x51) {
				/* locked tempo change */
				LOCKED(THIS.tempo = 60000000 / ( 
				(e->payload[0] << 16) + 
				(e->payload[1] << 8) + 
				 e->payload[2]));
			}
		}
		
		if(e->type == NOTEON && trk->human)
		{
			LOCKED(THIS.pause = true);
			while(true)
			{
				sched_yield();
				/* search for the wanted event */
				for(EventList::iterator it = THIS.events.begin();
				    it != THIS.events.end(); it++)
				    {
					/* check if the note matches */
				    	if((*it)->type == e->type 
					&& (*it)->data[0] == e->data[0])
					{
						/* update event time */
						(*it)->time += trk->delta;
						
						/* change the tempo to match human speed */
						double tempo = e->time * THIS.tempo / (*it)->time;
						double ratio = THIS.tempo / tempo;
						/* don't change the tempo */
						if(ratio < 0.95 || ratio > 1.05)
							printf("Ignoring drastic tempo change: %f\n", ratio);
						else
						{
							LOCKED(THIS.tempo = tempo);
						}

						delete *it;
						THIS.events.erase(it);

						LOCKED(THIS.pause = false);
						break;
					}
					/* don't wait too long for the player */
					if(trk->delta > 5000000 / THIS.unit)
					{
						printf("I've been waiting too long\n");
						/* give the pianist a hint */
						LOCKED(THIS.stream->WriteEvent(e));
						LOCKED(THIS.pause = false);
						break;
					}
				    }
				for(EventList::iterator it = THIS.events.begin();
				    it != THIS.events.end(); it++)
				    {				    	
					/* forget old notes */
					if(THIS.time - (*it)->time > 2500000 / THIS.unit)
					{
						printf("Forgotten orphaned note taken at %llu\n", (*it)->time);
						delete *it;
						THIS.events.erase(it);
						/* restore iterator */
						it = THIS.events.begin();
					}
				    }
				if(!THIS.pause) break;
			}
		}
		else
		{
			/* synchronized write operation */
			LOCKED(THIS.stream->WriteEvent(e));
		}
		/* free event */
		delete e;
	}

	trk->finished = true;
	return 0;
}

void * ReadThread(void * arg)
{
	MidiFile * file = (MidiFile*)arg;
	while(file->m_this.playing)
	{
		sched_yield();
		/* syncronized read */
		pthread_mutex_lock(&file->m_this.lock);
		try
		{
			midi_evt * e = file->m_this.stream->ReadEvent();
			e->time = file->m_this.time;

			if(e->type == NOTEON)
			{
				/* keep these events */
				file->m_this.events.push_back(e);
			}
			else if(e->type == NOTEOFF)
			{
				/* hack for broken files */
				e->type = NOTEON;
				e->data[1] = 0;
				/* keep these events */
				file->m_this.events.push_back(e);
			}
			else delete e;
		}
		catch(const std::logic_error & ex)
		{
			/* since I/O is nonblocking we may get here */
		}
		pthread_mutex_unlock(&file->m_this.lock);
	}
	return 0;
}

uint64_t get_clock()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint64_t)tv.tv_usec + 1000000 * tv.tv_sec;
}

void hisleep(uint64_t delay)
{
	uint64_t now;
	now = get_clock();
	while(get_clock() < now + delay)
		sched_yield();
}

void MidiFile::Play(MidiStream & out, uint16_t human)
{
	std::list<Track*> tracks;

	std::vector<char> buffer;
	m_File.Read(buffer, sizeof(MTHD));

	PMTHD MThd = (PMTHD)&buffer[0];
	uint16_t div = ntohs(MThd->div);
	uint16_t ntracks = ntohs(MThd->tracks);

	printf("Format: %d\n", ntohs(MThd->fmt));
	printf("Tracks: %d\n", ntracks);
	printf("Division: %04X\n", div);

	pthread_t tid;
	m_this.stream = &out;
	m_this.tempo = 120;
	m_this.time = 0;
	m_this.playing = false;
	m_this.pause = false;
	pthread_mutex_init(&m_this.lock, NULL);

	if(div & 1 << 15)
	{
		CHECK(false, "SMPTE is not implemented");
	}

	/* elevate process priority */
	int oldprio = getpriority(PRIO_PROCESS, 0);
	setpriority(PRIO_PROCESS, 0, -10);

	for(int i = 0; i < ntracks; i++)
	{
		m_File.Read(buffer, sizeof(MTRK));
		PMTRK MTrk = (PMTRK)&buffer[0];
		/* check track signature */
		if(strncmp((const char*)&MTrk->id, "MTrk", 4) != 0)
		{
			CHECK(false, "Invalid track signature");
		}
		/* load the track data into memory */
		Track * trk = new Track(this, new Memory(ntohl(MTrk->len)), i);
		m_File.Read(buffer, ntohl(MTrk->len));
		memcpy(trk->mem->Buffer(), &buffer[0], buffer.size());
		/* store it to free later */
		tracks.push_back(trk);
		/* mark human-assisted track */
		if(human == i) trk->human = true;
		/* create thread */
		pthread_create(&tid, NULL, PlayThread, trk);
	}

	m_this.playing = true;
	/* start the input thread as well */
	pthread_create(&tid, NULL, ReadThread, this);

	while(true)
	{
		m_this.unit = 60000000 / (div * m_this.tempo);
		hisleep(m_this.unit);
		if(!m_this.pause) m_this.time++;

		/* check for finished tracks */
		for(std::list<Track*>::iterator it = tracks.begin();
		    it != tracks.end(); it++)
		    {
		    	/* increase per-track delta time - the time since
			 * the last input event has been written */
		    	(*it)->delta++;

		    	if((*it)->finished)
			{
				delete static_cast<Track*>(*it)->mem;
				delete *it;

				tracks.erase(it);
				it = tracks.begin();
			}
		    }
		/* exit if all tracks finished */
		if(tracks.size() == 0) break;
	}
	/* let everyone finish work */
	m_this.playing = false;
	/* restore old process priority */
	setpriority(PRIO_PROCESS, 0, oldprio);
}

/*****************************************************************************/
/* Track								     */
/****************************************************************************/
Track::Track(MidiFile * p, Memory * m, uint16_t n)
{
	parent = p;
	mem = m;
	finished = false;
	human = false;
	idx = n;
	delta = 0;
}
