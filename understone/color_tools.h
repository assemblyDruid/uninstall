#ifndef __UE_COLOR_TOOLS_H__
#define __UE_COLOR_TOOLS_H__

#include "debug_tools.h"
#include "maths_tools.h"
#include "type_tools.h"

#include <stdbool.h>

static u8
BindValueTo8BitColorChannel(const r32 value_min, const r32 value_max, const r32 value)
{
    uAssert(value_max > value_min);
    uAssert((value_max >= value) && (value_min <= value));

    return (u8)NormalizeToRange(value_min, value_max, 0.0f, 255.0f, value);
}

static void
RGB32ToHSV32(const Color32RGB* restrict const rgb_source, Color32HSV* restrict const hsv_result)
{
    uAssert(rgb_source);
    uAssert(hsv_result);

    // Normalize [ TOLERANCE, 1.0f ]
    r32 rgb_r = NormalizeToRange(0.0f, 255.0f, 0.0f, 1.0f, rgb_source->channel.R);

    r32 rgb_g = NormalizeToRange(0.0f, 255.0f, 0.0f, 1.0f, rgb_source->channel.G);

    r32 rgb_b = NormalizeToRange(0.0f, 255.0f, 0.0f, 1.0f, rgb_source->channel.B);

    r32 rgb_min = rgb_r;
    r32 rgb_max = rgb_r;
    r32 chroma  = 0.0f;

    // Determine min rgb component
    if (rgb_min > rgb_g)
    {
        rgb_min = rgb_g;
    }

    if (rgb_min > rgb_b)
    {
        rgb_min = rgb_b;
    }

    // Determine max rgb component
    if (rgb_max < rgb_g)
    {
        rgb_max = rgb_g;
    }

    if (rgb_max < rgb_b)
    {
        rgb_max = rgb_b;
    }

    // Assign chroma
    uAssert(rgb_max >= rgb_min);
    chroma = rgb_max - rgb_min;

    // Asign value
    r32 value = rgb_max;
    uAssert(value >= 0.0f);
    uAssert(value <= 360.0f);

    // Assign hue
    r32 hue = 0.0f;
    if (chroma == 0.0f)
    {
        // Intentional nop
    }
    else if (value == rgb_r)
    {
        hue = 60.0f * ((rgb_g - rgb_b) / chroma);
    }
    else if (value == rgb_g)
    {
        hue = 60.0f * (2.0f + ((rgb_b - rgb_r) / chroma));
    }
    else if (value == rgb_b)
    {
        hue = 60.0f * (4.0f + ((rgb_r - rgb_g) / chroma));
    }
    else
    {
        uAssert(false);
    }

    if (hue < 0.0f)
    {
        hue += 360.0f;
    }

    uAssert(hue >= 0.0f);
    uAssert(hue <= 360.0f);

    // Assign saturation
    r32 saturation = 0.0f;
    if (value != 0.0f)
    {
        saturation = chroma / value;
    }

    hsv_result->H = hue;
    hsv_result->S = saturation;
    hsv_result->V = value;
}

static void
HSV32ToRGB32(const Color32HSV* restrict const hsv_source, Color32RGB* restrict const rgb_result)
{
    uAssert(rgb_result);
    uAssert(hsv_source);
    uAssert(hsv_source->H >= 0.0f);
    uAssert(hsv_source->H <= 360.0f);
    uAssert(hsv_source->S >= 0.0f);
    uAssert(hsv_source->S <= 1.0f);
    uAssert(hsv_source->V >= 0.0f);
    uAssert(hsv_source->V <= 1.0f);

    // Assign hue_prime
    r32 hue_prime = hsv_source->H / 60.0f;

    // Assign chroma
    r32 chroma = hsv_source->V * hsv_source->S;
    uAssert(chroma >= 0.0f);
    uAssert(chroma <= 1.0f);

    // Assign 2nd largest color component
    r32 secondary_color = chroma * (1.0f - (r32)fabs(fmod(hue_prime, 2.0f) - 1.0f));
    uAssert(secondary_color >= 0.0f);
    uAssert(secondary_color <= 1.0f);

    // Compute rgb components
    r32 rgb_r = 0.0f;
    r32 rgb_g = 0.0f;
    r32 rgb_b = 0.0f;

    bool build_color_channels = true;
    r32  lightness_component  = hsv_source->V - chroma;
    uAssert(lightness_component >= 0.0f);
    if (hue_prime < 0.0f)
    {
        build_color_channels = false;
    }

    uAssert(hue_prime <= 6.0f);
    if (build_color_channels)
    {
        if (hue_prime >= 0.0f && hue_prime <= 1.0f)
        {
            rgb_g = secondary_color;
            rgb_r = chroma;
        }
        if (hue_prime > 1.0f && hue_prime <= 2.0f)
        {
            rgb_r = secondary_color;
            rgb_g = chroma;
        }
        if (hue_prime > 2.0f && hue_prime <= 3.0f)
        {
            rgb_b = secondary_color;
            rgb_g = chroma;
        }
        if (hue_prime > 3.0f && hue_prime <= 4.0f)
        {
            rgb_g = secondary_color;
            rgb_b = chroma;
        }
        if (hue_prime > 4.0f && hue_prime <= 5.0f)
        {
            rgb_r = secondary_color;
            rgb_b = chroma;
        }
        if (hue_prime > 5.0f && hue_prime <= 6.0f)
        {
            rgb_b = secondary_color;
            rgb_r = chroma;
        }
    }

    rgb_r += lightness_component;
    rgb_g += lightness_component;
    rgb_b += lightness_component;

    uAssert(rgb_r >= 0.0f);
    uAssert(rgb_g >= 0.0f);
    uAssert(rgb_b >= 0.0f);
    uAssert(rgb_r <= 1.0f);
    uAssert(rgb_g <= 1.0f);
    uAssert(rgb_b <= 1.0f);

    // Assign rgb components
    rgb_result->channel.R = (u8)round(NormalizeToRange(0.0f, 1.0f, 0.0f, 255.0f, rgb_r));

    rgb_result->channel.G = (u8)round(NormalizeToRange(0.0f, 1.0f, 0.0f, 255.0f, rgb_g));

    rgb_result->channel.B = (u8)round(NormalizeToRange(0.0f, 1.0f, 0.0f, 255.0f, rgb_b));
}

#endif // __UE_COLOR_TOOLS_H__
