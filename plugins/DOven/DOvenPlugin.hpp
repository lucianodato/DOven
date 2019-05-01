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

#ifndef DOVEN_PLUGIN_HPP_INCLUDED
#define DOVEN_PLUGIN_HPP_INCLUDED

#include "DistrhoPlugin.hpp"

static const float denormalGuard = 1e-15f; // http://www.earlevel.com/main/2019/04/19/floating-point-denormals/
static const uint32_t frameSize = 4;

inline float fromDb(float v) { return ((v) > -90.0f ? expf(v / 20.f * logf(10.f)) : 0.0f); }
inline float toDb(float v) { return ((v) > -90.0f ? 20.f * log10(v) : 0.0f); }
inline float sign(float v) { return ((v) >= 0.f ? 1.f : -1.f); }
inline float yamahaDistortion(float v) { return (1.5f * v - (powf(v, 3) / 2.f)); } // Yamaha function from Old digital racks

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------

class DOvenPlugin : public Plugin
{
public:
    enum Parameters
    {
        paramHeat,
        paramPolyB,
        paramGain,
        paramCount
    };

    DOvenPlugin();

protected:
    // -------------------------------------------------------------------
    // Information

    const char* getLabel() const noexcept override
    {
        return "D'Oven";
    }

    const char* getDescription() const override
    {
        return "Digital Saturator to Cook your tracks";
    }

    const char* getMaker() const noexcept override
    {
        return "Luciano Dato";
    }

    const char* getHomePage() const override
    {
        return "https://github.com/lucianodato/DOven";
    }

    const char* getLicense() const noexcept override
    {
        return "GPL";
    }

    uint32_t getVersion() const noexcept override
    {
        return d_version(1, 0, 0);
    }

    int64_t getUniqueId() const noexcept override
    {
        return d_cconst('D', 'O', 'V', 'N');
    }

    // -------------------------------------------------------------------
    // Init

    void initParameter(uint32_t index, Parameter& parameter) override;
    void initProgramName(uint32_t index, String& programName) override;

    // -------------------------------------------------------------------
    // Internal data

    float getParameterValue(uint32_t index) const override;
    void  setParameterValue(uint32_t index, float value) override;
    void  loadProgram(uint32_t index) override;

    // -------------------------------------------------------------------
    // DSP functions


    // -------------------------------------------------------------------
    // Process

    void activate() override;
    void deactivate() override;
    void run(const float** inputs, float** outputs, uint32_t frames) override;

    // -------------------------------------------------------------------

private:
    float fHeat;
    bool fPolyB;
    float fGain;
    float mu, d;
    float xn1 = 0.f, xn2 = 0.f, xn3 = 0.f, yn = 0.f, yn1 = 0.f, yn2 = 0.f, yn3 = 0.f;
    uint32_t clipping_flag = 0;
    uint32_t clipping_flag_n1 = 1;
    uint32_t flag_poly = 0;    
    float a, b, c, e, x_n, err, h0, h1, h2, h3, pol;
    float left;


    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DOvenPlugin)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif // DOVEN_PLUGIN_HPP_INCLUDED
