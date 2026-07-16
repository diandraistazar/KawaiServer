import java.net.*;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.EOFException;
import java.io.FileNotFoundException;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.util.Map;
import java.util.Arrays;
import java.util.List;
import java.util.logging.Logger;
import java.lang.ProcessBuilder;
import java.lang.Process;

import http.httpParser;
import http.httpObject;
import mime.Mime;

public class KawaiServer {
	public static final Logger LOGGER = Logger.getLogger(KawaiServer.class.getName());
	public static InetAddress IPAddress = null;
	public static int port = 0;
	public static File rootDir = null;
	
	private static ServerSocket socket = null;
	private static boolean looping = true;

	static {
		// $1 = date
		// $4 = Level Name (e.g ERROR, SEVERE)
		// $5 = message
		System.setProperty("java.util.logging.SimpleFormatter.format", "[%1$tY-%1$tm-%1$td %1$tH:%1$tM:%1$tS] %4$-6s: %5$s %n");
	}

	public static void main(String[] argv) {
		if(argv.length < 3) {
			System.out.println("KawaiServer: <ip-address> <port> <root-directory>");
			System.exit(1);
		}

		if(!setupServer(argv)) {
			KawaiServer.LOGGER.severe("Unable to setup the server");
			cleanupServer(); // After create and bound the server socket, but an error occured, the program has to run cleanupServer() to clean them
			System.exit(1);
		}

		KawaiServer.LOGGER.info("");
		KawaiServer.LOGGER.info("### SERVER CONFIGURATION");
		KawaiServer.LOGGER.info("IP=" + KawaiServer.getAddressStr());
		KawaiServer.LOGGER.info("ROOT_DIR=" + KawaiServer.rootDir.getAbsolutePath());
		KawaiServer.LOGGER.info("DEFAULT=/index.html");
		KawaiServer.LOGGER.info("SERVICE=HTTP");
		KawaiServer.LOGGER.info("");

		KawaiServer.LOGGER.info("Starting the server..");
		startServer();
		
		KawaiServer.LOGGER.info("Cleaning up the server..");
		cleanupServer();

		KawaiServer.LOGGER.info(KawaiServer.class.getSimpleName() + " terminated");
	}

	public static void cleanupServer() {
		// Close the server socket
		if(socket != null && !socket.isClosed()) {
			try {
				socket.close();
			} catch(IOException e) {
				KawaiServer.LOGGER.info("Cannot close the server socket. SKIP");
			}
		}
	}

	public static String getAddressStr() {
		return KawaiServer.socket.getInetAddress().getHostAddress() + ":" + KawaiServer.socket.getLocalPort();
	}

	public static boolean setupServer(String[] args) {
		// Set the server socket's IP address
		try {
			IPAddress = InetAddress.getByName(args[0]);
		} catch(UnknownHostException e) {
			KawaiServer.LOGGER.severe("The specified ip address is unknown");
			return false;
		}

		// Set the server port
		try {
			port = Integer.parseInt(args[1]);
		} catch(NumberFormatException e) {
			KawaiServer.LOGGER.severe("The specified port isn't correct format: " + e);
			return false;
		}

		// Create the server socket
		try {
			socket = new ServerSocket(port, 1, IPAddress);
		} catch(IOException e) {
			KawaiServer.LOGGER.severe("while creating a new server socket " + e);
			return false;
		}

		KawaiServer.LOGGER.info("The socket server will be set for " + KawaiServer.getAddressStr());

		// Set socket options for the socket server
		try {
			socket.setReuseAddress(true);
		} catch(SocketException e) {
			KawaiServer.LOGGER.severe("while setting socket options: " + e);
			return false;
		}

		KawaiServer.LOGGER.info("The socket has been set for some options");

		// Checking whetever the requested root dir is valid or not
		rootDir = new File(args[2]);
		if(!rootDir.isDirectory()) {
			KawaiServer.LOGGER.severe("The specified root-directory is either doesn't exists or non-directory");
			return false;
		}

		KawaiServer.LOGGER.info("The specified root-directory is mapped");

		return true;
	}

	public static void sendFile(KawaiClient clientDest, File filepath) throws FileNotFoundException,IOException {
		FileInputStream fileStream = new FileInputStream(filepath);
		byte[] buffer = new byte[512];
		int readBytes = 0;

		while(fileStream.available() > 0) {
			Arrays.fill(buffer, (byte)0);

			readBytes = fileStream.read(buffer);
			if(readBytes > 0) {
				KawaiServer.sendData(clientDest, buffer);
			} else {
				break;
			}
		}

		fileStream.close();
	}

	public static void sendData(KawaiClient clientDest, byte[] data) throws IOException {
		DataOutputStream clientOut = clientDest.outputStream;
		
		clientOut.write(data);
	}

