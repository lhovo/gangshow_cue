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

class IndexHandler(web.RequestHandler):
	def initialize(self, ref_object, scene_object):
		self.getWorksheet = ref_object
		self.scene = scene_object

	def get(self):
		data = []
		for i in self.getWorksheet():
			if not self.scene in i.title:
				data.append(i.title)
		self.render("index.html", data=data)

class ScriptHandler(web.RequestHandler):
	def initialize(self, ref_object, scene_object):
		self.getWorksheet = ref_object
		self.scene = scene_object

	def set_default_headers(self):
		self.set_header("type", "text/javascript")

	def findScenes(self, sceneList, number):
		outScene = sceneList[0]['Scene']
		
		for scene in sceneList:
			try:
				if int(scene['Cue']) <= number:
					outScene = scene['Scene']
			except ValueError:
				pass

		return {"Scene":outScene}

	def fullScript(self, keys):
		data = []
		for i in self.getWorksheet():
			print(i)
			if keys in i.title:
				data = i.get_all_records()
			elif self.scene in i.title:
				scenes = i.get_all_records()
		
		output = {}
		presentCue = 0
		additionalCue = 0
		for a in data:
			if isinstance(a['Cue'], int):
				presentCue = a['Cue']
				additionalCue = 0
			else:
				additionalCue += 1

			if True: #a['TeamNote'] != '':
				output[presentCue*1000 + additionalCue] = {**a, **self.findScenes(scenes, presentCue)}
				del(output[presentCue*1000 + additionalCue]['Cue']) # remove the duplicate
				output[presentCue*1000 + additionalCue]['RunScript'] = "<br />".join(a['RunScript'].split("\n"))
				output[presentCue*1000 + additionalCue]['TeamNote'] = "<br />".join(a['TeamNote'].split("\n"))

		return output

	def cueOnly(self, keys):
		data = []
		for i in self.getWorksheet():
			if keys in i.title:
				data = i.get_all_records()
				break
		
		output = {}
		presentCue = 0
		for a in data:
			if isinstance(a['Cue'], int):
				presentCue = a['Cue']
				output[presentCue] = {**a}
				del(output[presentCue]['Cue']) # remove the duplicate

		return output

	def get(self, keys):
		# output = self.fullScript(keys)
		output = self.cueOnly(keys)
		self.render("data.js", data=json.dumps(output))

class CuePageHandler(web.RequestHandler):
	def getPageLayout(self, lookup):
		page = {
			"Cast": "cast.html",
			"Lighting": "lighting.html",
			3: "March",
			4: "April",
			5: "May",
			6: "June",
			7: "July",
			8: "August",
			9: "September",
			10: "October",
			11: "November",
			12: "December"
		}
		return page.get(lookup, "cue.html")

	def get(self, lookup):	
		self.render(self.getPageLayout(lookup), data=lookup)

cl = []
clientMessage = {'cue':1, 'standby':0}

class SocketHandler(websocket.WebSocketHandler):
	def check_origin(self, origin):
		return True

	def open(self):
		if self not in cl:
			cl.append(self)
			self.write_message(json.dumps(clientMessage))

	def on_close(self):
		if self in cl:
			cl.remove(self)

	# Mssage echo it out to other clients if it's json
	def on_message(self, message):
		try:
			incoming = json.loads(message)
			if 'cue' in incoming:
				clientMessage['cue'] = incoming['cue']
			if 'standby' in incoming:
				clientMessage['standby'] = incoming['standby']
			for client in cl:
				if not self is client:
					if 'update' in incoming:
						client.write_message(json.dumps(incoming))
					else:
						client.write_message(json.dumps(clientMessage))

			if 'hlp' in incoming:
				self.write_message(json.dumps({"update":clientMessage['cue']}))

		except json.decoder.JSONDecodeError:
			print("Unable to decode: ", message)

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

	gc = gspread.authorize(creds)

	# Open a worksheet from spreadsheet with one shot
	wks = gc.open_by_key(SPREADSHEET_ID)

	return wks.worksheets()

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
			(r"/", IndexHandler, {"ref_object" : getWorksheet, "scene_object": LOOKPUP_SHEET }),
			(r'/asset/(.*)', web.StaticFileHandler, {'path': 'templates'}),
			(r"/cue/(?P<lookup>.*)", CuePageHandler, {}),
			(r'/script/(?P<keys>.*)', ScriptHandler, {"ref_object" : getWorksheet, "scene_object": LOOKPUP_SHEET }),
			(r"/ws", SocketHandler),
			(r'/(favicon\.ico)', web.StaticFileHandler, {'path': 'templates'}),
		],
		cookie_secret=COOKIE_SECRET,
		template_path=os.path.join(os.path.dirname(__file__), "templates"),
		static_path=os.path.join(os.path.dirname(__file__), "templates"),
		xsrf_cookies=True,
		debug=options.debug,
	)
	app.listen(options.port)
	ioloop.IOLoop.current().start()

if __name__ == '__main__':

	main()
