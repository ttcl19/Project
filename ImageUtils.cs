using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BodyTetrisWrapper
{
    static class ImageUtils
    {
        public static byte[] ImageSubset(byte[] pixels, int w, int h, int X1, int Y1, int X2, int Y2)
        {

            byte[] newPixels = new byte[3*(X2-X1)*(Y2-Y1)];

            int oldIndex = 0;
            int newIndex = 0;
            
            for (int y = Y1; y < Y2; y++)
            {
                for (int x = X1; x < X2 + 1; x++)
                {
                    
                    oldIndex = x + y * w;
                    newIndex = (x - X1) + (y - Y1) * (X2 - X1);
                    newPixels[newIndex] = pixels[oldIndex];
                    newPixels[newIndex + 1] = pixels[oldIndex + 1];
                    newPixels[newIndex + 2] = pixels[oldIndex + 2];
                    
                }
            }
            
            return newPixels;
        }

        public static byte[] RotateImage(byte[] pixels, int w, int h, int rotation)
        {
            //rotation is in multiples of 90 degrees CW.

            byte[] newPixels = new byte[3*w * h];

            switch (rotation)
            {
                case 0:
                    return pixels;
                case 2:
                    //simply reverse direction
                    for (int i = 0; i < w * h; i++)
                    {
                        newPixels[3 * i] = pixels[3 * (w * h - i)];
                        newPixels[3 * i + 1] = pixels[3 * (w * h - i) + 1];
                        newPixels[3 * i + 2] = pixels[3 * (w * h - i) + 2];
                    }
                    return newPixels;
                case 1:
                    for (int i = 0; i < w * h; i++)
                    {
                        int newIndex = (h - 1 - i / w) + h * (i % w);
                        newPixels[3 * newIndex] = pixels[3 * i];
                        newPixels[3 * newIndex + 1] = pixels[3 * i + 1];
                        newPixels[3 * newIndex + 2] = pixels[3 * i + 2];
                    }
                    return newPixels;
                case 3:
                    for (int i = 0; i < w * h; i++)
                    {
                        int newIndex = (w * (h - 1) + 1 + i/w) - h * (i % w);
                        newPixels[3 * newIndex] = pixels[3 * i];
                        newPixels[3 * newIndex + 1] = pixels[3 * i + 1];
                        newPixels[3 * newIndex + 2] = pixels[3 * i + 2];
                    }
                    return newPixels;
            }

            //it should never come to this.
            return newPixels;
        }

        public static void BlackOut(byte[] pixels, int shape, int orientation, int w, int h)
        {
            //Blacks out the pixels that aren't part of the shape.

            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w; x++)
                {
                    if (!Tetronimos.IsIn(shape, orientation, x, y, w, h))
                    {
                        //blacken pixel
                        int index = x + y * w;

                        pixels[3 * index] = 0;
                        pixels[3 * index + 1] = 0;
                        pixels[3 * index + 2] = 0;
                    }
                }
            }
        }
    }
}