	public static byte[] recvData(KawaiClient client_src) throws IOException {
		DataInputStream clientIn = client_src.inputStream;
		byte[] buffer = new byte[0];
		
		buffer = new byte[clientIn.available()];
		clientIn.readFully(buffer);

		return buffer;
	}

	public static void handleClient(KawaiClient client) throws IOException,EOFException {
		byte[] receivedData = null;
		httpObject httpData = null;

		receivedData = KawaiServer.recvData(client);
		if(receivedData.length < 1) {
			throw new EOFException("the size of received data is 0 bytes");
		}
		
		// Parsing the received bytes
		httpData = httpParser.parse(receivedData);
		if(httpData == null) {
			KawaiServer.LOGGER.severe("HTTP request isn't recognized");
			return;
		}
		
		// Print the connected client information
		KawaiServer.LOGGER.info("");
		KawaiServer.LOGGER.info("### CONNECTED CLIENT INFO");
		KawaiServer.LOGGER.info("IP=" + client.getAddressStr());
		KawaiServer.LOGGER.info("SIZE=" + receivedData.length + " bytes");
		KawaiServer.LOGGER.info("## HTTP RESPONSE INFO");
		KawaiServer.LOGGER.info("METHOD=" + httpData.method);
		KawaiServer.LOGGER.info("PATH=" + httpData.requestPath);
		KawaiServer.LOGGER.info("PARAMS=" + httpData.parameters.size());
		KawaiServer.LOGGER.info("VERSION=" + httpData.httpVersion);
		KawaiServer.LOGGER.info("HEADERS=" + httpData.headers.size());
		KawaiServer.LOGGER.info("BODY=" + (httpData.body.length > 0 ? true : false));
		KawaiServer.LOGGER.info("");

		if(httpData.method.compareTo("GET") == 0) {
			KawaiServer.handleGETMethod(client, httpData);
		}
	}

	public static void startServer() {
		KawaiServer.LOGGER.info("Listening connections..");

		while(KawaiServer.looping) {
			KawaiClient client = null;
			
			try{
				client = new KawaiClient(KawaiServer.socket.accept());
				
				KawaiServer.handleClient(client);
			} catch(EOFException e) {
				KawaiServer.LOGGER.severe("calling KawaiServer.recvData(): " + e);
				continue;
			} catch(IOException e) {
				KawaiServer.LOGGER.severe("calling socket.accept(): " + e);
				continue;
			} finally {
				client.close();
				KawaiServer.LOGGER.info(client.getAddressStr() + " disconnected");
			}
		}
	}

	// These below are services
	public static String PHPService(httpObject httpData, String file) {
		ProcessBuilder php = new ProcessBuilder("php", file);
		DataInputStream phpInput = null;
		Process phpProc = null;
		byte[] data = null;
		Map<String, String> envs = php.environment();
		
		envs.put("ROOT_DIR", KawaiServer.rootDir.getPath());
		envs.put("REQUEST_PATH", httpData.requestPath);

		for(Map.Entry<String, String> parameter : httpData.parameters.entrySet()) {
			envs.put(parameter.getKey(), parameter.getValue());
		}

		try {
			phpProc = php.start();
			phpProc.waitFor();

			if(phpProc.exitValue() != 0) {
				KawaiServer.LOGGER.severe("Exit code of PHP is non-zero");
				return null;
			}
			
			phpInput = new DataInputStream(phpProc.getInputStream());
			if(phpInput.available() > 0) {
				data = new byte[phpInput.available()];
			} else {
				return null;
			}
			
			phpInput.read(data);
		} catch(Exception e) {
			KawaiServer.LOGGER.severe("PHPService got: " + e);
			return null;
		} finally {
			if(phpProc != null) {
				phpProc.destroyForcibly();
			}
		}

		return new String(data);
	}

	// These below are methods handling
	
