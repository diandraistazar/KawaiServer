package http;

import java.util.Arrays;
import java.util.Map;

public class httpParser {
	public static enum MessageType {
		HTTP_RESPONSE,
		HTTP_REQUEST
	}

	public static httpObject parse(byte[] data) {
		String[] temp = new String[3]; // store either first line of HTTP request or HTTP response (e.g method, path, http version)
		String dataString = new String(data);
		httpObject httpData = new httpObject();
		int startIndex = 0, endIndex = 0;
		char[] chara = { ' ', ' ', '\n' };
	
		for(int i = 0; i < 3; i++) {
			endIndex = dataString.indexOf(chara[i], startIndex);
			if(endIndex < 0) {
				return null;
			} else {
				temp[i] = dataString.substring(startIndex, endIndex);
				startIndex = endIndex + 1;
			}
		}

		if(temp[0].contains("HTTP/")) { // Response header handling
			httpData.type = httpParser.MessageType.HTTP_RESPONSE;
			httpData.httpVersion = temp[0];
			httpData.statusCode = Integer.parseInt(temp[1]);
			httpData.statusDesc = temp[2];

			if(startIndex >= dataString.length()) {
				return httpData;
			}

			if((endIndex = dataString.indexOf("\r\n\r\n", startIndex)) > 0) {
				endIndex += 4;
			}
			else if((endIndex = dataString.indexOf("\n\n", startIndex)) > 0) {
				endIndex += 2;
			}
			else {
				return httpData;
			}
			
			httpParser.parseHeaders(dataString.substring(startIndex, endIndex), httpData);
			httpData.body = Arrays.copyOfRange(data, endIndex, data.length);
		}
		else if(temp[2].contains("HTTP/")) { // Request header handling
			httpData.type = httpParser.MessageType.HTTP_REQUEST;
			httpData.method = temp[0];
			httpData.requestPath = httpParser.parseSpecialChara(temp[1]);
			httpData.httpVersion = temp[2];
			
			httpParser.parseParameters(httpData);

			if(startIndex >= dataString.length()) {
				return httpData;
			}

			if((endIndex = dataString.indexOf("\r\n\r\n", startIndex)) > 0) {
				endIndex += 4;
			}
			else if((endIndex = dataString.indexOf("\n\n", startIndex)) > 0) {
				endIndex += 2;
			}
			else {
				return httpData;
			}

			httpParser.parseHeaders(dataString.substring(startIndex, endIndex), httpData);
			httpData.body = Arrays.copyOfRange(data, endIndex, data.length);
		} 
		else {
			return null;
		}

		return httpData;
	}

	public static String create(httpObject httpData) {
		String http = "";

		if(httpData.type == httpParser.MessageType.HTTP_RESPONSE) {
			http += httpData.httpVersion;
			http += " ";
			http += httpData.statusCode;
			http += " ";
			http += httpData.statusDesc;
			http += "\r\n";
		}
		else if(httpData.type == httpParser.MessageType.HTTP_REQUEST) {
			http += httpData.method;
			http += " ";
			http += httpData.requestPath;
			http += " ";
			http += httpData.httpVersion;
			http += "\r\n";
		}
		else {
			return null;
		}

		for(Map.Entry<String, String> header : httpData.headers.entrySet()) {
			http += header.getKey();
			http += ": ";
			http += header.getValue();
			http += "\r\n";
		}
		
		http += "\r\n";
		if(httpData.body != null && httpData.body.length > 0) {
			http += new String(httpData.body);
		}
		return http;
	}

	private static String parseSpecialChara(String string) {
		String[] charas = {
			"%20", " "
		};

		for(int i = 0; i < charas.length; i+=2) {
			if(string.contains(charas[i])) {
				string = string.replace(charas[i], charas[i+1]);
			}
		}

		return string;
	}

	private static void parseParameters(httpObject httpData) {
		String stringPath = httpData.requestPath;
		int seperator = 0, start = 0, end = 0;

		seperator = stringPath.indexOf('?');
		if(seperator < 0) {
			return;
		}

		httpData.requestPath = stringPath.substring(0, seperator); // parse the resource path only

		start = seperator + 1; // start after ?
		while(start > 0) {
			end = stringPath.indexOf('&', start);
			seperator = stringPath.indexOf('=', start);
			
			if(seperator > 0) {
				httpData.parameters.put(
					stringPath.substring(start, seperator),
					stringPath.substring(seperator+1, (end < 0 ? stringPath.length() : end))
				);
			}

			start = end;
			start++; // if end is -1, then after start++ will be 0, and looping exited
		}
	}

	private static void parseHeaders(String headers, httpObject httpData) {
		int startIndex = 0, endIndex = -1, seperator = 0;
		boolean looping = true;

		while(looping) {
			startIndex = endIndex + 1;
			endIndex = headers.indexOf('\n', startIndex);
			seperator = headers.indexOf(": ", startIndex);
			
			if(endIndex < 0) {
				looping = false;
			}
			
			if(seperator < 0) {
				continue;
			}
			
			httpData.addHeader(
				headers.substring(startIndex, seperator),
				headers.substring(seperator + 2, (endIndex < 0 ? headers.length()-1 : endIndex))
			);
		}
	}
}
