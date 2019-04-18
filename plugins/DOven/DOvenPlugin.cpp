/*
 * D'Oven Plugin
 * Copyright (C) 2019 Luciano Dato <lucianodato@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * For a full copy of the license see the LICENSE file.
 */

#include "DOvenPlugin.hpp"

#include <cmath>

#define FROM_DB(v) ((v) > -90.0f ? expf(v / 20.f * logf(10.f)) : 0.0f)
#define TO_DB(v) ((v) > -90.0f ? 20.f * log10(v) : 0.0f)
#define SIGN(v) ((v) >= 0.f ? 1.f : -1.f)
#define FIXDENORMAL(v) ((!std::isnormal(v)) ? 0.f : v)
#define YAMAHA_WAVESHAPER(v) (1.5f * v - (powf(v, 3) / 2.f)) // Yamaha function from Old digital racks


START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

DOvenPlugin::DOvenPlugin()
    : Plugin(paramCount, 1, 0) // 1 program, 0 states
{
    // set default values
    loadProgram(0);

    // reset
    deactivate();
}

// -----------------------------------------------------------------------
// Init

void DOvenPlugin::initParameter(uint32_t index, Parameter &parameter)
{
    switch (index)
    {
    case paramHeat:
        parameter.hints      = kParameterIsAutomable;
        parameter.name       = "Heat";
        parameter.symbol     = "heat";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;

    case paramGain:
        parameter.hints      = kParameterIsAutomable;
        parameter.name       = "Gain";
        parameter.symbol     = "gain";
        parameter.unit       = "dB";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = -10.0f;
        parameter.ranges.max = 10.0f;
        break;
    }
}

void DOvenPlugin::initProgramName(uint32_t index, String &programName)
{
    if (index != 0)
        return;

    programName = "Default";
}

// -----------------------------------------------------------------------
// Internal data

float DOvenPlugin::getParameterValue(uint32_t index) const
{
    switch (index)
    {
    case paramHeat:
        return fHeat;
    case paramGain:
        return fGain;
    default:
        return 0.0f;
    }
}

void DOvenPlugin::setParameterValue(uint32_t index, float value)
{
    if (getSampleRate() <= 0.0)
        return;

    switch (index)
    {
    case paramHeat:
        fHeat = value;
        break;
    case paramGain:
        fGain = value;
        break;
    }
}

void DOvenPlugin::loadProgram(uint32_t index)
{
    if (index != 0)
        return;

    // Default values
    fHeat  = 0.0f;
    fGain = 0.f;

    // Activate
    activate();
}

// -----------------------------------------------------------------------
// DSP functions


// -----------------------------------------------------------------------
// Process

void DOvenPlugin::activate()
{
}

void DOvenPlugin::deactivate()
{
}

void DOvenPlugin::run(const float **inputs, float **outputs, uint32_t frames)
{
    const float* in1  = inputs[0];
    float*       out1 = outputs[0];
    float left;

    for (uint32_t i=0; i < frames; ++i)
    {
        //Input
        left = in1[i];

        /*
            Clipping
        */
        if (fHeat > 0.f){
            //Piece-wise clipping
            if (fabs(left) < (1.f - fHeat))
            {
                left = YAMAHA_WAVESHAPER(left);
            }
            else
            {
                left = SIGN(left) * (1.f - fHeat); // Hard Clipping
            }

            //Compensate for gain rise
            left = left * FROM_DB(-3.f);
        }

        /*
            Output Gain
        */
        if (fGain != 0.f)
        {
            left = left * FROM_DB(fGain);
        }

        //Output
        out1[i] = left;
    }
}

// -----------------------------------------------------------------------

Plugin* createPlugin()
{
    return new DOvenPlugin();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
