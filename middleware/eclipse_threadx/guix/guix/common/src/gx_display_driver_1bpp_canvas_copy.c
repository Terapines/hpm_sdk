/***************************************************************************
 * Copyright (c) 2024 Microsoft Corporation 
 * 
 * This program and the accompanying materials are made available under the
 * terms of the MIT License which is available at
 * https://opensource.org/licenses/MIT.
 * 
 * SPDX-License-Identifier: MIT
 **************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */
/** GUIX Component                                                        */
/**                                                                       */
/**   Display Management (Display)                                        */
/**                                                                       */
/**************************************************************************/

#define GX_SOURCE_CODE


/* Include necessary system files.  */

#include "gx_api.h"
#include "gx_display.h"
#include "gx_utility.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _gx_display_driver_1bpp_canvas_copy                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Kenneth Maxwell, Microsoft Corporation                              */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Canvas copy function for the 1bpp display driver.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*   canvas                                 The canvas to copy from       */
/*   composite                              The canvas to copy to         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _gx_utility_rectangle_shift           Move the rectangle            */
/*    _gx_utility_rectangle_overlap_detect  Detect two rectangles being   */
/*                                            overlap to each other       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    GUIX Internal Code                                                  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Kenneth Maxwell          Initial Version 6.0           */
/*  09-30-2020     Kenneth Maxwell          Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID _gx_display_driver_1bpp_canvas_copy(GX_CANVAS *canvas, GX_CANVAS *composite)
{
GX_RECTANGLE dirty;
GX_RECTANGLE overlap;
GX_UBYTE    *read;
GX_UBYTE    *write;
INT          row;
INT          column;
UINT         read_pos;
UINT         write_pos;
GX_UBYTE     read_mask;
GX_UBYTE     write_mask;
INT          readstride;
INT          writestride;
INT          offset;

    dirty.gx_rectangle_left = dirty.gx_rectangle_top = 0;
    dirty.gx_rectangle_right = (GX_VALUE)(canvas -> gx_canvas_x_resolution - (GX_VALUE)1);
    dirty.gx_rectangle_bottom = (GX_VALUE)(canvas -> gx_canvas_y_resolution - (GX_VALUE)1);
    readstride = (canvas->gx_canvas_x_resolution + 7) >> 3;
    writestride = (composite->gx_canvas_x_resolution + 7) >> 3;

    _gx_utility_rectangle_shift(&dirty, canvas -> gx_canvas_display_offset_x, canvas -> gx_canvas_display_offset_y);

    if (_gx_utility_rectangle_overlap_detect(&dirty, &composite -> gx_canvas_dirty_area, &overlap))
    {
        offset = overlap.gx_rectangle_left - dirty.gx_rectangle_left;
        read_pos = (UINT)((overlap.gx_rectangle_top - dirty.gx_rectangle_top) * readstride + (offset >> 3));
        write_pos = (UINT)(overlap.gx_rectangle_top * writestride + (overlap.gx_rectangle_left >> 3));

        for (row = overlap.gx_rectangle_top; row <= overlap.gx_rectangle_bottom; row++)
        {
            read = (GX_UBYTE *)canvas -> gx_canvas_memory;
            write = (GX_UBYTE *)composite -> gx_canvas_memory;
            read += read_pos;
            write += write_pos;
            read_mask = (GX_UBYTE)(((GX_UBYTE)0x80) >> (offset & 0x07));
            write_mask = (GX_UBYTE)(0x80 >> (overlap.gx_rectangle_left & 0x07));

            for (column = overlap.gx_rectangle_left; column <= overlap.gx_rectangle_right; column++)
            {
                if (((*read) & read_mask) == read_mask)
                {
                    *write |= write_mask;
                }
                else
                {
                    *write = (GX_UBYTE)((*write) & (~write_mask));
                }
                read_mask >>= 1;
                write_mask >>= 1;
                if (!read_mask)
                {
                    read++;
                    read_mask = 0x80;
                }
                if (!write_mask)
                {
                    write++;
                    write_mask = 0x80;
                }
            }
            write_pos = (UINT)((INT)write_pos + writestride);
            read_pos = (UINT)((INT)read_pos + readstride);
        }
    }
}

