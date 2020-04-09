/*
 * K2579 Step Sequencer - Song File Manager
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
// initialize the song file manager
void song_file_init(void);

// song file task - run on the main thread
void song_file_task(void);

// load song from flash
void song_file_load(unsigned char song);

// save song to flash
void song_file_save(unsigned char song);

