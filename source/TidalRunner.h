#pragma once

#include <juce_core/juce_core.h>
#include "ext/subprocess.hpp"
#include <stdio.h>

/**
 * Very cheap process runner that might block on IO.
 */
class ProcessRunner {
    subprocess::Popen process;
    std::thread stdoutThread;
    std::thread stderrThread;
    std::function<void(const juce::String&)> callback;

    void threadReader(FILE *in) {
        char line[4096];
        while (fgets(line, 4095, in) != nullptr) {
            line[strnlen(line, 4095)-1] = 0; // remove newline
            callback(juce::String(juce::CharPointer_UTF8(line)));
        }

        callback("End of output for TIDAL");
    }

public:
    ProcessRunner(const std::string &exec, const std::string tidalPrompt, std::function<void(const juce::String&)> callback) :
        process({exec, "-ghci-script", tidalPrompt}, subprocess::input{subprocess::PIPE}, subprocess::output{subprocess::PIPE}, subprocess::error{subprocess::PIPE}),
        callback(std::move(callback)) {
        stdoutThread = std::thread(&ProcessRunner::threadReader, this, process.output());
        stderrThread = std::thread(&ProcessRunner::threadReader, this, process.error());
    }

    ~ProcessRunner() {
        process.kill(SIGTERM);
        stdoutThread.join();
        stderrThread.join();
    }

    bool isRunning() {
        return process.poll();
    }

    void sendString(juce::String msg) {
        fputs(msg.toRawUTF8(), process.input());
        fputc('\n', process.input());
    }
};


class TidalRunner {
    std::string tidalStartup;
    std::unique_ptr<ProcessRunner> process;

public:
    std::function<void(const juce::String &line)> stdoutCallback;
    std::function<void(const juce::String &line)> stdinCallback;

    TidalRunner() {
        stdinCallback = [this](const juce::String &line) {
            sendString(line);
        };
    }

    ~TidalRunner() {
    }

    void setTidalStartup(const juce::String newTidalStartup) {
        tidalStartup = newTidalStartup.toStdString();
    }

    void startTidal() {
        if ( process != nullptr && process->isRunning() ) {
            juce::Logger::writeToLog("Tidal is already running");
            return;
        }
        try {
            process = std::make_unique<ProcessRunner>("ghci", tidalStartup, stdoutCallback);
        } catch (std::runtime_error &e) {
            juce::Logger::writeToLog("Failed to start Tidal (ghci): " + juce::String(e.what()));
        }
    }
    void stopTidal() {
        process.reset();
    }
    void sendString(juce::String toSend) {
        if ( process != nullptr && process->isRunning() )
            process->sendString(toSend);
        else
            juce::Logger::writeToLog("Tidal is not running");
    }
};

