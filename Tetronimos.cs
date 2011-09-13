using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace BodyTetrisWrapper
{
    static class Tetronimos
    {
        public static string GetString(int shape, int orientation)
        {
            //I'm assuming for now that orientation is something basic
            // like 0,1,2,3

            string msg = "";
            switch(shape)
            {
                case 0:
                    msg += "drill";
                    msg += " " + orientation;
                    break;
                case 1:
                    msg += "knight";
                    msg += " " + orientation;
                    break;
                case 2:
                    msg += "anti-knight";
                    msg += " " + orientation;
                    break;
                case 3:
                    msg += "box";
                    break;
                case 4:
                    msg += "lightning";
                    msg += " " + orientation;
                    break;
                case 5:
                    msg += "hat";
                    msg += " " + orientation;
                    break;
                case 6:
                    msg += "anti-lightning";
                    msg += " " + orientation;
                    break;

            }


            return "shape";
        }

        public static bool IsIn(int shape, int orientation, int x, int y, int w, int h)
        {
            //Returns true if the pixel is "in" the tetronimo, and should be coloured in.

            double x_frac = x / (double)w;
            double y_frac = y / (double)h;

            if (x_frac < 0 || x_frac > 1 || y_frac < 0 || y_frac > 1)
                return false; //out of bounds of box.

            //see Shapes namespace (ShapeLibrary files in CPP project) for shape/orientation format.
            switch (shape)
            {
                case 1: //I
                    //only one orientation.
                    return true;

                    break;
                case 2: //J
                    switch (orientation)
                    {
                        case 0:
                            if (x_frac >= 0.5 || y_frac >= 2.0/3.0)
                                return true;
                            break;
                        case 1:
                            if (y_frac <= 0.5 || x_frac >= 2.0 / 3.0)
                                return true;
                            break;
                        case 2:
                            if (x_frac <= 0.5 || y_frac <= 1.0 / 3.0)
                                return true;
                            break;
                        case 3:
                            if (y_frac >= 0.5 || x_frac <= 1.0 / 3.0)
                                return true;
                            break;
                    }

                    break;
                case 3: //L
                    switch (orientation)
                    {
                        case 0:
                            if (x_frac <= 0.5 || y_frac >= 2.0/3.0)
                                return true;
                            break;
                        case 1:
                            if (y_frac >= 0.5 || x_frac >= 2.0 / 3.0)
                                return true;
                            break;
                        case 2:
                            if (x_frac >= 0.5 || y_frac <= 1.0 / 3.0)
                                return true;
                            break;
                        case 3:
                            if (y_frac <= 0.5 || x_frac <= 1.0 / 3.0)
                                return true;
                            break;
                    }

                    break;
                case 4: //O
                    //orientation doesn't make a diff.
                    return true;
                case 5: //Z
                    switch (orientation)
                    {
                        case 0:
                            if ((x_frac < 1.0 / 3.0 && y_frac > 0.5) ||
                                (x_frac > 2.0 / 3.0 && y_frac < 0.5))
                                return false;
                            return true;
                        case 1:
                            if ((x_frac < 0.5 && y_frac < 1.0 / 3.0) ||
                                (x_frac > 0.5 && y_frac > 2.0 / 3.0))
                                return false;
                            return true;
                    }
                    break;
                case 6: //T
                    switch (orientation)
                    {
                        case 0:
                            if (y_frac > 0.5 && (x_frac < 1.0 / 3.0 || x_frac > 2.0 / 3.0))
                                return false;
                            return true;
                        case 1:
                            if (x_frac > 0.5 && (y_frac < 1.0 / 3.0 || y_frac > 2.0 / 3.0))
                                return false;
                            return true;
                        case 2:
                            if (y_frac < 0.5 && (x_frac < 1.0 / 3.0 || x_frac > 2.0 / 3.0))
                                return false;
                            return true;
                        case 3:
                            if (x_frac < 0.5 && (y_frac < 1.0 / 3.0 || y_frac > 2.0 / 3.0))
                                return false;
                            return true;
                    }
                    break;
                case 7: //S
                    switch (orientation)
                    {
                        case 0:
                            if ((x_frac < 1.0 / 3.0 && y_frac < 0.5) ||
                                (x_frac > 2.0 / 3.0 && y_frac > 0.5))
                                return false;
                            return true;
                        case 1:
                            if ((x_frac > 0.5 && y_frac < 1.0 / 3.0) ||
                                (x_frac < 0.5 && y_frac > 2.0 / 3.0))
                                return false;
                            return true;
                    }
                    break;
            }

            //else
            return false;
        }
    }
}
