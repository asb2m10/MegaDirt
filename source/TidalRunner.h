#pragma once

#include <juce_core/juce_core.h>
#include <boost/process.hpp>
#include <boost/process/v1/io.hpp>
#include <boost/process/v1/child.hpp>
#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <functional>

class ProcessRunner {
    boost::process::v1::ipstream instream;
    boost::process::v1::ipstream errstream;
    boost::process::v1::opstream outstream;
    boost::process::v1::child process;
    std::thread stdoutThread;
    std::thread stderrThread;
    std::function<void(const juce::String&)> callback;

    void threadReader(boost::process::v1::ipstream &in) {
        std::string line;
        while (in && std::getline(in, line)) {
            callback(juce::String(line));
        }
        callback("End of output for TIDAL");
    }

public:
    ProcessRunner(const std::string &exec, const std::string tidalPrompt, std::function<void(const juce::String&)> callback) :
        process(exec, "-ghci-script", tidalPrompt, boost::process::v1::std_out > instream, boost::process::v1::std_err > errstream, boost::process::v1::std_in < outstream),
        callback(std::move(callback)) {
        stdoutThread = std::thread(&ProcessRunner::threadReader, this, std::ref(instream));
        stderrThread = std::thread(&ProcessRunner::threadReader, this, std::ref(errstream));
    }

    ~ProcessRunner() {
        process.terminate();
        stdoutThread.join();
        stderrThread.join();
    }

    bool isRunning() {
        return process.running();
    }

    void sendString(juce::String msg) {
        outstream << msg << std::endl;
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

        tidalStartup = "~/src/MegaDirt/stidal.ghci";
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

        boost::process::v2::filesystem::path ghci_path =
            boost::process::v2::environment::find_executable("ghci");

        if ( ghci_path.empty() ) {
            juce::Logger::writeToLog("Haskell interpreter not found on system PATH");
            return;
        }

        process = std::make_unique<ProcessRunner>(ghci_path.string(), tidalStartup, stdoutCallback);
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

