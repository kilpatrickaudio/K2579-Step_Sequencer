/*
 * K2579 Step Sequencer - Sequencer MIDI Handler
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
#include "seq_midi.h"
#include "midi_callbacks.h"
#include "midi.h"
#include "sequencer.h"
#include "song.h"
#include "clock.h"
#include "eeprom.h"
#include "sysconfig.h"
#include "song_file.h"
#include "screen_handler.h"
#include "TimeDelay.h"
#include "lcd.h"

// device restart
#define BOOTLOADER_ADDR 0x9FC00000

// debugging / programming via SYSEX
#define CMD_READ_EEPROM 0x70
#define CMD_WRITE_EEPROM 0x71
#define CMD_READBACK_EEPROM 0x72

// channels
unsigned char pt1_chan;
unsigned char pt2_chan;

// note state
char last_trigger_key;

// SYSEX receive handling
unsigned char sysex_rx_buf[256];
unsigned char sysex_rx_count;

// initialize the MIDI handler
void seq_midi_init(void) {
	pt1_chan = 0;  // channel 1
	pt2_chan = 1;  // channel 2
	sysex_rx_count = 0;
	last_trigger_key = 255;
}

// get the MIDI channel for a part
unsigned char seq_midi_get_channel(unsigned char part) {
	if(part > 1) return 0;
	if(part == 1) return pt2_chan;
	return pt1_chan;
}

// set the MIDI channel for a part
void seq_midi_set_channel(unsigned char part, unsigned char channel) {
	if(part > 1) return;
	if(part == 1) pt2_chan = channel;
	else pt1_chan = channel;
}

//
// SETUP MESSAGES
//
// learn the MIDI channel
void _midi_learn_channel(unsigned char channel) {
  // not used
}

//
// CHANNEL MESSAGES
//
// note off - note on with velocity = 0 calls this
void _midi_rx_note_off(unsigned char channel, 
		unsigned char note) {
	unsigned char map;
	unsigned char trigger;
	// our channels
	if(channel == pt1_chan || channel == pt2_chan) {
		map = sysconfig_get_key_map();
		trigger = sysconfig_get_key_trigger();

		// handle key map swapping
		if(sequencer_get_control_override(SYSCONFIG_MOD_KEY_MAP) == 1) {
			if(map == SYSCONFIG_KEY_MAP_B) map = SYSCONFIG_KEY_MAP_A;
			else map = SYSCONFIG_KEY_MAP_B;
		}

		// key map B
		if(map == SYSCONFIG_KEY_MAP_B) {
			// key trigger sequence
			if(note > 47 && note < 64) {
				if(trigger == SYSCONFIG_KEY_TRIGGER_MOM &&
						last_trigger_key == (note - 48)) {
					clock_stop_command();
					last_trigger_key = 255;
				}
			}
		}
		// key map A
		else {
			// key trigger sequence
			if(note > 23 && note < 40) {
				if(trigger == SYSCONFIG_KEY_TRIGGER_MOM &&
						last_trigger_key == (note - 24)) {
					clock_stop_command();
					last_trigger_key = 255;
				}
			}
		}
	}
}

// note on - note on with velocity > 0 calls this
void _midi_rx_note_on(unsigned char channel, 
		unsigned char note, 
		unsigned char velocity) {
	unsigned char map;

	// our channels
	if(channel == pt1_chan || channel == pt2_chan) {
		map = sysconfig_get_key_map();
		// handle key map swapping
		if(sequencer_get_control_override(SYSCONFIG_MOD_KEY_MAP) == 1) {
			if(map == SYSCONFIG_KEY_MAP_B) map = SYSCONFIG_KEY_MAP_A;
			else map = SYSCONFIG_KEY_MAP_B;
		}

		// key map B
		if(map == SYSCONFIG_KEY_MAP_B) {
			// key transpose
			if(note > 23 && note < 48) {
				sequencer_key_transpose(note - 36);
			}
			// key trigger sequence
			else if(note > 47 && note < 64) {
				last_trigger_key = note - 48;
				sequencer_key_trigger(last_trigger_key);
			}
			// key start
			else if(note == 65) {
				clock_run_command();
			}
			// key stop
			else if(note == 67) {
				clock_stop_command();
			}
			// key reset
			else if(note == 69) {
				clock_reset_input();
			}
			// key control restore
			else if(note == 71) {
				sequencer_control_restore();
			}
		}
		// key map A
		else {
			// key trigger sequence
			if(note > 23 && note < 40) {
				last_trigger_key = note - 24;
				sequencer_key_trigger(last_trigger_key);
			}
			// key start
			else if(note == 41) {
				clock_run_command();
			}
			// key stop
			else if(note == 43) {
				clock_stop_command();
			}
			// key reset
			else if(note == 45) {
				clock_reset_input();
			}
			// key control restore
			else if(note == 47) {
				sequencer_control_restore();
			}
			// key transpose
			else if(note > 47 && note < 73) {
				sequencer_key_transpose(note - 60);
			}
		}
	}
}

// key pressure
void _midi_rx_key_pressure(unsigned char channel, 
		unsigned char note,
		unsigned char pressure) {
	// echo our channels
	if(channel == pt1_chan || channel == pt2_chan) {
		_midi_tx_key_pressure(channel, note, pressure);
	}
}

// control change
void _midi_rx_control_change(unsigned char channel,
		unsigned char controller,
		unsigned char value) {
	// respond on both channels the same
	if(channel == pt1_chan || channel == pt2_chan) {
		// controller 1 = key map swap
		if(controller == 1) {
			sequencer_control_change(SYSCONFIG_MOD_KEY_MAP, value);
		}
		// controllers 20-25 = mod 1-6
		else if(controller > 19 && controller < 26) {
			sequencer_control_change(controller - 19, value);
		}
		// controller 31 = control restore
		else if(controller == 31 && value > 63) {
			sequencer_control_restore();
		}
		// controller 64 = mod 7
		else if(controller == 64) {
			sequencer_control_change(SYSCONFIG_MOD_SEQ_DIR, value);			
		}
		// echo others
		else if(controller < 120) {
			_midi_tx_control_change(channel, controller, value);
		}
	}
}

// channel mode - all sounds off
void _midi_rx_all_sounds_off(unsigned char channel) {
	// XXX reset the system
}

// channel mode - reset all controllers
void _midi_rx_reset_all_controllers(unsigned char channel) {
	// XXX reset the system
}

// channel mode - local control
void _midi_rx_local_control(unsigned char channel, unsigned char value) {
	// not supported
}

// channel mode - all notes off
void _midi_rx_all_notes_off(unsigned char channel) {
	// XXX reset the system
}

// channel mode - omni off
void _midi_rx_omni_off(unsigned char channel) {
	// not supported
}

// channel mode - omni on
void _midi_rx_omni_on(unsigned char channel) {
	// not supported
}

// channel mode - mono on
void _midi_rx_mono_on(unsigned char channel) {
	// not supported
}

// channel mode - poly on
void _midi_rx_poly_on(unsigned char channel) {
	// not supported
}

// program change
void _midi_rx_program_change(unsigned char channel,
		unsigned char program) {
	// echo our channels
	if(channel == pt1_chan || channel == pt2_chan) {
		_midi_tx_program_change(channel, program);
	}
}

// channel pressure
void _midi_rx_channel_pressure(unsigned char channel,
		unsigned char pressure) {
	// echo our channels
	if(channel == pt1_chan || channel == pt2_chan) {
		_midi_tx_channel_pressure(channel, pressure);
	}
}

// pitch bend
void _midi_rx_pitch_bend(unsigned char channel,
		unsigned int bend) {
	// echo our channels
	if(channel == pt1_chan || channel == pt2_chan) {
		_midi_tx_pitch_bend(channel, bend);
	}
}

//
// SYSTEM COMMON MESSAGES
//
// song position
void _midi_rx_song_position(unsigned int pos) {
	sequencer_midi_song_pos(pos);
}

// song select
void _midi_rx_song_select(unsigned char song) {
	if(song > 7) return;
	song_file_load(song);
}

//
// SYSEX MESSAGES
//
// sysex message start
void _midi_rx_sysex_start(void) {
	sysex_rx_count = 0;
}

// sysex message data byte
void _midi_rx_sysex_data(unsigned char data_byte) {
	sysex_rx_buf[sysex_rx_count] = data_byte;
	sysex_rx_count ++;
}

// sysex message received
void _midi_rx_sysex_msg(unsigned char data[], unsigned char len) {
	unsigned char cmd;
	unsigned char dev_type = midi_get_device_type();
	if(len < 4) return;
	// parse header
	if(data[0] != 0x00) return;  // look for
	if(data[1] != 0x01) return;  // Kilpatrick Audio
	if(data[2] != 0x72) return;  // SYSEX ID
	cmd = data[3];
	// K2579 commands 
	if(len >=5 && cmd == dev_type) {
		// read EEPROM data
		if((data[4] == CMD_READ_EEPROM) && (len == 13)) {
			int addr = ((data[5] & 0x0f) << 28) |
				((data[6] & 0x0f) << 24) |
				((data[7] & 0x0f) << 20) |
				((data[8] & 0x0f) << 16) |
				((data[9] & 0x0f) << 12) |
				((data[10] & 0x0f) << 8) |
				((data[11] & 0x0f) << 4) |
				(data[12] & 0x0f);
			unsigned char buf[32];
			int i;
			eeprom_read_page(addr, buf);
			// respond
			_midi_tx_sysex_start();
			_midi_tx_sysex_data(0x00);
			_midi_tx_sysex_data(0x01);
			_midi_tx_sysex_data(0x72);
			_midi_tx_sysex_data(dev_type);
			_midi_tx_sysex_data(CMD_READBACK_EEPROM);
			_midi_tx_sysex_data((addr >> 28) & 0x0f);
			_midi_tx_sysex_data((addr >> 24) & 0x0f);
			_midi_tx_sysex_data((addr >> 20) & 0x0f);
			_midi_tx_sysex_data((addr >> 16) & 0x0f);
			_midi_tx_sysex_data((addr >> 12) & 0x0f);
			_midi_tx_sysex_data((addr >> 8) & 0x0f);
			_midi_tx_sysex_data((addr >> 4) & 0x0f);
			_midi_tx_sysex_data(addr & 0x0f);
			for(i = 0; i < 32; i ++) {
				_midi_tx_sysex_data((buf[i] >> 4) & 0x0f);
				_midi_tx_sysex_data(buf[i] & 0x0f);
			}
			_midi_tx_sysex_end();
		}
		// write EEPROM data
		else if(data[4] == CMD_WRITE_EEPROM && len == (13 + 64)) {
			int addr = ((data[5] & 0x0f) << 28 |
				(data[6] & 0x0f) << 24 |
				(data[7] & 0x0f) << 20 |
				(data[8] & 0x0f) << 16 |
				(data[9] & 0x0f) << 12 |
				(data[10] & 0x0f) << 8 |
				(data[11] & 0x0f) << 4 |
				(data[12] & 0x0f));
			unsigned char buf[32];
			int inCount = 0;
			int i;
			for(i = 0; i < 32; i ++) {
				buf[i] = (data[13 + inCount] << 4) | (data[13 + inCount + 1] & 0x0f);
				inCount += 2;
			}
			eeprom_write_page(addr, buf);
		}
	}
}

//
// SYSTEM REALTIME MESSAGES
//
// timing tick
void _midi_rx_timing_tick(void) {
	clock_midi_tick();
}

// start song
void _midi_rx_start_song(void) {
	clock_midi_start();
}

// continue song
void _midi_rx_continue_song(void) {
	clock_midi_continue();
}

// stop song
void _midi_rx_stop_song(void) {
	clock_midi_stop();
}

// active sensing
void _midi_rx_active_sensing(void) {
	// not supported
}

// system reset
void _midi_rx_system_reset(void) {
	// XXX reset the system
	_midi_tx_system_reset();
}

//
// SPECIAL CALLBACKS
//
void _midi_restart_device(void) {
	int i;
	lcd_clear_screen();
	screen_write_popup(1000, " ", "SYSEX RESTART");
	screen_task();
	for(i = 0; i < 64; i ++) {
		lcd_task();
	}
	DelayMs(100);
	void (*fptr)(void);
	fptr = (void (*)(void))BOOTLOADER_ADDR;
	fptr();
}
