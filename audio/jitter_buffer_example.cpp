#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <chrono>

// Constants
const int BUFFER_SIZE = 100;  // Size of the circular buffer
const int PLAYOUT_DELAY_MS = 20;  // Playout delay in milliseconds

// Audio packet structure
struct AudioPacket {
    int sequenceNumber;
    std::string data;
};

// Circular buffer class
class JitterBuffer {
public:
    JitterBuffer() : readIndex_(0), writeIndex_(0) {}

    void PushPacket(const AudioPacket& packet) {
        // Store the packet in the buffer map
        buffer_[packet.sequenceNumber] = packet;
        writeIndex_ = packet.sequenceNumber;
    }

    AudioPacket PopPacket() {
        // Retrieve and remove the next sequential packet from the buffer
        auto packetIter = buffer_.find(readIndex_);
        if (packetIter != buffer_.end()) {
            AudioPacket packet = packetIter->second;
            buffer_.erase(packetIter);
            readIndex_ = packet.sequenceNumber + 1;
            return packet;
        }
        return {};
    }

    bool IsEmpty() const {
        return buffer_.empty();
    }

private:
    std::map<int, AudioPacket> buffer_;
    int readIndex_;
    int writeIndex_;
};

// Audio player class
class AudioPlayer {
public:
    void PlayPacket(const AudioPacket& packet) {
        // Simulate audio playback
        std::cout << "Playing packet: " << packet.sequenceNumber << std::endl;
    }
};

// Jitter buffer worker function
void JitterBufferWorker(JitterBuffer& jitterBuffer, AudioPlayer& audioPlayer) {
    while (true) {
        // Check if there's a packet available in the buffer
        if (!jitterBuffer.IsEmpty()) {
            // Retrieve the packet from the buffer
            AudioPacket packet = jitterBuffer.PopPacket();

            // Play out the packet
            audioPlayer.PlayPacket(packet);

            // Simulate playout delay
            std::this_thread::sleep_for(std::chrono::milliseconds(PLAYOUT_DELAY_MS));
        } else {
            // Sleep for a short time if the buffer is empty
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}

int main() {
    JitterBuffer jitterBuffer;
    AudioPlayer audioPlayer;

    // Start the jitter buffer worker thread
    std::thread workerThread(JitterBufferWorker, std::ref(jitterBuffer), std::ref(audioPlayer));

    // Simulate receiving audio packets with disorder
    for (int i = 0; i < 10; ++i) {
        // Generate a sample audio packet
        AudioPacket packet;
        packet.sequenceNumber = i;
        packet.data = "AudioPacket" + std::to_string(i);

        // Push the packet into the jitter buffer
        jitterBuffer.PushPacket(packet);

        // Sleep for a short time before sending the next packet
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Wait for the worker thread to finish
    workerThread.join();

    return 0;
}
