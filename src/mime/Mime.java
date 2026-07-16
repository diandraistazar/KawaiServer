package mime;

import java.util.Map;
import java.util.HashMap;

public class Mime {
	public static Map<String, String> mimeTypes = new HashMap<String, String>();

	static {
		// Video
		mimeTypes.put("mp4", "video/mp4");

		// Audio
		mimeTypes.put("mp3", "audio/mpeg");

		// Image
		mimeTypes.put("png", "image/png");
		mimeTypes.put("jpg", "image/jpg");

		// Text
		mimeTypes.put("txt", "text/plain");
		mimeTypes.put("html", "text/html");
		mimeTypes.put("css", "text/css");
		mimeTypes.put("php", "text/php");
	}

	public static String getFileExt(String filePath) {
		int dot = filePath.lastIndexOf('.');
		if(dot < 0) {
			return null;
		}

		return filePath.substring(dot + 1, filePath.length());
	}

	public static String getMimeType(String filePath) {
		String fileExt = Mime.getFileExt(filePath);
		String mimeType = null;

		if(fileExt != null && (mimeType = Mime.mimeTypes.get(fileExt)) != null) {
			return mimeType;
		}

		return null;
	}
}
