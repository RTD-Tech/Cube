#ifndef NETWORKMODE_H
#define NETWORKMODE_H

namespace SquareCore {

// Network mode enumeration
enum class NetworkMode {
    STANDALONE,  // Single-player, no networking
    CLIENT,      // Connected to a server
    SERVER       // Running as server
};

}

#endif
