package http;

import java.util.Map;
import java.util.HashMap;

public class httpObject {
	public httpParser.MessageType type;
	public String httpVersion;
	
	// For response type
	public int statusCode;
	public String statusDesc;

	// For request type
	public String method;
	public String requestPath;
	public Map<String, String> parameters = new HashMap<String, String>();

	// The rest of member
	public Map<String, String> headers = new HashMap<String, String>();
	public byte[] body = null;

	public void addHeader(String key, String value) {
		this.headers.put(key, value);
	}
}
