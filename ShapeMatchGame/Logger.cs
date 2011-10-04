using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace BodyTetrisWrapper
{
    class Logger
    {
        string logFilename = "";
        StreamWriter logStream;
        DateTime logStartTime;

        public Logger(string path)
        {
            logStartTime = DateTime.Now;

            string dir = path.Substring(0, path.LastIndexOf("/"));

            if (!Directory.Exists(dir))
            {
                Directory.CreateDirectory(dir);
            }

            logFilename = path;
            logStream = new StreamWriter(logFilename, true);

            AddLogWithTime("LogStart");
        }

        public void AddLog(string log)
        {
            logStream.WriteLine(log);
        }

        public void AddLogWithTime(string log)
        {
            DateTime Now = DateTime.Now;
            string dateTimeString = Now.Month + "-" + Now.Day + "-" + Now.Hour + ":" + Now.Minute + ":" + Now.Second + "." + Now.Millisecond;

            double msDiff = (Now - logStartTime).TotalMilliseconds;

            AddLog(dateTimeString + " " + msDiff + " " + log);
        }

        public void AddLogWithTime(string log, int[] array)
        {
            log += " ";
            for (int i = 0; i < array.Length; i++)
            {
                log += array[i] + ",";
            }
            AddLogWithTime(log);
        }

        public void Close()
        {
            logStream.Close();
        }
    }
}
