using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;
using System.Windows.Media;
using System.Windows.Media.Imaging;

//for osc
using Ventuz.OSC;

namespace BodyTetrisWrapper
{
    class Program
    {
        static int OSC_PORT = 7710; //overridden by OSC_PORT.txt
        static UdpWriter OSCSender; 

        //HACK TWEETING_ENABLED
        const bool TWEETING_ENABLED = false;

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int IntFunc(int i);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate IntPtr AllocFunc(int i);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int PtrPassFunc(IntPtr ptr);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate int TweetFunc(int i1, int i2, int w, int h, int squareSize, IntPtr ptr);

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
        static Twitpic twittershh = new Twitpic("TweetrisTOshh", "squiggle");

        static int numPhotos = 0;

        public static int SavePNG(byte[] pixels, int w, int h, string Filename)
        {
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
                w * 3);

            FileStream stream = new FileStream(Filename, FileMode.Create);
            PngBitmapEncoder encoder = new PngBitmapEncoder();

            encoder.Interlace = PngInterlaceOption.On;
            encoder.Frames.Add(BitmapFrame.Create(image));
            encoder.Save(stream);

            stream.Close();

            return 0;
        }

        public static int TweetPicture(int shape, int ori, int w, int h, int squareSize, IntPtr ptr)
        {
            try
            {
                //ptr is the pointer to the photo bits.
                byte[] pixels = new byte[w * h * 3];
                Marshal.Copy(ptr, pixels, 0, w * h * 3);

                ImageUtils.BlackOut(pixels,shape,ori,w,h);

                int orientation = Tetronimos.OrientationConversion(shape, ori);

                string TweetString = Tetronimos.GetString(shape, orientation);
                string TweetFileName = "" + TweetString + " " + numPhotos + ".png";

                SavePNG(pixels, w, h, TweetFileName);

                if (TWEETING_ENABLED)
                    twitter.UploadPhoto(pixels, TweetString, TweetFileName);

                string TweetShhString = "BLOCK," + numPhotos + "," + Tetronimos.GetShhName(shape) + "," + ori + ",";

                byte[][] blocks = new byte[4][];

                switch (shape)
                {
                    case 1: //LINE
                        blocks[0] = ImageUtils.ImageSubset(pixels, w, h, 0, 0, w, h * 1 / 4);
                        blocks[1] = ImageUtils.ImageSubset(pixels, w, h, 0, h * 1 / 4, w, h * 2 / 4);
                        blocks[2] = ImageUtils.ImageSubset(pixels, w, h, 0, h * 2 / 4, w, h * 3 / 4);
                        blocks[3] = ImageUtils.ImageSubset(pixels, w, h, 0, h * 3 / 4, w, h);

                        for (int i = 0; i < 4; i++)
                        {
                            SavePNG(blocks[i], w, w, TweetShhString + (i+1) + ".png");
                            if (TWEETING_ENABLED)
                            {
                                twittershh.UploadPhoto(blocks[i], TweetShhString + (i + 1), TweetShhString + (i + 1) + ".png");
                            }
                        }
                        break;
                    case 2: //LEL
                        switch (orientation)
                        {
                            case 0:
                                blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 0);
                                blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 1);
                                blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 1);
                                blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 2, 1);
                                break;
                            case 90:
                                blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 2);
                                blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 2);
                                blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 1);
                                blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 0);
                                break;
                            case 180:
                                blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 2, 1);
                                blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 2, 0);
                                blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 0);
                                blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 0);
                                break;
                            case 270:
                                blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 0);
                                blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 0);
                                blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 1);
                                blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 2);
                                break;
                        }

                        for (int i = 0; i < 4; i++)
                        {
                            blocks[i] = ImageUtils.RotateImage90s(blocks[i], squareSize, squareSize, orientation);
                        }
                        //ROW,COL
                        SavePNG(blocks[0], squareSize, squareSize, TweetShhString + "0,0" + ".png");
                        SavePNG(blocks[1], squareSize, squareSize, TweetShhString + "0,1" + ".png");
                        SavePNG(blocks[2], squareSize, squareSize, TweetShhString + "1,1" + ".png");
                        SavePNG(blocks[3], squareSize, squareSize, TweetShhString + "2,2" + ".png");

                        break;
                    case 3: //REL
                        switch (orientation)
                        {
                            case 0:
                                blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 0);
                                blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 1);
                                blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 0);
                                blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 2, 0);
                                break;
                            case 90:
                                blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 2);
                                blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 2);
                                blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 1);
                                blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 0);
                                break;
                            case 180:
                                blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 2, 1);
                                blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 2, 0);
                                blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 1);
                                blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 1);
                                break;
                            case 270:
                                blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 0);
                                blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 0);
                                blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 1);
                                blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 2);
                                break;
                        }

                        for (int i = 0; i < 4; i++)
                        {
                            blocks[i] = ImageUtils.RotateImage90s(blocks[i], squareSize, squareSize, orientation);
                        }
                        //ROW,COL
                        SavePNG(blocks[0], squareSize, squareSize, TweetShhString + "0,0" + ".png");
                        SavePNG(blocks[1], squareSize, squareSize, TweetShhString + "0,1" + ".png");
                        SavePNG(blocks[2], squareSize, squareSize, TweetShhString + "1,0" + ".png");
                        SavePNG(blocks[3], squareSize, squareSize, TweetShhString + "2,0" + ".png");
                        
                        break;
                    case 4: //SQUARE
                        blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 0);
                        blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 1);
                        blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 0);
                        blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 1);

                        SavePNG(blocks[0], squareSize, squareSize, TweetShhString + "0,0" + ".png");
                        SavePNG(blocks[1], squareSize, squareSize, TweetShhString + "0,1" + ".png");
                        SavePNG(blocks[2], squareSize, squareSize, TweetShhString + "1,0" + ".png");
                        SavePNG(blocks[3], squareSize, squareSize, TweetShhString + "1,1" + ".png");
                        break;

                    case 5: // Z/RESS
                        switch (orientation)
                        {
                            case 0:
                                blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 0);
                                blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 1);
                                blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 1);
                                blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 2, 1);
                                break;
                            case 180:
                                blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 0);
                                blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 1);
                                blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 1);
                                blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 2);
                                break;
                        }
                        for (int i = 0; i < 4; i++)
                        {
                            blocks[i] = ImageUtils.RotateImage90s(blocks[i], squareSize, squareSize, orientation);
                        }
                        //ROW,COL
                        SavePNG(blocks[0], squareSize, squareSize, TweetShhString + "0,0" + ".png");
                        SavePNG(blocks[1], squareSize, squareSize, TweetShhString + "0,1" + ".png");
                        SavePNG(blocks[2], squareSize, squareSize, TweetShhString + "1,1" + ".png");
                        SavePNG(blocks[3], squareSize, squareSize, TweetShhString + "2,1" + ".png");

                        break;

                    case 6: //TEE
                        switch (orientation)
                        {
                            case 0:
                                blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 0);
                                blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 0);
                                blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 1);
                                blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 2, 0);
                                break;
                            case 90:
                                blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 2);
                                blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 1);
                                blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 1);
                                blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 0);
                                break;
                            case 180:
                                blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 2, 1);
                                blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 1);
                                blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 0);
                                blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 1);
                                break;
                            case 270:
                                blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 2);
                                blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 1);
                                blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 1);
                                blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 0);
                                break;
                        }

                        for (int i = 0; i < 4; i++)
                        {
                            blocks[i] = ImageUtils.RotateImage90s(blocks[i], squareSize, squareSize, orientation);
                        }
                        //ROW,COL
                        SavePNG(blocks[0], squareSize, squareSize, TweetShhString + "0,0" + ".png");
                        SavePNG(blocks[1], squareSize, squareSize, TweetShhString + "1,1" + ".png");
                        SavePNG(blocks[2], squareSize, squareSize, TweetShhString + "1,1" + ".png");
                        SavePNG(blocks[3], squareSize, squareSize, TweetShhString + "2,0" + ".png");
                        break;
                    case 7: // S/LESS
                        switch (orientation)
                        {
                            case 0:
                                blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 1);
                                blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 1);
                                blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 0);
                                blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 2, 0);
                                break;
                            case 180:
                                blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 2, 1);
                                blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 1);
                                blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 1);
                                blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 0);
                                break;
                        }
                        for (int i = 0; i < 4; i++)
                        {
                            blocks[i] = ImageUtils.RotateImage90s(blocks[i], squareSize, squareSize, orientation);
                        }
                        //ROW,COL
                        SavePNG(blocks[0], squareSize, squareSize, TweetShhString + "0,1" + ".png");
                        SavePNG(blocks[1], squareSize, squareSize, TweetShhString + "1,1" + ".png");
                        SavePNG(blocks[2], squareSize, squareSize, TweetShhString + "1,0" + ".png");
                        SavePNG(blocks[3], squareSize, squareSize, TweetShhString + "2,0" + ".png");
                        break;
                }
                
            
            }
            catch (Exception e)
            {
                Console.WriteLine(e);
            }
            numPhotos++;
            return 0;
        }

        static void SetUpOSCPort()
        {
            //get OSC PORT
            string OSC_PORT_FILE = "OSC_PORT.txt";
            try
            {
                TextReader tr = new StreamReader(OSC_PORT_FILE);
                OSC_PORT = Convert.ToInt32(tr.ReadLine());
            }
            catch
            {
                //do nothing.
            }
            
            OSCSender = new UdpWriter("127.0.0.1", OSC_PORT);
        }

        static void Main(string[] args)
        {
            SetUpOSCPort();

            AllocFunc allocGlobalFunc = new AllocFunc(Program.allocGlobal);
            TweetFunc tweetbackFunc = new TweetFunc(Program.TweetPicture);
            int ret = setTweetback(allocGlobalFunc,tweetbackFunc);

            Action KinectLoop = new Action(openKinectWindow);
            KinectLoop.BeginInvoke(null, null);

            bool run = true;

            Console.WriteLine("Type -1 and press ENTER to start game.");
            
            while (run)
            {
                string input = Console.ReadLine();
                try
                {
                    int number = Convert.ToInt32(input);
                    numericCommand(number);

                    //HACK OSC
                    OscBundle bundle = new OscBundle();
                    OscElement message = new OscElement("/I/am/sending/you/five", 5);

                    bundle.AddElement(message);
                    
                    
                    //send bundle
                    OSCSender.Send(bundle);


                }
                catch
                {
                    //string commands.
                }
                input = "";

                //yes, this is an infinite loop. Behold!
            }

        }

        
    }
}
