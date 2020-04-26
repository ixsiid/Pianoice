#include <iostream>
#include "midi.hpp"

int main () {
    MidiInterface * p = new MidiInterface("foobar");
    p->connect("Midi Through", MidiDirection::OUT);
    while (true) {
         std::cin.get();
    }
    exit(0);
}





