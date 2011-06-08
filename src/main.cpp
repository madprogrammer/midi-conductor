/* Copyright (c) Anoufrienko Sergey aka madpr
 */
#include <stdlib.h>
#include <stdio.h>
#include "File.h"
#include "Device.h"
#include "Midi.h"

int main(int argc, char * argv[])
{
	if (argc == 1) {
		fprintf(stderr, "usage: %s <device> <midi-file> <human-track-number>\n", argv[0]);
		return EXIT_FAILURE;
	}

	Device midi = Device(argv[1]);
	MidiStream ms = MidiStream(midi, false);

	File f = File(argv[2]);
	MidiFile m = MidiFile(f);

	m.Play(ms, atoi(argv[3]));

	return EXIT_SUCCESS;
}

