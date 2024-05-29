#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lua.h"
#include "lauxlib.h"
#include "png.h"

/**
 * convert input from HWC format to CHW format
 * \param input A single image. The byte array has length of 3*h*w
 * \param h image height
 * \param w image width
 * \param output A float array. should be freed by caller after use
 * \param output_count Array length of the `output` param
 */
void hwc_to_chw(const uint8_t* input, size_t h, size_t w, float** output, size_t* output_count) {
  size_t stride = h * w;
  *output_count = stride * 3;
  float* output_data = (float*)malloc(*output_count * sizeof(float));
  for (size_t i = 0; i != stride; ++i) {
    for (size_t c = 0; c != 3; ++c) {
      output_data[c * stride + i] = input[i * 3 + c];
    }
  }
  *output = output_data;
}

/**
 * convert input from CHW format to HWC format
 * \param input A single image. This float array has length of 3*h*w
 * \param h image height
 * \param w image width
 * \param output A byte array. should be freed by caller after use
 */
static void chw_to_hwc(const float* input, size_t h, size_t w, uint8_t** output) {
  size_t stride = h * w;
  uint8_t* output_data = (uint8_t*)malloc(stride * 3);
  for (size_t c = 0; c != 3; ++c) {
    size_t t = c * stride;
    for (size_t i = 0; i != stride; ++i) {
      float f = input[t + i];
      if (f < 0.f || f > 255.0f) f = 0;
      output_data[i * 3 + c] = (uint8_t)f;
    }
  }
  *output = output_data;
}

static int l_read_image_file(lua_State *L)
{
    const char *input_file = luaL_checkstring(L, 1);
    size_t input_data_length;
    float* out;
    size_t output_count;

    png_image image; /* The control structure used by libpng */
    /* Initialize the 'png_image' structure. */
    memset(&image, 0, (sizeof image));
    image.version = PNG_IMAGE_VERSION;
    if (png_image_begin_read_from_file(&image, input_file) == 0)
    {
        luaL_error(L, "Reading image file failed");
    }

    image.format = PNG_FORMAT_BGR;
    input_data_length = PNG_IMAGE_SIZE(image);

    char* buffer = calloc(input_data_length, sizeof(char));
    if (png_image_finish_read(&image, NULL /*background*/, buffer, 0 /*row_stride*/, NULL /*colormap*/) == 0)
    {
        luaL_error(L, "Finish reading image file failed");
    }
    hwc_to_chw((const uint8_t*)buffer, image.height, image.width, &out, &output_count);

    lua_pushlstring(L, (const char*)out, output_count * sizeof(float));
    lua_pushinteger(L, image.height);
    lua_pushinteger(L, image.width);
    free(buffer);
    free(out);
    return 3;
}

static int l_read_grayscale_image_file(lua_State *L)
{
    const char *input_file = luaL_checkstring(L, 1);
    size_t input_data_length;

    png_image image;
    memset(&image, 0, sizeof image);
    image.version = PNG_IMAGE_VERSION;
    if (png_image_begin_read_from_file(&image, input_file) == 0)
    {
        luaL_error(L, "Reading image file failed");
    }
    image.format = PNG_FORMAT_GRAY;
    input_data_length = PNG_IMAGE_SIZE(image);

    char* buffer = calloc(input_data_length, sizeof(char));
    float* out = calloc(input_data_length, sizeof(float));
    if (png_image_finish_read(&image, NULL /*background*/, buffer, 0 /*row_stride*/, NULL /*colormap*/) == 0)
    {
        luaL_error(L, "Finish reading image file failed");
    }
    for (size_t i = 0; i < input_data_length; i++) {
        out[i] = (float)buffer[i];
    }

    lua_pushlstring(L, (const char*)out, input_data_length * sizeof(float));
    lua_pushinteger(L, image.height);
    lua_pushinteger(L, image.width);
    free(buffer);
    free(out);
    return 3;
}

static int l_write_image_file(lua_State *L)
{
    const char *image_data = luaL_checkstring(L, 1);
    unsigned int height = (unsigned int)luaL_checkinteger(L, 2);
    unsigned int width = (unsigned int)luaL_checkinteger(L, 3);
    const char *output_file = luaL_checkstring(L, 4);

    uint8_t *image_data_hwc;
    chw_to_hwc((const float *)image_data, height, width, &image_data_hwc);

    png_image image;
    memset(&image, 0, (sizeof image));
    image.version = PNG_IMAGE_VERSION;
    image.format = PNG_FORMAT_BGR;
    image.height = height;
    image.width = width;
    if (png_image_write_to_file(&image, output_file, 0 /*convert_to_8bit*/, image_data_hwc, 0 /*row_stride*/,
                                NULL /*colormap*/) == 0)
    {
        luaL_error(L, "write to '%s' failed:%s\n", output_file, image.message);
    }

    free(image_data_hwc);
    return 0;
}

static const struct luaL_Reg luapngutils[] = {
    {"read", l_read_image_file},
    {"write", l_write_image_file},
    {"read_grayscale", l_read_grayscale_image_file},
    {NULL, NULL}};

#if defined(_WIN32) || defined(_WIN64)
__declspec(dllexport)
#endif
int
luaopen_luapng(lua_State *L)
{
    luaL_newlib(L, luapngutils);

    return 1;
}
