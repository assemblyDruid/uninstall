#ifndef __UE_IMAGE_TOOLS_H__
#define __UE_IMAGE_TOOLS_H__

#include <engine_tools/debug_tools.h>
#include <engine_tools/memory_tools.h>
#include <engine_tools/type_tools.h>
#include <stdbool.h>
#ifdef _linux_
#include <unistd.h>
#endif

uMemoryArena* imageArena;
#define imagePushData(new_data, type) uMAPushData(imageArena, new_data, type)
#define imagePushArray(new_data, type, num_bytes) \
    uMAPushArray(imageArena, new_data, type, num_bytes)
/* #define imageAlloc(type, num_bytes) uMAAllocate(imageArena, type, num_bytes)
 */

// Note: used in ms bitmap
#define uBI_RGB       0x0000
#define uBI_RLE8      0x0001
#define uBI_RLE4      0x0002
#define uBI_BITFIELDS 0x0003
#define uBI_JPEG      0x0004
#define uBI_PNG       0x0005
#define uBI_CMYK      0x000B
#define uBI_CMYKRLE8  0x000C
#define uBI_CMYKRLE4  0x000D

typedef struct
{
    size_t      img_height;
    size_t      img_width;
    u8*         img_start;
    u8*         img_end;
    u8*         img_cursor;
    uEndianness endian;
    u8*         img_pixels;
} uImage;

u8
uReadNextByte(const restrict uImage* const img)
{
    if (img->img_cursor < img->img_end)
    {
        return *img->img_cursor++;
    }

    uError_v("uReadNextByte(): Out of bounds.\n");
    return 0;
}

u16
uRead16AsLE(const restrict uImage* const img)
{
    u16 tmp = uReadNextByte(img);
    return tmp + (uReadNextByte(img) << 8);
}

u32
uRead32AsLE(const restrict uImage* const img)
{
    u32 tmp = uRead16AsLE(img);
    return tmp + (uRead16AsLE(img) << 16);
}

bool
uLoadBitmap(const restrict char* const file_path, const restrict uImage* const img)
{
    printf("TODO: uLoadBitmap()"); // [ cfarvin::TODO ]
    return false;
}

static u8
BindValueTo8BitColorChannel(const r32 value_min, const r32 value_max, const r32 value)
{
    __UE_ASSERT__(value_max > value_min);
    __UE_ASSERT__((value_max >= value) && (value_min <= value));

    return (u8)NormalizeToRange(value_min, value_max, 0.0f, 255.0f, value);
}

static void
RGB32ToHSV32(const Color32_RGB* restrict const rgb_source,
             _mut_ Color32_HSV* restrict const hsv_result)
{
    __UE_ASSERT__(rgb_source);
    __UE_ASSERT__(hsv_result);

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
    __UE_ASSERT__(rgb_max >= rgb_min);
    chroma = rgb_max - rgb_min;

    // Asign value
    r32 value = rgb_max;
    __UE_ASSERT__(value >= 0.0f);
    __UE_ASSERT__(value <= 360.0f);

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
        __UE_ASSERT__(false);
    }

    if (hue < 0.0f)
    {
        hue += 360.0f;
    }

    __UE_ASSERT__(hue >= 0.0f);
    __UE_ASSERT__(hue <= 360.0f);

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
HSV32ToRGB32(const Color32_HSV* restrict const hsv_source,
             _mut_ Color32_RGB* restrict const rgb_result)
{
    __UE_ASSERT__(rgb_result);
    __UE_ASSERT__(hsv_source);
    __UE_ASSERT__(hsv_source->H >= 0.0f);
    __UE_ASSERT__(hsv_source->H <= 360.0f);
    __UE_ASSERT__(hsv_source->S >= 0.0f);
    __UE_ASSERT__(hsv_source->S <= 1.0f);
    __UE_ASSERT__(hsv_source->V >= 0.0f);
    __UE_ASSERT__(hsv_source->V <= 1.0f);

    // Assign hue_prime
    r32 hue_prime = hsv_source->H / 60.0f;

    // Assign chroma
    r32 chroma = hsv_source->V * hsv_source->S;
    __UE_ASSERT__(chroma >= 0.0f);
    __UE_ASSERT__(chroma <= 1.0f);

    // Assign 2nd largest color component
    r32 secondary_color = chroma * (1.0f - (r32)fabs(fmod(hue_prime, 2.0f) - 1.0f));
    __UE_ASSERT__(secondary_color >= 0.0f);
    __UE_ASSERT__(secondary_color <= 1.0f);

    // Compute rgb components
    r32 rgb_r = 0.0f;
    r32 rgb_g = 0.0f;
    r32 rgb_b = 0.0f;

    bool build_color_channels = true;
    r32  lightness_component  = hsv_source->V - chroma;
    __UE_ASSERT__(lightness_component >= 0.0f);
    if (hue_prime < 0.0f)
    {
        build_color_channels = false;
    }

    __UE_ASSERT__(hue_prime <= 6.0f);
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

    __UE_ASSERT__(rgb_r >= 0.0f);
    __UE_ASSERT__(rgb_g >= 0.0f);
    __UE_ASSERT__(rgb_b >= 0.0f);
    __UE_ASSERT__(rgb_r <= 1.0f);
    __UE_ASSERT__(rgb_g <= 1.0f);
    __UE_ASSERT__(rgb_b <= 1.0f);

    // Assign rgb components
    rgb_result->channel.R = (u8)round(NormalizeToRange(0.0f, 1.0f, 0.0f, 255.0f, rgb_r));

    rgb_result->channel.G = (u8)round(NormalizeToRange(0.0f, 1.0f, 0.0f, 255.0f, rgb_g));

    rgb_result->channel.B = (u8)round(NormalizeToRange(0.0f, 1.0f, 0.0f, 255.0f, rgb_b));
}

