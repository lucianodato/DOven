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

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

DOvenPlugin::DOvenPlugin()
    : Plugin(paramCount, 1, 0) // 1 program, 0 states
{
    // set default values
    loadProgram(0);

    // set latency
    setLatency(frameSize);

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
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Heat";
        parameter.symbol = "heat";
        parameter.unit = " ";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 0.9f;
        break;

    case paramPolyB:
        parameter.hints = kParameterIsAutomable | kParameterIsBoolean;
        parameter.name = "Antialiasing ON";
        parameter.symbol = "aliasing";
        parameter.unit = " ";
        parameter.ranges.def = 0.0f;
        parameter.ranges.min = 0.0f;
        parameter.ranges.max = 1.0f;
        break;


    case paramGain:
        parameter.hints = kParameterIsAutomable;
        parameter.name = "Gain";
        parameter.symbol = "gain";
        parameter.unit = "dB";
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
    case paramPolyB:
        return fPolyB;
    default:
        return 0.f;
    }
}

void DOvenPlugin::setParameterValue(uint32_t index, float value)
{
    if (getSampleRate() <= 0.f)
        return;

    switch (index)
    {
    case paramHeat:
        fHeat = value;
        break;
    case paramGain:
        fGain = value;
        break;
    case paramPolyB:
        fPolyB = value;
        break;
    }
}

void DOvenPlugin::loadProgram(uint32_t index)
{
    if (index != 0)
        return;

    // Default values
    fHeat = 0.0f;
    fGain = 0.f;
    fPolyB = false;

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
    const float *in1 = inputs[0];
    float *out1 = outputs[0];
    float L = (1.f - fHeat);

    for (uint32_t i = 0; i < frames; ++i)
    {
        left = in1[i] + denormalGuard;

        /*
            Clipping
        */
        if (fHeat > 0.f)
        {
            //Piece-wise clipping
            if (fabs(left) <= L)
            {
                //Apply distortion and compensate for gain rise
                yn = yamahaDistortion(left)*fromDb(-3.f);
                clipping_flag = 0;
            }
            else
            {
                yn = sign(left)*L; // Hard Clipping
                clipping_flag = 1;
            }

            if (fPolyB)
            {
                if ((clipping_flag - clipping_flag_n1) != 0)
                {
                    flag_poly = 1;
                }
                else if (flag_poly == 1)
                {
                    flag_poly = 0;
                    
                    a = (-1.f/6.f)*xn3 + (1.f/2.f)*xn2 - (1.f/2.f)*xn1 + (1.f/6.f)*left;
                    b = xn3 - (5.f/2.f)*xn2 + 2.f*xn1 - (1.f/2.f)*left;
                    c = (-11.f/6.f)*xn3 + 3.f*xn2 - (3.f/2.f)*xn1 + (1.f/3.f)*left;
                    e = xn3;

                    // Newton - Raphson
                    x_n = 1.5f;

                    for (uint32_t m = 0; m < 100; m++)
                    {
                        err = (a*powf(x_n, 3.f) + b*powf(x_n, 2.f) + c*x_n + e - sign(xn2)*L) / (3.f*a*powf(x_n, 2.f) + 2.f*b*x_n + c);
                        
                        if (fabs(err) > 0.00001f)
                        {
                            x_n = x_n - err;
                        }
                        else
                        {
                            break;
                        }
                    }

                    // Slope at clipping point
                    mu = 3.f*a*powf(x_n, 2.f) + 2.f*b*x_n + c;

                    // Fractional part of clipping point
                    d = x_n - 1.f;

                    h0 = -powf(d, 5.f)/120.f + powf(d, 4.f)/24.f - powf(d, 3.f)/12.f + powf(d, 2.f)/12.f - d/24.f + 1.f/120.f;
                    h1 = powf(d, 5.f)/40.f - powf(d, 4.f)/12.f + powf(d, 2.f)/3.f - d/2.f + 7.f/30.f;
                    h2 = -powf(d, 5.f)/40.f + powf(d, 4.f)/24.f + powf(d, 3.f)/12.f + powf(d, 2.f)/12.f + d/24.f + 1.f/120.f;
                    h3 = powf(d, 5.f)/120.f;

                    mu = fabs(mu);
                    pol = sign(xn2);

                    yn3 = yn3 - pol*h0*mu;
                    yn2 = yn2 - pol*h1*mu;
                    yn1 = yn1 - pol*h2*mu;
                    yn = yn - pol*h3*mu;
                }               
            }

            clipping_flag_n1 = clipping_flag;
        }
        else
        {
            yn = left;
        }
        
        // Move samples back
        out1[i] = yn3;
        yn3 = yn2;
        yn2 = yn1;
        yn1 = yn;

        xn3 = xn2;
        xn2 = xn1;
        xn1 = left; 

        /*
            Output Gain
        */
        if (fGain != 0.f)
        {
            out1[i] = out1[i] * fromDb(fGain);
        }
    }
}

// -----------------------------------------------------------------------

Plugin *createPlugin()
{
    return new DOvenPlugin();
}

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO
