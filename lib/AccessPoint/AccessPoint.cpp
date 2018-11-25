/*
 * AccessPoint.cpp
 *
 *  Created on: 25.11.2018
 *      Author: conradwild
 */

#include "AccessPoint.h"
#define DBG_OUTPUT_PORT Serial

ESP8266WebServer AccessPoint::server;
WebContent AccessPoint::web_content;

static bool hasSD = false;
File uploadFile;

static void returnOK() {
	AccessPoint::server.send(200, "text/plain", "");
}

static void returnFail(String msg) {
	AccessPoint::server.send(500, "text/plain", msg + "\r\n");
}

static bool loadFromSdCard(String path) {
	String dataType = "text/plain";
	if (path.endsWith("/")) {
		path += "index.htm";
	}

	if (path.endsWith(".src")) {
		path = path.substring(0, path.lastIndexOf("."));
	} else if (path.endsWith(".htm")) {
		dataType = "text/html";
	} else if (path.endsWith(".css")) {
		dataType = "text/css";
	} else if (path.endsWith(".js")) {
		dataType = "application/javascript";
	} else if (path.endsWith(".png")) {
		dataType = "image/png";
	} else if (path.endsWith(".gif")) {
		dataType = "image/gif";
	} else if (path.endsWith(".jpg")) {
		dataType = "image/jpeg";
	} else if (path.endsWith(".ico")) {
		dataType = "image/x-icon";
	} else if (path.endsWith(".xml")) {
		dataType = "text/xml";
	} else if (path.endsWith(".pdf")) {
		dataType = "application/pdf";
	} else if (path.endsWith(".zip")) {
		dataType = "application/zip";
	}

	File dataFile = SD.open(path.c_str());
	if (dataFile.isDirectory()) {
		path += "/index.htm";
		dataType = "text/html";
		dataFile = SD.open(path.c_str());
	}

	if (!dataFile) {
		return false;
	}

	if (AccessPoint::server.hasArg("download")) {
		dataType = "application/octet-stream";
	}

	if (AccessPoint::server.streamFile(dataFile, dataType) != dataFile.size()) {
		DBG_OUTPUT_PORT.println("Sent less data than expected!");
	}

	dataFile.close();
	return true;
}

static void handleFileUpload() {
	if (AccessPoint::server.uri() != "/edit") {
		return;
	}
	HTTPUpload& upload = AccessPoint::server.upload();
	if (upload.status == UPLOAD_FILE_START) {
		if (SD.exists((char *) upload.filename.c_str())) {
			SD.remove((char *) upload.filename.c_str());
		}
		uploadFile = SD.open(upload.filename.c_str(), FILE_WRITE);
		DBG_OUTPUT_PORT.print("Upload: START, filename: ");
		DBG_OUTPUT_PORT.println(upload.filename);
	} else if (upload.status == UPLOAD_FILE_WRITE) {
		if (uploadFile) {
			uploadFile.write(upload.buf, upload.currentSize);
		}
		DBG_OUTPUT_PORT.print("Upload: WRITE, Bytes: ");
		DBG_OUTPUT_PORT.println(upload.currentSize);
	} else if (upload.status == UPLOAD_FILE_END) {
		if (uploadFile) {
			uploadFile.close();
		}
		DBG_OUTPUT_PORT.print("Upload: END, Size: ");
		DBG_OUTPUT_PORT.println(upload.totalSize);
	}
}

static void deleteRecursive(String path) {
	File file = SD.open((char *) path.c_str());
	if (!file.isDirectory()) {
		file.close();
		SD.remove((char *) path.c_str());
		return;
	}

	file.rewindDirectory();
	while (true) {
		File entry = file.openNextFile();
		if (!entry) {
			break;
		}
		String entryPath = path + "/" + entry.name();
		if (entry.isDirectory()) {
			entry.close();
			deleteRecursive(entryPath);
		} else {
			entry.close();
			SD.remove((char *) entryPath.c_str());
		}
		yield();
	}

	SD.rmdir((char *) path.c_str());
	file.close();
}

static void handleDelete() {
	if (AccessPoint::server.args() == 0) {
		return returnFail("BAD ARGS");
	}
	String path = AccessPoint::server.arg(0);
	if (path == "/" || !SD.exists((char *) path.c_str())) {
		returnFail("BAD PATH");
		return;
	}
	deleteRecursive(path);
	returnOK();
}

static void handleCreate() {
	if (AccessPoint::server.args() == 0) {
		return returnFail("BAD ARGS");
	}
	String path = AccessPoint::server.arg(0);
	if (path == "/" || SD.exists((char *) path.c_str())) {
		returnFail("BAD PATH");
		return;
	}

	if (path.indexOf('.') > 0) {
		File file = SD.open((char *) path.c_str(), FILE_WRITE);
		if (file) {
			file.write((const char *) 0);
			file.close();
		}
	} else {
		SD.mkdir((char *) path.c_str());
	}
	returnOK();
}