	public static void handleGETMethod(KawaiClient client, httpObject httpData) {
		File requestPath = new File(KawaiServer.rootDir.getPath() + httpData.requestPath);
		String httpResponse = null;
		httpObject createdHttp = new httpObject();
		boolean useBody = true;
		
		createdHttp.type = httpParser.MessageType.HTTP_RESPONSE;
		createdHttp.httpVersion = "HTTP/1.1";
		createdHttp.addHeader("Server", "KawaiServer");
		
		// If the requested resource is exists
		if(requestPath.exists()) {
			String mimeType = null;

			// If the requested resource is directory
			if(requestPath.isDirectory()) {
				List<String> listItem = Arrays.asList(requestPath.list());

				if(listItem.contains("index.html")) {
					requestPath = new File(requestPath.getPath() + "/index.html");
					mimeType = Mime.mimeTypes.get("html");
					
					createdHttp.statusCode = 200;
					createdHttp.statusDesc = "Found";
					createdHttp.addHeader("Content-Type", mimeType);
					createdHttp.addHeader("Content-Length", String.valueOf(requestPath.length()));

					useBody = false;
				}
				else if(listItem.contains("index.php")) {
					requestPath = new File(requestPath.getPath() + "/index.php");
					String data = KawaiServer.PHPService(httpData, requestPath.getPath());
					
					createdHttp.statusCode = 200;
					createdHttp.statusDesc = "Found";

					if(data == null) {
						mimeType = Mime.mimeTypes.get("php");
						createdHttp.addHeader("Content-Length", String.valueOf(requestPath.length()));

						useBody = false;
					} else {
						mimeType = Mime.mimeTypes.get("html");
						createdHttp.body = data.getBytes();
						createdHttp.addHeader("Content-Length", String.valueOf(createdHttp.body.length));
					}
					
					createdHttp.addHeader("Content-Type", Mime.mimeTypes.get("html"));
				}
				else {
					listItem = Arrays.asList(KawaiServer.rootDir.list());

					if(listItem.contains("index.html")) {
						requestPath = new File(KawaiServer.rootDir.getPath() + "/index.html");
						mimeType = Mime.mimeTypes.get("html");
						
						createdHttp.statusCode = 200;
						createdHttp.statusDesc = "Found";
						createdHttp.addHeader("Content-Type", mimeType);
						createdHttp.addHeader("Content-Length", String.valueOf(requestPath.length()));

						useBody = false;
					}
					else if(listItem.contains("index.php")) {
						requestPath = new File(KawaiServer.rootDir.getPath() + "/index.php");
						String data = KawaiServer.PHPService(httpData, requestPath.getPath());
						
						createdHttp.statusCode = 200;
						createdHttp.statusDesc = "Found";

						if(data == null) {
							mimeType = Mime.mimeTypes.get("php");
							createdHttp.addHeader("Content-Length", String.valueOf(requestPath.length()));

							useBody = false;
						} else {
							mimeType = Mime.mimeTypes.get("html");
							createdHttp.body = data.getBytes();
							createdHttp.addHeader("Content-Length", String.valueOf(createdHttp.body.length));
						}
						
						createdHttp.addHeader("Content-Type", Mime.mimeTypes.get("html"));
					}
					else {
						createdHttp.statusCode = 404;
						createdHttp.statusDesc = "Not Found";
					}
				}
			}

			// If the requested resource is file
			else if(requestPath.isFile()) {
				mimeType = Mime.getMimeType(requestPath.getName());
				String data = null;

				if(mimeType == null) {
					mimeType = Mime.mimeTypes.get("txt");
					createdHttp.addHeader("Content-Length", String.valueOf(requestPath.length()));
					
					useBody = false;
				}
				else if(mimeType.compareTo(Mime.mimeTypes.get("php")) == 0) {
					data = KawaiServer.PHPService(httpData, requestPath.getPath());

					if(data != null) {
						mimeType = Mime.mimeTypes.get("html");
						createdHttp.body = data.getBytes();
						createdHttp.addHeader("Content-Length", String.valueOf(createdHttp.body.length));
					}
					else {
						createdHttp.addHeader("Content-Length", String.valueOf(requestPath.length()));
					}
				} 
				else {
					createdHttp.addHeader("Content-Length", String.valueOf(requestPath.length()));
					
					useBody = false;
				}
				
				createdHttp.statusCode = 200;
				createdHttp.statusDesc = "Found";
				createdHttp.addHeader("Content-Type", mimeType);
			}
		} 
		
		// If the resource resource isn't exists
		else {
			KawaiServer.LOGGER.info(requestPath + " isn't exists");
			
			createdHttp.statusCode = 404;
			createdHttp.statusDesc = "Not Found";
		}
		
		// Create the HTTP response
		httpResponse = httpParser.create(createdHttp);
		if(httpResponse == null) {
			KawaiServer.LOGGER.severe("Cannot create the HTTP response");
			return;
		}

		// Then, send the data to the connected client
		try {
			KawaiServer.sendData(client, httpResponse.getBytes());

			if(!useBody) {
				KawaiServer.sendFile(client, requestPath);
			}
		} catch(Exception e) {
			KawaiServer.LOGGER.severe("Failed to send the resource: " + e);
			return;
		}
		
		KawaiServer.LOGGER.info("Successfully to send the resource to the connected client");
	}
}

class KawaiClient {
	public DataInputStream inputStream;
	public DataOutputStream outputStream;

	private Socket socket;
	
	KawaiClient(Socket created_socket) throws IOException {
		this.socket = created_socket;

		this.inputStream = new DataInputStream(socket.getInputStream());
		this.outputStream = new DataOutputStream(socket.getOutputStream());
	}

	public String getAddressStr() {
		return this.socket.getInetAddress().getHostAddress() + ":" + this.socket.getPort();
	}

	public void close() {
		if(this.socket != null && !this.socket.isClosed()) {
			try {
				this.socket.close();
			} catch(IOException e) {
				KawaiServer.LOGGER.info("IOException occured from socket.close(). SKIP");
			}
		}
	}
}
