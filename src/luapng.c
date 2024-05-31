#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lua.h"
#include "lauxlib.h"
#include "png.h"

static int l_read_image_file(lua_State *L)
{
    const char *input_file = luaL_checkstring(L, 1);
    size_t input_data_length;

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

    size_t stride = input_data_length / 3;

    lua_createtable(L, input_data_length, 2);

    lua_pushstring(L, "height");
    lua_pushinteger(L, (lua_Integer)image.height);
    lua_rawset(L, -3);

    lua_pushstring(L, "width");
    lua_pushinteger(L, (lua_Integer)image.width);
    lua_rawset(L, -3);

    for (size_t i = 0; i != stride; ++i) {
        for (size_t c = 0; c != 3; ++c) {
            lua_pushinteger(L, (uint8_t)buffer[i * 3 + c]);
            lua_rawseti(L, -2, (lua_Integer)(c * stride + i + 1));
        }
    }

    free(buffer);
    return 1;
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
    if (png_image_finish_read(&image, NULL /*background*/, buffer, 0 /*row_stride*/, NULL /*colormap*/) == 0)
    {
        luaL_error(L, "Finish reading image file failed");
    }

    lua_createtable(L, input_data_length, 2);

    lua_pushstring(L, "height");
    lua_pushinteger(L, (lua_Integer)image.height);
    lua_rawset(L, -3);

    lua_pushstring(L, "width");
    lua_pushinteger(L, (lua_Integer)image.width);
    lua_rawset(L, -3);

    for (size_t i = 0; i < input_data_length; ++i) {
        lua_pushinteger(L, (uint8_t)buffer[i]);
        lua_rawseti(L, -2, (lua_Integer)(i + 1));
    }

    free(buffer);
    return 1;
}

static int l_write_image_file(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    const char *output_file = luaL_checkstring(L, 2);

    lua_getfield(L, 1, "height");
    if (!lua_isnumber(L, -1)) { luaL_error(L, "Image data height is not a number"); return 0; }
    size_t h = (size_t)lua_tointeger(L, -1);
    lua_getfield(L, 1, "width");
    if (!lua_isnumber(L, -1)) { luaL_error(L, "Image data width is not a number"); return 0; }
    size_t w = (size_t)lua_tointeger(L, -1);
    lua_pop(L, 2);

    size_t stride = h * w;
    uint8_t* output_data = (uint8_t*)malloc(stride * 3);
    for (size_t c = 0; c != 3; ++c) {
        size_t t = c * stride;
        for (size_t i = 0; i != stride; ++i) {
            lua_rawgeti(L, 1, (lua_Integer)(t + i + 1));
            if (!lua_isnumber(L, -1)) {
                luaL_error(L, "Bad element type");
                free(output_data);
                return 0;
            }
            lua_Number f = lua_tonumber(L, -1);
            lua_pop(L, 1);
            if (f < 0.f) f = 0;
            if (f > 255.0f) f = 255;
            output_data[i * 3 + c] = (uint8_t)f;
        }
    }

    png_image image;
    memset(&image, 0, (sizeof image));
    image.version = PNG_IMAGE_VERSION;
    image.format = PNG_FORMAT_BGR;
    image.height = h;
    image.width = w;
    if (png_image_write_to_file(&image, output_file, 0 /*convert_to_8bit*/, output_data, 0 /*row_stride*/,
                                NULL /*colormap*/) == 0)
    {
        luaL_error(L, "write to '%s' failed:%s\n", output_file, image.message);
    }

    free(output_data);
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
