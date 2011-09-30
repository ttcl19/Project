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
        public delegate void VoidFunc();

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void IntFunc(int i);

        [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
        public delegate void TwoIntFunc(int i, int j);

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

        [DllImport("SkeletalViewer.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int setOSCEvents(VoidFunc RoundStart, IntFunc Countdown, IntFunc Holding, IntFunc HoldFail,
            VoidFunc Timeout, IntFunc ShapeCompleted, TwoIntFunc ShapeStatus);

        private static IntPtr allocGlobal(int size)
        {
            IntPtr memPtr = Marshal.AllocHGlobal(size);
            return memPtr;
        }

        static Twitpic twitter = new Twitpic("TweetrisTO","squiggle");
        static Twitpic twittershh = new Twitpic("TweetrisTOshh", "squiggle");

        static int numPhotos = 0;

        public static void makeFullImage(byte[] pixels, int w, int h, string Filename)
        {
            //puts file in /images/ directory
            //puts empty file in /images_names/ directory.
            SavePNG(pixels, w, h, "full_images/"+Filename);
            FileStream stream = new FileStream("full_images_names/" + Filename, FileMode.Create);
            stream.Close();
        }

        public static void makeBlockImage(byte[] pixels, int w, int h, string Filename)
        {
            SavePNG(pixels, w, h, "block_images/" + Filename);
            FileStream stream = new FileStream("block_images_names/" + Filename, FileMode.Create);
            stream.Close();
        }

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

                makeFullImage(pixels, w, h, TweetFileName); //mainstream pictures

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
                            makeBlockImage(blocks[i], w, w, TweetShhString + "0,"+ i + ".png");
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
                        makeBlockImage(blocks[0], squareSize, squareSize, TweetShhString + "0,0" + ".png");
                        makeBlockImage(blocks[1], squareSize, squareSize, TweetShhString + "0,1" + ".png");
                        makeBlockImage(blocks[2], squareSize, squareSize, TweetShhString + "1,1" + ".png");
                        makeBlockImage(blocks[3], squareSize, squareSize, TweetShhString + "2,2" + ".png");

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
                        makeBlockImage(blocks[0], squareSize, squareSize, TweetShhString + "0,0" + ".png");
                        makeBlockImage(blocks[1], squareSize, squareSize, TweetShhString + "0,1" + ".png");
                        makeBlockImage(blocks[2], squareSize, squareSize, TweetShhString + "1,0" + ".png");
                        makeBlockImage(blocks[3], squareSize, squareSize, TweetShhString + "2,0" + ".png");
                        
                        break;
                    case 4: //SQUARE
                        blocks[0] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 0);
                        blocks[1] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 0, 1);
                        blocks[2] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 0);
                        blocks[3] = ImageUtils.ImageGetSquare(pixels, w, h, squareSize, 1, 1);

                        makeBlockImage(blocks[0], squareSize, squareSize, TweetShhString + "0,0" + ".png");
                        makeBlockImage(blocks[1], squareSize, squareSize, TweetShhString + "0,1" + ".png");
                        makeBlockImage(blocks[2], squareSize, squareSize, TweetShhString + "1,0" + ".png");
                        makeBlockImage(blocks[3], squareSize, squareSize, TweetShhString + "1,1" + ".png");
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
                        makeBlockImage(blocks[0], squareSize, squareSize, TweetShhString + "0,0" + ".png");
                        makeBlockImage(blocks[1], squareSize, squareSize, TweetShhString + "0,1" + ".png");
                        makeBlockImage(blocks[2], squareSize, squareSize, TweetShhString + "1,1" + ".png");
                        makeBlockImage(blocks[3], squareSize, squareSize, TweetShhString + "2,1" + ".png");

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
                        makeBlockImage(blocks[0], squareSize, squareSize, TweetShhString + "0,0" + ".png");
                        makeBlockImage(blocks[1], squareSize, squareSize, TweetShhString + "1,1" + ".png");
                        makeBlockImage(blocks[2], squareSize, squareSize, TweetShhString + "1,1" + ".png");
                        makeBlockImage(blocks[3], squareSize, squareSize, TweetShhString + "2,0" + ".png");
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
                        makeBlockImage(blocks[0], squareSize, squareSize, TweetShhString + "0,1" + ".png");
                        makeBlockImage(blocks[1], squareSize, squareSize, TweetShhString + "1,1" + ".png");
                        makeBlockImage(blocks[2], squareSize, squareSize, TweetShhString + "1,0" + ".png");
                        makeBlockImage(blocks[3], squareSize, squareSize, TweetShhString + "2,0" + ".png");
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

        static List<Delegate> DontThrowOutMaDelegates = new List<Delegate>();

        static void Main(string[] args)
        {
            SetUpOSCPort();
            
            AllocFunc allocGlobalFunc = new AllocFunc(Program.allocGlobal);
            TweetFunc TweetPictureDel = new TweetFunc(Program.TweetPicture); DontThrowOutMaDelegates.Add(TweetPictureDel);
            int ret = setTweetback(allocGlobalFunc, TweetPictureDel);
            VoidFunc RoundStartDel = new VoidFunc(Program.RoundStart); DontThrowOutMaDelegates.Add(RoundStartDel);
            IntFunc CountdownDel = new IntFunc(Program.Countdown); DontThrowOutMaDelegates.Add(CountdownDel);
            IntFunc HoldingDel = new IntFunc(Program.Holding); DontThrowOutMaDelegates.Add(HoldingDel);
            IntFunc HoldFailDel = new IntFunc(Program.HoldingFail); DontThrowOutMaDelegates.Add(HoldFailDel);
            VoidFunc TimeoutDel = new VoidFunc(Program.Timeout); DontThrowOutMaDelegates.Add(TimeoutDel);
            IntFunc ShapeCompletedDel = new IntFunc(Program.ShapeCompleted); DontThrowOutMaDelegates.Add(ShapeCompletedDel);
            TwoIntFunc ShapeStatusDel = new TwoIntFunc(Program.ShapeStatus); DontThrowOutMaDelegates.Add(ShapeStatusDel);
            ret = setOSCEvents(RoundStartDel, CountdownDel, HoldingDel, HoldFailDel, TimeoutDel, ShapeCompletedDel, ShapeStatusDel);

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
                }
                catch
                {
                    //string commands.
                }
                input = "";

                //yes, this is an infinite loop. Behold!
            }

        }

        //OSC Events
        public static void GameStart()
        {
            OscBundle bundle = new OscBundle();
            bundle.AddElement(new OscElement("/gamestart"));
            //send bundle
            OSCSender.Send(bundle);
        }
        public static void RoundStart()
        {
            Console.WriteLine("Sending Round Start");
            OscBundle bundle = new OscBundle();
            bundle.AddElement(new OscElement("/roundstart"));
            //send bundle
            OSCSender.Send(bundle);
        }
        public static void PersonMissing()
        {
            //TODO PersonMissing
        }
        public static void Countdown(int timeRemaining)
        {
            OscBundle bundle = new OscBundle();
            bundle.AddElement(new OscElement("/countdown", timeRemaining));
            //send bundle
            OSCSender.Send(bundle);
        }

        public static void Holding(int player)
        {
            OscBundle bundle = new OscBundle();
            bundle.AddElement(new OscElement("/holding", player));
            //send bundle
            OSCSender.Send(bundle);
        }

        public static void HoldingFail(int player)
        {
            OscBundle bundle = new OscBundle();
            bundle.AddElement(new OscElement("/HoldFail", player));
            //send bundle
            OSCSender.Send(bundle);
        }

        public static void Timeout()
        {
            OscBundle bundle = new OscBundle();
            bundle.AddElement(new OscElement("/timeout"));
            //send bundle
            OSCSender.Send(bundle);
        }
        public static void ShapeCompleted(int winner)
        {
            OscBundle bundle = new OscBundle();
            bundle.AddElement(new OscElement("/pwins", winner));
            //send bundle
            OSCSender.Send(bundle);
        }
        public static void ShapeStatus(int shape1, int shape2)
        {
            //Console.WriteLine("Got Shape Status");


            OscBundle bundle = new OscBundle();
            bundle.AddElement(new OscElement("/shape1/" + shape1));
            bundle.AddElement(new OscElement("/shape2/" + shape2));
            //send bundle
            OSCSender.Send(bundle);
        }
    }
}