static void
WritePPM32(const restrict Color32_RGB* const pixel_array,
           u32                               image_width,
           u32                               image_height,
           const restrict char* const        image_name)
{
    __UE_ASSERT__(pixel_array && image_width && image_height);

    FILE* ppm_file = fopen(image_name, "w");
    __UE_ASSERT__(ppm_file);

    char ppm_header[MAX_PPM_HEADER_SIZE];
    u32  success = snprintf(ppm_header,
                           MAX_PPM_HEADER_SIZE,
                           "P3\n%d %d\n255\n",
                           image_width,
                           image_height);
    __UE_ASSERT__((success > 0) && (success < MAX_PPM_HEADER_SIZE));
    fwrite(ppm_header, success, 1, ppm_file);

    // [ cfarvin::TODO ] This is as temporary as it is bad.
    char   rgb_tripplet[MAX_PPM_TRIPPLET_SIZE];
    size_t pix_idx = 0;
    for (size_t y = 0; y < image_height; y++)
    {
        for (size_t x = 0; x < image_width; x++)
        {
            for (char idx = 0; idx < MAX_PPM_TRIPPLET_SIZE; idx++)
            {
                rgb_tripplet[(size_t)idx] = '\0';
            }

            success = snprintf(rgb_tripplet,
                               MAX_PPM_TRIPPLET_SIZE,
                               "%d %d %d\n",
                               pixel_array[pix_idx].channel.R,
                               pixel_array[pix_idx].channel.G,
                               pixel_array[pix_idx].channel.B);

            __UE_ASSERT__(success > 0);
            __UE_ASSERT__(success < MAX_PPM_TRIPPLET_SIZE);

            fwrite(rgb_tripplet, success, 1, ppm_file);
            pix_idx++;
        }
    }

    __UE_ASSERT__(pix_idx == (image_height * image_width));
    fclose(ppm_file);
}

static void
WriteBitmap32(const restrict Color32_RGB* const pixel_array,
              u32                               image_width,
              u32                               image_height,
              const char* const                 image_name)

{
    __UE_ASSERT__(pixel_array && image_width && image_height);
    u32 pixel_array_size = sizeof(u32) * image_width * image_height;

    BitmapHeader bitmap_header       = { 0 };
    bitmap_header.magic_number       = 0x4D42;
    bitmap_header.file_size          = pixel_array_size + sizeof(BitmapHeader);
    bitmap_header.pix_arr_offset     = sizeof(BitmapHeader);
    bitmap_header.bitmap_header_size = sizeof(BitmapHeader) - 14;
    bitmap_header.bitmap_width       = image_width;
    bitmap_header.bitmap_height      = image_height;
    bitmap_header.num_color_planes   = 1;
    bitmap_header.bits_per_pix       = 32;
    bitmap_header.pix_arr_size       = pixel_array_size;

    FILE* bitmap_file = fopen(image_name, "wb");
    __UE_ASSERT__(bitmap_file);

    fwrite(&bitmap_header, sizeof(BitmapHeader), 1, bitmap_file);
    fwrite(pixel_array, pixel_array_size, 1, bitmap_file);

    fclose(bitmap_file);
}

#endif // __UE_IMAGE_TOOLS_H__
