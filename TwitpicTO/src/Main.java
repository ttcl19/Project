import java.io.File;
import java.io.IOException;


public class Main {
    public static void main(String args[]) throws IOException {
        TwitpicUpload uploader = new TwitpicUpload();
       
        File full_dir = new File(LoginInfo.FULL_NAMES_DIR);
        File block_dir = new File(LoginInfo.BLOCK_NAMES_DIR);
       
       // Constantly browse the image directory and upload if there are new pictures.
        while (true){
        	//System.out.println("Looking for files...");
        	//UPLOAD BLOCK IMAGES
        	String[] block_list = block_dir.list();
            for (int i = 0 ; i < block_list.length ; i++) {
            	String filename = LoginInfo.BLOCK_NAMES_DIR + "/" + block_list[i];
            	String imagename = LoginInfo.BLOCK_IMAGES_DIR + "/" + block_list[i];
            	
            	System.out.println("Processing: " + filename);
            	//changes
            	
            	//1 - Upload
                boolean uploaded = uploader.upload(imagename, block_list[i].substring(0, block_list[i].length()-4), LoginInfo.BLOCK_LOGIN, LoginInfo.BLOCK_PASSWORD);
                if (!uploaded) {
                	System.out.println("Error while uploading " + block_list[i]); 
                }
                
                else {
                	// 2 - Remove name
                	File file = new File(filename);
                	file.delete();
                }
            }
        	
        	// Upload FULL IMAGES
        	
        	String[] list = full_dir.list();
            
            for (int i = 0 ; i < list.length ; i++) {
            	String filename = LoginInfo.FULL_NAMES_DIR + "/" + list[i];
            	String imagename = LoginInfo.FULL_IMAGES_DIR + "/" + list[i];
            	
            	System.out.println("Processing: " + filename);
            	
            	
            	//1 - Upload
                boolean uploaded = uploader.upload(imagename, "ouech!", LoginInfo.FULL_LOGIN, LoginInfo.FULL_PASSWORD);
                if (!uploaded) {
                	System.out.println("Error while uploading " + list[i]); 
                }
                
                else {
                	// 2 - Remove name
                	File file = new File(filename);
                	file.delete();
                }
            }
            
            
            //delay, to prevent cpu hugging.
            try {
            	Thread.sleep(100L); // in ms
            }
           
            catch (Exception e) {}
            
        }
    }
}


