from oauth2client import file, client, tools
from tornado import websocket, web, ioloop, escape, locks
import json
import asyncio
import os.path
import uuid
import gspread

from tornado.options import define, options, parse_command_line

define("port", default=8888, help="run on the given port", type=int)
define("debug", default=True, help="run in debug mode")

# Return the main page with all the webpage selections on it
class IndexHandler(web.RequestHandler):
	def initialize(self, ref_object, scene_object):
		self.getWorksheet = ref_object
		self.scene = scene_object

	def get(self):
		data = []
		for i in self.getWorksheet():
			if not self.scene in i.title:
				data.append(i.title)

		if len(data) is 0:
			data.append("Unable to fetch data")
		self.render("index.html", data=data)

# Return the correct data from gSpreadsheets for the requested page
class ScriptHandler(web.RequestHandler):
	def initialize(self, ref_object):
		self.getWorksheet = ref_object

	def set_default_headers(self):
		self.set_header("type", "text/javascript")

	def cueOnly(self, pageName):
		data = []
		for sheet in self.getWorksheet():
			if pageName in sheet.title:
				data = sheet.get_all_records()
				break
		
		output = {}
		presentCue = 0
		for row in data:
			if isinstance(row['Cue'], int):
				presentCue = row['Cue']
				output[presentCue] = {**row}
				del(output[presentCue]['Cue']) # remove the duplicate

		return output

	def get(self, pageName):
		# output = self.fullScript(pageName)
		output = self.cueOnly(pageName)
		self.render("data.js", data=json.dumps(output))

# Return the correct HTML page
class CuePageHandler(web.RequestHandler):
	def getPageLayout(self, lookup):
		page = {
			"Cast": "cast.html",
			"Lighting": "lighting.html",
			"Sound": "sound.html"
		}
		return page.get(lookup, "cue.html")

	def get(self, lookup):	
		self.render(self.getPageLayout(lookup), data=lookup)

# WebSocket Handler
clientConnections = []
controllerConnections = []
clientMessage = {'cue':1, 'standby':0}

def sendClientMessage():
	for client in clientConnections:
		client.write_message(json.dumps(clientMessage))

class SocketHandler(websocket.WebSocketHandler):
	def initialize(self, connectionType):
		self.connectionType = connectionType

	def check_origin(self, origin):
		return True

	def open(self):
		if self.connectionType is "webSockHardware":
			if self not in controllerConnections:
				controllerConnections.append(self)
				self.write_message(json.dumps({"update":clientMessage['cue']}))
		elif self.connectionType is "webClient":
			if self not in clientConnections:
				clientConnections.append(self)
				self.write_message(json.dumps(clientMessage))

	def on_close(self):
		if self.connectionType is "webSockHardware":
			if self in controllerConnections:
				controllerConnections.remove(self)
		elif self.connectionType is "webClient":
			if self in clientConnections:
				clientConnections.remove(self)

	# Mssage echo it out to other clients if it's json
	def on_message(self, message):
		if self.connectionType is "webSockHardware":
			try:
				incoming = json.loads(message)
				if 'cue' in incoming:
					clientMessage['cue'] = incoming['cue']
				if 'standby' in incoming:
					clientMessage['standby'] = incoming['standby']

				if 'hlp' in incoming:
					self.write_message(json.dumps({"update":clientMessage['cue']}))

			except json.decoder.JSONDecodeError:
				print("Unable to decode:", message)
		else:
			print("Invalid message on channel", message)

# If modifying these scopes, delete the file token.json.
SCOPES = 'https://www.googleapis.com/auth/spreadsheets.readonly'
SPREADSHEET_ID = ""
LOOKPUP_SHEET = ""

def getWorksheet():
	store = file.Storage('token.json')
	creds = store.get()

	if not creds or creds.invalid:
		flow = client.flow_from_clientsecrets('credentials.json', SCOPES)
		creds = tools.run_flow(flow, store)

	try:
		gc = gspread.authorize(creds)

		# Open a worksheet from spreadsheet with one shot
		wks = gc.open_by_key(SPREADSHEET_ID)

		return wks.worksheets()
	except:
		print("Server unable to connect to data store")
		return []

def main():
	global SPREADSHEET_ID, LOOKPUP_SHEET

	parse_command_line()
	COOKIE_SECRET = "__TODO:_GENERATE_YOUR_OWN_RANDOM_VALUE_HERE__"

	with open('input_keys.json', 'r') as f:
		input_keys = json.load(f)
		SPREADSHEET_ID = input_keys['SPREADSHEET_ID']
		LOOKPUP_SHEET = input_keys['LOOKPUP_SHEET']
		COOKIE_SECRET = input_keys['COOKIE_SECRET']

	app = web.Application(
		[
			(r"/", IndexHandler, { "ref_object" : getWorksheet, "scene_object": LOOKPUP_SHEET }),
			(r'/asset/(.*)', web.StaticFileHandler, { 'path': 'templates'}),
			(r"/cue/(?P<lookup>.*)", CuePageHandler, {}),
			(r'/script/(?P<pageName>.*)', ScriptHandler, { "ref_object" : getWorksheet }),
			(r"/ws", SocketHandler, { "connectionType" : "webClient" }),
			(r"/hardware", SocketHandler, { "connectionType" : "webSockHardware" }),
		],
		cookie_secret=COOKIE_SECRET,
		template_path=os.path.join(os.path.dirname(__file__), "templates"),
		static_path=os.path.join(os.path.dirname(__file__), "static"),
		xsrf_cookies=True,
		debug=options.debug,
	)
	app.listen(options.port)
	# Send connected websocket clients an update every 2 seconds
	ioloop.PeriodicCallback(sendClientMessage, 2000).start()
	ioloop.IOLoop.current().start()

if __name__ == '__main__':

	main()
