#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include <wiringPi.h>
#include <wiringSerial.h>
#include <iostream>

class MainContentComponent   : public AudioAppComponent,
                               public Slider::Listener
{
public:
    MainContentComponent()
    :   currentSampleRate (0.0),
        currentAngle (0.0),
        angleDelta (0.0)
    {
        addAndMakeVisible (frequencySlider);
        frequencySlider.setRange (50.0, 5000.0);
        frequencySlider.setSkewFactorFromMidPoint (500.0);
        frequencySlider.addListener (this);

        setSize (600, 100);
        setAudioChannels (0, 1); // no inputs, one output
    }

    ~MainContentComponent()
    {
        shutdownAudio();
    }

    void resized() override
    {
        frequencySlider.setBounds (10, 10, getWidth() - 20, 20);
    }

    void sliderValueChanged (Slider* slider) override
    {
        if (slider == &frequencySlider)
        {
            if (currentSampleRate > 0.0)
                updateAngleDelta();
        }
    }

    void updateAngleDelta()
    {
        const double cyclesPerSample = frequencySlider.getValue() / currentSampleRate;
        angleDelta = cyclesPerSample * 2.0 * double_Pi;
    }

    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        currentSampleRate = sampleRate;
        updateAngleDelta();
    }

    void releaseResources() override
    {
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        const float level = 0.125f;
        float* const buffer = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);

        for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            const float currentSample = (float) std::sin (currentAngle);
            currentAngle += angleDelta;
            buffer[sample] = currentSample * level;
        }
    }

    void receivePotInput(int val)
    {
        double freqVal = val * (4950 / 255) + 50.0;
        frequencySlider.setValue(freqVal);
    }

private:
    Slider frequencySlider;
    double currentSampleRate, currentAngle, angleDelta; // [1]

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


class ThreadedMainComponent : public Thread,
                              public MainContentComponent
{
public:
    ThreadedMainComponent()
         : Thread ("Poll serial thread"),
           MainContentComponent()
    {
        fd = serialOpen("/dev/ttyACM0", 115200);
        if (fd == -1 ) {
            std::cout << "Error opening device";
        }
        wiringPiSetup();
    }

    ~ThreadedMainComponent()
    {
        serialClose(fd);
    }

    void run() override
    {
        while (!threadShouldExit()) {
            if(serialDataAvail(fd)) {
                const MessageManagerLock mml (Thread::getCurrentThread());

                if (! mml.lockWasGained()) {
                    return;
                }
                int val = serialGetchar(fd);
                receivePotInput(val);
            }
        }
    }
private:
    int fd;
};

Component* createMainContentComponent() {
    ThreadedMainComponent *component = new ThreadedMainComponent();
    component->startThread();
    return component;
}

#endif  // MAINCOMPONENT_H_INCLUDED
