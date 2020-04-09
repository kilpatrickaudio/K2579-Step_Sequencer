/*
 * K2579 Step Sequencer - Song File Manager
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
#include "song_file.h"
#include "song.h"
#include "eeprom.h"
#include "sysconfig.h"
#include "clock.h"
#include "sequencer.h"
#include "gui.h"

// file manager states
#define SONG_FILE_IDLE 0
#define SONG_FILE_LOADING 1
#define SONG_FILE_SAVING 2
#define SONG_FILE_ERASING 3

#define SONG_FILE_BUFFERS 64

unsigned char seq_buf[128];
// song file states
unsigned char song_file_state;
unsigned int song_file_buf_count;
unsigned char processing_song;
unsigned char skip_count;

// initialize the song file manager
void song_file_init(void) {
	song_file_state = SONG_FILE_IDLE;
	processing_song = 0;
	skip_count = 0;
	// load the last loaded song
	song_file_load(sysconfig_get_current_song());
}

// song file task
void song_file_task(void) {
	unsigned char seq;
	unsigned char buf_offset;

	// load song
	if(song_file_state == SONG_FILE_LOADING) {
		// force playback to stop
		clock_stop_command();

		// load the song data for song
		int addr = (processing_song << 11) | (song_file_buf_count << 5);

		// load a buffer from the EEPROM
		buf_offset = (addr & 0x60);
		eeprom_read_page(addr, seq_buf + buf_offset);

		// we have loaded a sequence of 4 buffers = 128 bytes
		if((song_file_buf_count & 0x03) == 0x03) {
			seq = (addr >> 7) & 0x0f;
			// check if this buffer is valid
			if(seq_buf[0x7f] != SONG_CONFIGURE_MARK) {
				song_clear_song();
				song_file_state = SONG_FILE_IDLE;
				return;
			}
			song_load_seq_buf(seq, seq_buf);
		}

		song_file_buf_count ++;
		// loading is complete
		if(song_file_buf_count == SONG_FILE_BUFFERS) {
			song_file_state = SONG_FILE_IDLE;
			sysconfig_set_current_song(processing_song);
			sequencer_new_song_loaded();
			gui_song_load_updated();
		}
	}
	// save song
	else if(song_file_state == SONG_FILE_SAVING) {
		// force playback to stop
		clock_stop_command();

		// save the song data for a song
		int addr = (processing_song << 11) | (song_file_buf_count << 5);

		// need to get 4 new buffers - 128 bytes
		if((song_file_buf_count & 0x03) == 0x00) {
			seq = (addr >> 7) & 0x0f;
			song_save_seq_buf(seq, seq_buf);
		}

		// save a buffer to the EEPROM
		buf_offset = (addr & 0x60);
		eeprom_write_page(addr, seq_buf + buf_offset);

		song_file_buf_count ++;
		// saving is complete
		if(song_file_buf_count == SONG_FILE_BUFFERS) {
			song_file_state = SONG_FILE_IDLE;
			sysconfig_set_current_song(processing_song);
			gui_song_save_updated();
		}
	}
}

// load song from flash
void song_file_load(unsigned char song) {
	if(song > 7) return;
	if(song_file_state != SONG_FILE_IDLE) return;
	song_file_state = SONG_FILE_LOADING;
	song_file_buf_count = 0;
	processing_song = song;
}

// save song to flash
void song_file_save(unsigned char song) {
	if(song > 7) return;
	if(song_file_state != SONG_FILE_IDLE) return;
	song_file_state = SONG_FILE_SAVING;
	song_file_buf_count = 0;
	processing_song = song;
}