static void printDirectory() {
	if (!AccessPoint::server.hasArg("dir")) {
		return returnFail("BAD ARGS");
	}
	String path = AccessPoint::server.arg("dir");
	if (path != "/" && !SD.exists((char *) path.c_str())) {
		return returnFail("BAD PATH");
	}
	File dir = SD.open((char *) path.c_str());
	path = String();
	if (!dir.isDirectory()) {
		dir.close();
		return returnFail("NOT DIR");
	}
	dir.rewindDirectory();
	AccessPoint::server.setContentLength(CONTENT_LENGTH_UNKNOWN);
	AccessPoint::server.send(200, "text/json", "");
	WiFiClient client = AccessPoint::server.client();

	AccessPoint::server.sendContent("[");
	for (int cnt = 0; true; ++cnt) {
		File entry = dir.openNextFile();
		if (!entry) {
			break;
		}

		String output;
		if (cnt > 0) {
			output = ',';
		}

		output += "{\"type\":\"";
		output += (entry.isDirectory()) ? "dir" : "file";
		output += "\",\"name\":\"";
		output += entry.name();
		output += "\"";
		output += "}";
		AccessPoint::server.sendContent(output);
		entry.close();
	}
	AccessPoint::server.sendContent("]");
	dir.close();
}

static void handleNotFound() {
	if (hasSD && loadFromSdCard(AccessPoint::server.uri())) {
		return;
	}
	String message = "SDCARD Not Detected\n\n";
	message += "URI: ";
	message += AccessPoint::server.uri();
	message += "\nMethod: ";
	message += (AccessPoint::server.method() == HTTP_GET) ? "GET" : "POST";
	message += "\nArguments: ";
	message += AccessPoint::server.args();
	message += "\n";
	for (uint8_t i = 0; i < AccessPoint::server.args(); i++) {
		message += " NAME:" + AccessPoint::server.argName(i) + "\n VALUE:"
				+ AccessPoint::server.arg(i) + "\n";
	}
	AccessPoint::server.send(404, "text/plain", message);
	DBG_OUTPUT_PORT.print(message);
}

//static void handle_root() {
//	WebContent *wc;
//	String heading;
//	String line1;
//	String line2;
//	String line3;
//
//	wc = &AccessPoint::web_content;
//
//	wc->clear();
//
//	heading = wc->create_heading("HummelCounter");
//	line1 = wc->create_entry("Line1");
//
//	wc->append(heading);
//	wc->append(line1);
//
//	AccessPoint::server.send(200, "text/html",
//			AccessPoint::web_content.output());
//}

int AccessPoint::initWifi() {
	const char *ssid = sysdefs::wifi::ssid.c_str();
	const char *password = sysdefs::wifi::passphrase.c_str();
	const char* host = "esp8266sd";

	DBG_OUTPUT_PORT.begin(115200);
	DBG_OUTPUT_PORT.setDebugOutput(true);
	DBG_OUTPUT_PORT.print("\n");
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	DBG_OUTPUT_PORT.print("Connecting to ");
	DBG_OUTPUT_PORT.println(ssid);

	// Wait for connection
	uint8_t i = 0;
	while (WiFi.status() != WL_CONNECTED && i++ < 20) { //wait 10 seconds
		delay(500);
	}
	if (i == 21) {
		DBG_OUTPUT_PORT.print("Could not connect to");
		DBG_OUTPUT_PORT.println(ssid);
		while (1) {
			delay(500);
		}
	}
	DBG_OUTPUT_PORT.print("Connected! IP address: ");
	DBG_OUTPUT_PORT.println(WiFi.localIP());

	if (MDNS.begin(host)) {
		MDNS.addService("http", "tcp", 80);
		DBG_OUTPUT_PORT.println("MDNS responder started");
		DBG_OUTPUT_PORT.print("You can now connect to http://");
		DBG_OUTPUT_PORT.print(host);
		DBG_OUTPUT_PORT.println(".local");
	}

	server.on("/list", HTTP_GET, printDirectory);
	server.on("/edit", HTTP_DELETE, handleDelete);
	server.on("/edit", HTTP_PUT, handleCreate);
	server.on("/edit", HTTP_POST, []() {
		returnOK();
	}, handleFileUpload);
	server.onNotFound(handleNotFound);

	server.begin();
	DBG_OUTPUT_PORT.println("HTTP server started");

	if (SD.begin(SS)) {
		DBG_OUTPUT_PORT.println("SD Card initialized.");
		hasSD = true;
	}
	return 0;
}

AccessPoint::AccessPoint() {
	// TODO Auto-generated constructor stub

}

AccessPoint::~AccessPoint() {
	// TODO Auto-generated destructor stub
}

void AccessPoint::handleClient() {
	server.handleClient();
}

int AccessPoint::stopWifi() {
	server.stop();
	return 0;
}
