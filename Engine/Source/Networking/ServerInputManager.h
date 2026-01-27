#ifndef SERVERINPUTMANAGER_H
#define SERVERINPUTMANAGER_H

#include "NetworkProtocol.h"
#include <unordered_map>
#include <mutex>

namespace SquareCore {

class ServerInputManager {
public:
    ServerInputManager() = default;
    ~ServerInputManager() = default;

    // Queue an input from a client
    void QueueInput(const InputState& input);

    // Get the latest input for a specific client
    InputState GetInputForClient(uint32_t clientID) const;

    // Check if a client has any input
    bool HasInputForClient(uint32_t clientID) const;

    // Clear all processed inputs (called after each simulation frame)
    void ClearProcessedInputs();

    // Get all clients that have sent input this frame
    std::vector<uint32_t> GetActiveClients() const;

private:
    // Store the most recent input for each client
    mutable std::mutex inputMutex;
    std::unordered_map<uint32_t, InputState> currentInputs;
};

}

#endif
