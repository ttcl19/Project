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
    }
}
