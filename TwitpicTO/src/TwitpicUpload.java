import java.io.File;
import java.io.IOException;

import com.harrison.lee.twitpic4j.TwitPic;
import com.harrison.lee.twitpic4j.TwitPicResponse;
import com.harrison.lee.twitpic4j.exception.TwitPicException;

public class TwitpicUpload {
    public TwitpicUpload() {   
       
    }
   
    public boolean upload(String filename, String message, String login, String password) {
        File picture = new File(filename);

        // Create TwitPic object and allocate TwitPicResponse object
        TwitPic tpRequest = new TwitPic(login, password);
        TwitPicResponse tpResponse = null;

        // Make request and handle exceptions                          
        try {
             tpResponse = tpRequest.uploadAndPost(picture, message);
        } catch (IOException e) {
                e.printStackTrace();
        } catch (TwitPicException e) {
                e.printStackTrace();
        }

        if (tpResponse != null) {
        	//tpResponse.dumpVars();
        	return tpResponse.getMediaAid() != null;
        }
        
        return false;
    }
}