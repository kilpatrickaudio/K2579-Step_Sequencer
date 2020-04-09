/*
 * K2579 Step Sequencer - User Interface
 *
 * Copyright 2011: Kilpatrick Audio
 * Written by: Andrew Kilpatrick
 *
 */
// initialize the GUI
void gui_init(void);

// run this every 16ms
void gui_task(void);

// notify the GUI that a playback value has changed
void gui_playback_updated(void);

// notify the GUI that a control override has changed
void gui_control_override_updated(void);

// notify the GUI that a song has just been loaded
void gui_song_load_updated(void);

// notify the GUI that a song has just been saved
void gui_song_save_updated(void);
