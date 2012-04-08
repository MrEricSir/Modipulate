#include <stdio.h>
#include <modipulate.h>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void mySleep(unsigned ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

class Note {
public:
    int note;
    int instrument;
    int sample;
    int volume_command;
    int volume_value;
    int effect_command;
    int effect_value;
    
    Note() {
        note = 0;
        instrument = 0;
        sample = 0;
        volume_command = 0;
        volume_value = 0;
        effect_command = 0;
        effect_value = 0;
    }
    
    void print() {
        printf("%3d%3d%3d", note, instrument, sample);
    }
};


class Row {
public:
    int channels;
    int row;
    Note** notes;
    
    Row(int row, int channels) {
        this->row = row;
        this->channels = channels;
        notes = new Note*[channels];
        for (int i = 0; i < channels; i++)
            notes[i] = new Note();
    }
    
    ~Row() {
        for (int i = 0; i < channels; i++)
            delete notes[i];
        
        delete notes;
    }
    
    void print() {
        printf("%3d. ", row);
        for (int i = 0; i < channels; i++) {
            notes[i]->print();
            
            if (i != channels - 1)
                printf("|");
        }
        
        printf("\n");
    }
};


char* filename = "../demos/media/v-cf.it";
odipulateSong song;
odipulateErr err;
odipulateSongInfo* song_info;
int channels;
Row* my_row = NULL;


void on_pattern_change(ModipulateSong song, int pattern_number, void* user_data) {
    printf("\nPattern: %d ************************************\n", pattern_number);
}


void on_note(ModipulateSong song, unsigned channel, int note,
    int instrument, int sample, int volume_command, int volume_value,
    int effect_command, int effect_value, void* user_data) {
    
    my_row->notes[channel]->note = note;
    my_row->notes[channel]->instrument = instrument;
    my_row->notes[channel]->sample = sample;
    my_row->notes[channel]->volume_command = volume_command;
    my_row->notes[channel]->volume_value = volume_value;
    my_row->notes[channel]->effect_command =effect_command;
    my_row->notes[channel]->effect_value = effect_value;
}


void on_row_change(ModipulateSong song, int row, void* user_data) {
    if (my_row != NULL) {
        my_row->print();
        delete my_row;
    }
    
    my_row = new Row(row, channels);
}


int main(int argc, char **argv) {
    printf("Modipulate console demo\n");
    printf("Playing: %s\n", filename);
    printf("Use ctrl-c to quit.\n");
    printf("***************************\n\n");
    
    err = modipulate_global_init();
    if (!MODIPULATE_OK(err)) {
        printf("\nERROR: %s\n", modipulate_global_get_last_error_string());
        return err;
    }
    
    err = modipulate_song_load(filename, &song);
    if (!MODIPULATE_OK(err)) {
        printf("\nERROR: %s\n", modipulate_global_get_last_error_string());
        return err;
    }
    
    // Setup callbacks.
    modipulate_song_on_pattern_change(song, on_pattern_change, NULL);
    modipulate_song_on_row_change(song, on_row_change, NULL);
    modipulate_song_on_note(song, on_note, NULL);
    
    // Print song info.
    err = modipulate_song_get_info(song, &song_info);
    if (!MODIPULATE_OK(err)) {
        printf("\nERROR: %s\n", modipulate_global_get_last_error_string());
        return err;
    }
    
    printf("\n***************************\n");
    printf("Song info\n");
    printf("Title: %s\n", song_info->title);
    printf("Message: %s\n", song_info->message);
    printf("Number of channels: %d\n", song_info->num_channels);
    printf("Number of patterns: %d\n", song_info->num_patterns);
    printf("Number of samples: %d\n", song_info->num_samples);
    printf("Number of instruments: %d\n", song_info->num_instruments);
    printf("Default tempo: %d\n", song_info->default_tempo);
    printf("***************************\n\n");
    
    channels = song_info->num_channels;
    
    err = modipulate_song_play(song, 1);
    if (!MODIPULATE_OK(err)) {
        printf("\nERROR: %s\n", modipulate_global_get_last_error_string());
        return err;
    }
    
    // Mimic a game loop.
    while(true) {
        err = modipulate_global_update();
        if (!MODIPULATE_OK(err)) {
            printf("\nERROR: %s\n", modipulate_global_get_last_error_string());
            return err;
        }
        
        mySleep(50); // wait 50 ms (to reduce CPU usage)
    }
    
    // Realstically you'll never get here if you ctrl-C out of the
    // demo program. The following code is presented for illustrative
    // purposes.
    
    // Stop playback.
    err = modipulate_song_play(song, 0);
    if (!MODIPULATE_OK(err))
        return err;
    
    // Print final row and clean up.
    if (my_row != NULL) {
        my_row->print();
        delete my_row;
    }
    
    modipulate_song_unload(song);
    
    // Shut down Modipulate.
    err = modipulate_global_deinit();
    if (!MODIPULATE_OK(err))
        return err;
    
    return 0;
}
