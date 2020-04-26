#include <iostream>
#include <rtmidi/RtMidi.h>

enum struct MidiDirection {
    IN,
    OUT,
};

class MidiInterface {
private:
    const char * name;
public:
    MidiInterface(const char * interfaceName);
    RtMidi * connect(std::string target, MidiDirection direction);
};

