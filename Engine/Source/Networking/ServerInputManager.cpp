#include "ServerInputManager.h"

namespace SquareCore {

void ServerInputManager::QueueInput(const InputState& input) {
    std::lock_guard<std::mutex> lock(inputMutex);
    currentInputs[input.clientID] = input;
}

InputState ServerInputManager::GetInputForClient(uint32_t clientID) const {
    std::lock_guard<std::mutex> lock(inputMutex);

    auto it = currentInputs.find(clientID);
    if (it != currentInputs.end()) {
        return it->second;
    }

    // Return empty input if client hasn't sent any
    InputState emptyInput;
    emptyInput.clientID = clientID;
    return emptyInput;
}

bool ServerInputManager::HasInputForClient(uint32_t clientID) const {
    std::lock_guard<std::mutex> lock(inputMutex);
    return currentInputs.find(clientID) != currentInputs.end();
}

void ServerInputManager::ClearProcessedInputs() {
    std::lock_guard<std::mutex> lock(inputMutex);
}

std::vector<uint32_t> ServerInputManager::GetActiveClients() const {
    std::lock_guard<std::mutex> lock(inputMutex);

    std::vector<uint32_t> clients;
    clients.reserve(currentInputs.size());

    for (const auto& [clientID, input] : currentInputs) {
        clients.push_back(clientID);
    }

    return clients;
}

}
