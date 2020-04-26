#include "midi.hpp"


MidiInterface::MidiInterface(const char * interfaceName) {
    name = interfaceName;
}

RtMidi * MidiInterface::connect(std::string target, MidiDirection direction) {
    char clientName[256] = "";
    sprintf(clientName, "%s Midi %s Client", name, direction == MidiDirection::IN ? "Input" : "Output");
    RtMidi * midi = direction == MidiDirection::IN ? 
        (RtMidi *)new RtMidiIn (RtMidi::UNSPECIFIED, clientName) :
        (RtMidi *)new RtMidiOut(RtMidi::UNSPECIFIED, clientName);

    unsigned int portCount = midi->getPortCount();
    for (unsigned int i=0; i<portCount; i++) {
        std::string portName = midi->getPortName(i).c_str();
        std::string::size_type index = portName.find(target, 0);
        if (index != std::string::npos) {
            char portName[256] = "";
            sprintf(portName, "%s Midi %s Port", name, direction == MidiDirection::IN ? "Input" : "Output");
            midi->openPort(i, portName);
            return midi;
        }
    }
    return NULL;
}


