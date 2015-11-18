//
// Created by Patricio Villalobos on 10/5/15.
//

#include <stdio.h>
#include <string.h>
#include <iostream>
#include "tga.h"


typedef struct _TargaHeader {
    unsigned char id_length, colormap_type, image_type;
    unsigned short colormap_index, colormap_length;
    unsigned char colormap_size;
    unsigned short x_origin, y_origin, width, height;
    unsigned char pixel_size, attributes;
} TargaHeader;

void R_LoadTGA(const char *name, byte **pic, int *width, int *height) {
    unsigned columns, rows, numPixels;
    byte *pixbuf;
    int row, column;
    byte *buf_p;
    byte *end;
    union {
        byte *b;
        void *v;
    } buffer;
    TargaHeader targa_header;
    byte *targa_rgba;
    int length;

    *pic = nullptr;

    if (width)
        *width = 0;
    if (height)
        *height = 0;

    //
    // load the file
    //
    FILE *f = fopen((char *) name, "rb");
    if (f == nullptr) return;
    fseek(f, 0, SEEK_END);
    length = (int) ftell(f);

    fseek(f, 0, SEEK_SET);

    byte *bb = (byte *) malloc(length);

    std::cout << fread(bb, 1, length, f) << std::endl;


    if (!bb || length < 0) {
        return;
    }

    buf_p = bb;
    end = bb + length;

    targa_header.id_length = buf_p[0];
    targa_header.colormap_type = buf_p[1];
    targa_header.image_type = buf_p[2];

    memcpy(&targa_header.colormap_index, &buf_p[3], 2);
    memcpy(&targa_header.colormap_length, &buf_p[5], 2);
    targa_header.colormap_size = buf_p[7];
    memcpy(&targa_header.x_origin, &buf_p[8], 2);
    memcpy(&targa_header.y_origin, &buf_p[10], 2);
    memcpy(&targa_header.width, &buf_p[12], 2);
    memcpy(&targa_header.height, &buf_p[14], 2);
    targa_header.pixel_size = buf_p[16];
    targa_header.attributes = buf_p[17];

    buf_p += 18;

    if (targa_header.image_type != 2
        && targa_header.image_type != 10
        && targa_header.image_type != 3) {

        std::cout << "LoadTGA: Only type 2 (RGB), 3 (gray), and 10 (RGB) TGA images supported : " << name << std::endl;
    }

    if (targa_header.colormap_type != 0) {
        std::cout << "LoadTGA: colormaps not supported" << std::endl;
    }

    if ((targa_header.pixel_size != 32 && targa_header.pixel_size != 24) && targa_header.image_type != 3) {
        std::cout << "LoadTGA: Only 32 or 24 bit images supported (no colormaps)" << std::endl;
    }

    columns = targa_header.width;
    rows = targa_header.height;
    numPixels = columns * rows * 4;

    if (!columns || !rows || numPixels > 0x7FFFFFFF || numPixels / columns / 4 != rows) {
        std::cout << "LoadTGA: " << name << " has an invalid image size" << std::endl;
    }


    targa_rgba = (byte *) malloc(numPixels);

    if (targa_header.id_length != 0) {
        if (buf_p + targa_header.id_length > end)
            std::cout << "LoadTGA: header too short (%s)" << std::endl;

        buf_p += targa_header.id_length;  // skip TARGA image comment
    }

    if (targa_header.image_type == 2 || targa_header.image_type == 3) {
        if (buf_p + columns * rows * targa_header.pixel_size / 8 > end) {
            std::cout << "LoadTGA: file truncated (" << name << ")" << std::endl;
        }

        // Uncompressed RGB or gray scale image
        for (row = rows - 1; row >= 0; row--) {
            pixbuf = targa_rgba + row * columns * 4;
            for (column = 0; column < columns; column++) {
                unsigned char red, green, blue, alphabyte;
                switch (targa_header.pixel_size) {

                    case 8:
                        blue = *buf_p++;
                        green = blue;
                        red = blue;
                        *pixbuf++ = red;
                        *pixbuf++ = green;
                        *pixbuf++ = blue;
                        *pixbuf++ = 255;
                        break;

                    case 24:
                        blue = *buf_p++;
                        green = *buf_p++;
                        red = *buf_p++;
                        *pixbuf++ = red;
                        *pixbuf++ = green;
                        *pixbuf++ = blue;
                        *pixbuf++ = 255;
                        break;
                    case 32:
                        blue = *buf_p++;
                        green = *buf_p++;
                        red = *buf_p++;
                        alphabyte = *buf_p++;
                        *pixbuf++ = red;
                        *pixbuf++ = green;
                        *pixbuf++ = blue;
                        *pixbuf++ = alphabyte;
                        break;
                    default:
                        std::cout << "LoadTGA: illegal pixel_size '" << targa_header.pixel_size << "' in file " <<
                        name << std::endl;
                        break;
                }
            }
        }
    }
    else if (targa_header.image_type == 10) {   // Runlength encoded RGB images
        unsigned char red, green, blue, alphabyte, packetHeader, packetSize, j;

        for (row = rows - 1; row >= 0; row--) {
            pixbuf = targa_rgba + row * columns * 4;
            for (column = 0; column < columns;) {
                if (buf_p + 1 > end)
                    std::cout << "LoadTGA: file truncated (" << name << ")" << std::endl;
                packetHeader = *buf_p++;
                packetSize = 1 + (packetHeader & 0x7f);
                if (packetHeader & 0x80) {        // run-length packet
                    if (buf_p + targa_header.pixel_size / 8 > end)
                        std::cout << "LoadTGA: file truncated (" << name << ")" << std::endl;
                    switch (targa_header.pixel_size) {
                        case 24:
                            blue = *buf_p++;
                            green = *buf_p++;
                            red = *buf_p++;
                            alphabyte = 255;
                            break;
                        case 32:
                            blue = *buf_p++;
                            green = *buf_p++;
                            red = *buf_p++;
                            alphabyte = *buf_p++;
                            break;
                        default:
                            std::cout << "LoadTGA: illegal pixel_size " << targa_header.pixel_size <<
                            " " << name << std::endl;
                            break;
                    }

                    for (j = 0; j < packetSize; j++) {
                        *pixbuf++ = red;
                        *pixbuf++ = green;
                        *pixbuf++ = blue;
                        *pixbuf++ = alphabyte;
                        column++;
                        if (column == columns) { // run spans across rows
                            column = 0;
                            if (row > 0)
                                row--;
                            else
                                goto breakOut;
                            pixbuf = targa_rgba + row * columns * 4;
                        }
                    }
                }
                else {                            // non run-length packet

                    if (buf_p + targa_header.pixel_size / 8 * packetSize > end)
                        std::cout << "LoadTGA: file truncated " << name << std::endl;
                    for (j = 0; j < packetSize; j++) {
                        switch (targa_header.pixel_size) {
                            case 24:
                                blue = *buf_p++;
                                green = *buf_p++;
                                red = *buf_p++;
                                *pixbuf++ = red;
                                *pixbuf++ = green;
                                *pixbuf++ = blue;
                                *pixbuf++ = 255;
                                break;
                            case 32:
                                blue = *buf_p++;
                                green = *buf_p++;
                                red = *buf_p++;
                                alphabyte = *buf_p++;
                                *pixbuf++ = red;
                                *pixbuf++ = green;
                                *pixbuf++ = blue;
                                *pixbuf++ = alphabyte;
                                break;
                            default:
                                std::cout << "LoadTGA: illegal pixel_size " << targa_header.pixel_size
                                << name << std::endl;
                                break;
                        }
                        column++;
                        if (column == columns) { // pixel packet run spans across rows
                            column = 0;
                            if (row > 0)
                                row--;
                            else
                                goto breakOut;
                            pixbuf = targa_rgba + row * columns * 4;
                        }
                    }
                }
            }
            breakOut:;
        }
    }


    // instead we just print a warning
    if (targa_header.attributes & 0x20) {
        std::cout << "WARNING: '" << name << "' TGA file header declares top-down image, ignoring" << std::endl;
    }

    if (width)
        *width = columns;
    if (height)
        *height = rows;

    *pic = targa_rgba;

    free(bb);
}