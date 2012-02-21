using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net;
using System.IO;
using System.Xml.Linq;

namespace BodyTetrisWrapper
{
    class Twitpic
    {
        string Username = "BodyTetris";
        string Password = "squiggle";

        /// <summary>
        /// URL for the TwitPic API's upload method
        /// </summary>
        private const string TWITPIC_UPLADO_API_URL = "http://twitpic.com/api/upload";

        /// <summary>
        /// URL for the TwitPic API's upload and post method
        /// </summary>
        private const string TWITPIC_UPLOAD_AND_POST_API_URL = "http://twitpic.com/api/uploadAndPost";

        internal const string ImageBmpContentType = "image/bmp";
        private const string ImageGifContentType = "image/gif";
        private const string ImageJpegContentType = "image/jpeg";
        private const string ImageTiffContentType = "image/tiff";
        private const string ImagePngContentType = "image/png";

        private const string ImageBmpFileExtension = ".bmp";
        private const string ImageGifFileExtension = ".gif";
        private const string ImageJpegFileExtension = ".jpeg";
        private const string ImageJpgFileExtension = ".jpg";
        private const string ImageTiffFileExtension = ".tiff";
        private const string ImagePngFileExtension = ".png";

        public Twitpic(string Username, string Password) {
            this.Username = Username;
            this.Password = Password;
        }

        private static string GetImageContentType(string imageUriString)
        {
            string imageContentType;
            if (imageUriString.EndsWith(ImageBmpFileExtension, StringComparison.OrdinalIgnoreCase))
            {
                imageContentType = ImageBmpContentType;
            }
            else if (imageUriString.EndsWith(ImageGifFileExtension, StringComparison.OrdinalIgnoreCase))
            {
                imageContentType = ImageGifContentType;
            }
            else if (imageUriString.EndsWith(ImageJpegFileExtension, StringComparison.OrdinalIgnoreCase) || imageUriString.EndsWith(ImageJpgFileExtension, StringComparison.OrdinalIgnoreCase))
            {
                imageContentType = ImageJpegContentType;
            }
            else if (imageUriString.EndsWith(ImageTiffFileExtension, StringComparison.OrdinalIgnoreCase))
            {
                imageContentType = ImageTiffContentType;
            }
            else
            {
                imageContentType = ImagePngContentType;
            }
            return imageContentType;
        }

        /// <summary>
        /// Uploads the photo and sends a new Tweet
        /// </summary>
        /// <param name="binaryImageData">The binary image data.</param>
        /// <param name="tweetMessage">The tweet message.</param>
        /// <param name="filename">The filename.</param>
        /// <returns>Return true, if the operation was succeded.</returns>
        public bool UploadPhoto(byte[] binaryImageData, string tweetMessage, string filename)
        {
            //TODO: currently uploads as video (mp4)...should upload as image.

            // Documentation: http://www.twitpic.com/api.do
            string boundary = Guid.NewGuid().ToString();
            string requestUrl = String.IsNullOrEmpty(tweetMessage) ? TWITPIC_UPLADO_API_URL : TWITPIC_UPLOAD_AND_POST_API_URL;
            HttpWebRequest request = (HttpWebRequest)WebRequest.Create(requestUrl);
            string encoding = "iso-8859-1";

            request.PreAuthenticate = true;
            request.AllowWriteStreamBuffering = true;
            request.ContentType = string.Format("multipart/form-data; boundary={0}", boundary);
            request.Method = "POST";

            string header = string.Format("--{0}", boundary);
            string footer = string.Format("--{0}--", boundary);

            StringBuilder contents = new StringBuilder();
            contents.AppendLine(header);

            string fileContentType = GetImageContentType(filename);
            string fileHeader = String.Format("Content-Disposition: file; name=\"{0}\"; filename=\"{1}\"", "media", filename);
            string fileData = Encoding.GetEncoding(encoding).GetString(binaryImageData);

            contents.AppendLine(fileHeader);
            contents.AppendLine(String.Format("Content-Type: {0}", fileContentType));
            contents.AppendLine();
            contents.AppendLine(fileData);

            contents.AppendLine(header);
            contents.AppendLine(String.Format("Content-Disposition: form-data; name=\"{0}\"", "username"));
            contents.AppendLine();
            contents.AppendLine(this.Username);

            contents.AppendLine(header);
            contents.AppendLine(String.Format("Content-Disposition: form-data; name=\"{0}\"", "password"));
            contents.AppendLine();
            contents.AppendLine(this.Password);

            if (!String.IsNullOrEmpty(tweetMessage))
            {
                contents.AppendLine(header);
                contents.AppendLine(String.Format("Content-Disposition: form-data; name=\"{0}\"", "message"));
                contents.AppendLine();
                contents.AppendLine(tweetMessage);
            }

            contents.AppendLine(footer);

            byte[] bytes = Encoding.GetEncoding(encoding).GetBytes(contents.ToString());
            request.ContentLength = bytes.Length;

            using (Stream requestStream = request.GetRequestStream())
            {
                requestStream.Write(bytes, 0, bytes.Length);

                using (HttpWebResponse response = (HttpWebResponse)request.GetResponse())
                {
                    using (StreamReader reader = new StreamReader(response.GetResponseStream()))
                    {
                        string result = reader.ReadToEnd();

                        XDocument doc = XDocument.Parse(result);

                        XElement rsp = doc.Element("rsp");
                        string status = rsp.Attribute(XName.Get("status")) != null ? rsp.Attribute(XName.Get("status")).Value : rsp.Attribute(XName.Get("stat")).Value;

                        return status.ToUpperInvariant().Equals("OK");
                    }
                }
            }
        }
    }
}
