using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace BodyTetrisWrapper
{
    class Program
    {
        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int IntFunc(int i);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr AllocFunc(int i);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int PtrPassFunc(IntPtr ptr);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int TweetFunc(int i1, int i2, int w, int h, IntPtr ptr);

        [DllImport("SkeletalViewer.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void openKinectWindow();

        [DllImport("SkeletalViewer.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int numericCommand(int cmd);

        [DllImport("SkeletalViewer.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int setTweetback(AllocFunc allocGlobal, TweetFunc tweetBack);

        private static IntPtr allocGlobal(int size)
        {
            IntPtr memPtr = Marshal.AllocHGlobal(size);
            //HACK allocatedGlobalPtrs.Add(memPtr);
            return memPtr;
        }

        static Twitpic twitter = new Twitpic("TweetrisTO","squiggle");

        static int numPhotos = 0;

        public static int TweetPicture(int shape, int orientation, int w, int h, IntPtr ptr)
        {
            try
            {
                //ptr is the pointer to the photo bits.

                byte[] pixels = new byte[w * h * 3];
                Marshal.Copy(ptr, pixels, 0, w * h * 3);

                BlackOut(pixels,shape,orientation,w,h);

                // Define the image palette
                BitmapPalette myPalette = BitmapPalettes.Halftone256;

                // Creates a new empty image with the pre-defined palette

                BitmapSource image = BitmapSource.Create(
                    w,
                    h,
                    96,
                    96,
                    PixelFormats.Rgb24,
                    myPalette,
                    pixels,
                    w*3);

                string TweetString = Tetronimos.GetString(shape, orientation);
                string TweetFileName = "" + TweetString + " " + numPhotos + ".png";

                FileStream stream = new FileStream(TweetFileName, FileMode.Create);
                PngBitmapEncoder encoder = new PngBitmapEncoder();

                encoder.Interlace = PngInterlaceOption.On;
                encoder.Frames.Add(BitmapFrame.Create(image));
                encoder.Save(stream);

                stream.Close();

                //TODO upload tweet photo.

                //HACK twitter.UploadPhoto(pixels, TweetString, TweetFileName);
            
                //TODO send and cut up mini-photos.
            
            
            }
            catch (Exception e)
            {

            }
            numPhotos++;
            return 0;
        }


        static void Main(string[] args)
        {
            AllocFunc allocGlobalFunc = new AllocFunc(Program.allocGlobal);
            TweetFunc tweetbackFunc = new TweetFunc(Program.TweetPicture);
            int ret = setTweetback(allocGlobalFunc,tweetbackFunc);

            Action KinectLoop = new Action(openKinectWindow);
            KinectLoop.BeginInvoke(null, null);

            


            //TODO provide callback for image writing.

            Console.WriteLine("Everything open; begin idlin'");

            bool run = true;
            while (run)
            {
                string input = Console.ReadLine();
                try
                {
                    int number = Convert.ToInt32(input);
                    numericCommand(number);
                }
                catch
                {
                    //string commands.
                }
                input = "";

                //yes, this is an infinite loop. Behold!
            }

        }

        static void BlackOut(byte[] pixels, int shape, int orientation, int w, int h)
        {
            //Blacks out the pixels that aren't part of the shape.

            for (int y = 0; y < h; y++)
            {
                for (int x = 0; x < w; x++)
                {
                    if (!Tetronimos.IsIn(shape, orientation, x, y, w, h))
                    {
                        //blacken pixel
                        int index = x + y*w;

                        pixels[3 * index] = 0;
                        pixels[3 * index + 1] = 0;
                        pixels[3 * index + 2] = 0;
                    }
                }
            }
        }
    }
}
