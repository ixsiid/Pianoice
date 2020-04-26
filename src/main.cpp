#include <iostream>
#include <signal.h>
#include <unistd.h>

#include <cstring>

#include "midi.hpp"

#define DEBUG

#ifdef DEBUG
#define _v(x) std::cout << x << std::endl
#else
#define _v(x)
#endif

static void usage() {
    std::cout << "[-c CREATE_DEVICE_NAME] "
              << "[-s SOUND_DEVICE]"
              << "-d CONNECT_DEVICE_NAME "
              << "KEY_SOUND_MAP_FILE" << std::endl;
}


void play(char * file, char * device) {
    int pid = fork();
    if (pid < 0) {
        std::cout << "Fork error" << std::endl;
        // Forkエラー
        return;
    }

    if (pid > 0) return; //親プロセスは何もしない
    
    // 子プロセスで再生する
    execl("/usr/bin/aplay", "aplay", device, file, nullptr);
}

char notes[128][256];

void callback_midi_input(double deltatime, std::vector<unsigned char> *message, void *userData)
{
	if ( message->size() > 0 ) {
		// printf("type = %d, %d [%f]\n", message->at(0), message->at(1), deltatime);

        switch((*message)[0] & 0b11110000) {
            // https://www.midi.org/specifications-old/item/table-1-summary-of-midi-message
            case 0b10010000:
                // 鍵盤が押された
                if (notes[message->at(1)][0]) play(notes[message->at(1)], (char *)userData);
                break;
            case 0b10000000:
                // 鍵盤が離れた
                break;
        }

    }
}

volatile sig_atomic_t err_flag = 0;

typedef struct _Config {
    char * map_file;
    char * create_device;
    char * connect_device;
    char * sound_device;
} Config;

int parseArgs(int argc, char * argv[], Config * config) {
    char * map_file = nullptr;
    static char create_device_default_name[12] = "Pianoice";
    static char default_sound_device[32] = "--device=default";
    char * create_device = create_device_default_name;
    char * connect_device = nullptr;
    char * sound_device = default_sound_device;

    do {
        int c =getopt(argc, argv, "c:s:d:");
        if (c == -1) {
            map_file = argv[optind];
            break;
        }

        switch(c) {
            case 'c':
                create_device = optarg;
                break;
            case 's':
                sprintf(sound_device, "--device=%s", optarg);
                break;
            case 'd':
                connect_device = optarg;
                break;
            case '?':
		        std::cout << "Unknown argment [" << (char)optopt << "]" << std::endl;
                break;
        }
    } while(true);

    bool valid_parameters = true;
    if (map_file == nullptr) {
        std::cout << "please map file" << std::endl;
        valid_parameters = false;
    }
    if (connect_device == nullptr) {
        std::cout << "please device name" << std::endl;
        valid_parameters = false;
    }
    if (!valid_parameters) return -1;
    
    _v(create_device);
    _v(connect_device);
    _v(map_file);

    config->create_device = create_device;
    config->connect_device = connect_device;
    config->map_file = map_file;
    config->sound_device = sound_device;

    return 0;
}

int main (int argc, char * argv[]) {
    Config config;
    if (parseArgs(argc, argv, &config) < 0) {
        usage();
        return 1;
    }

    FILE * fp = fopen(config.map_file, "r");
    if (fp == NULL) {
        std::cout << "Map file open error" << std::endl;
        return 1;
    }
    for(int i=0; i<128; i++) {
        char * t = notes[i];
        if (fgets(t, 256, fp) == NULL) {
            // エラー処理
            for (int j=i; j<128; j++) notes[j][0] = '\0';
            break;
        }

        // 改行文字が含まれているかの確認
        if (strchr(t, '\n') != NULL) {
            // 改行文字を終端記号に置換する
            t[strlen(t) - 1] = '\0';
        } else {
            // 入力ストリームをクリアする
            while(true) {
                int a = fgetc(fp);
                if (a == EOF) {
                    while(i<128) notes[i++][0] = '\0';
                    break;
                }
                if (a == '\n') break;
            }
        }

    }
    fclose(fp);
    
    MidiInterface * port = new MidiInterface(config.create_device);
    RtMidiIn * midiin = (RtMidiIn  *)port->connect(config.connect_device, MidiDirection::IN);
    midiin->setCallback(&callback_midi_input, config.sound_device);

    if (signal(SIGINT, [](int sig){err_flag = 1;}) == SIG_ERR) {
        std::cout << "Failed to regist SIGINT handler" << std::endl;
        return 1;
    }

    while(!err_flag);
    std::cout << "Exit" << std::endl;

    delete midiin;
    delete port;

    return 0;
}
