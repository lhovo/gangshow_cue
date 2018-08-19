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
define("demo", default=False, help="run with demo data")

class IndexHandler(web.RequestHandler):
	def initialize(self, ref_object, scene_object):
		self.wks = ref_object
		self.scene = scene_object

	def get(self):
		data = []
		if options.demo:
			with open('demo.json', 'r') as f:
				output = json.load(f)
		else:
			for i in self.wks.worksheets():
				if not self.scene in i.title:
					data.append(i.title)

		self.render("index.html", data=json.dumps(data))

class ScriptHandler(web.RequestHandler):
	def initialize(self, ref_object, scene_object):
		self.wks = ref_object
		self.scene = scene_object

	def get(self):
		if options.demo:
			with open('demo.json', 'r') as f:
				output = json.load(f)
		else:
			data = []
			for i in self.wks.worksheets():
				if not self.scene in i.title:
					# print(i.title)
					data = i.get_all_records()
				else:
					scenes = i.get_all_records()
			output = {}
			for a in data:
				output[a['Cue']] = {**a, **self.findScenes(scenes, a['Cue'])}

		self.render("script.js", data=json.dumps(output))

	def findScenes(self, sceneList, number):
		return {"Scene":"The witch"}

class CueHandler(web.RequestHandler):
	def initialize(self, ref_object, scene_object):
		self.wks = ref_object
		self.scene = scene_object
		
	def get(self):
		data = []
		if options.demo:
			with open('demo.json', 'r') as f:
				data = json.load(f)
		else:
			for i in self.wks.worksheets():
				if not self.scene in i.title:
					# print(i.title)
					data = i.get_all_records()
					break
		self.render("cue.html", data=json.dumps(data))

cl = []

class SocketHandler(websocket.WebSocketHandler):
	def check_origin(self, origin):
		return True

	def open(self):
		if self not in cl:
			cl.append(self)

	def on_close(self):
		if self in cl:
			cl.remove(self)

	# Mssage echo it out to other clients if it's json
	def on_message(self, message):
		try:
			incoming = json.loads(message)
			for client in cl:
				if not self is client:
					client.write_message(json.dumps(incoming))
		except json.decoder.JSONDecodeError:
			print("Unable to decode: ", message)

# If modifying these scopes, delete the file token.json.
SCOPES = 'https://www.googleapis.com/auth/spreadsheets.readonly'
SPREADSHEET_ID = ""
LOOKPUP_SHEET = ""

def main():
	parse_command_line()
	COOKIE_SECRET = "__TODO:_GENERATE_YOUR_OWN_RANDOM_VALUE_HERE__"

	if not options.demo:
		with open('input_keys.json', 'r') as f:
			input_keys = json.load(f)
			SPREADSHEET_ID = input_keys['SPREADSHEET_ID']
			LOOKPUP_SHEET = input_keys['LOOKPUP_SHEET']
			COOKIE_SECRET = input_keys['COOKIE_SECRET']

		store = file.Storage('token.json')
		creds = store.get()
		if not creds or creds.invalid:
			flow = client.flow_from_clientsecrets('credentials.json', SCOPES)
			creds = tools.run_flow(flow, store)

		gc = gspread.authorize(creds)

		# Open a worksheet from spreadsheet with one shot
		wks = gc.open_by_key(SPREADSHEET_ID)
		lookup = wks.worksheet(LOOKPUP_SHEET).get_all_records()
	else:
		wks = None
		LOOKPUP_SHEET = None

	app = web.Application(
		[
			(r"/", IndexHandler, {"ref_object" : wks, "scene_object": LOOKPUP_SHEET }),
			(r"/cue", CueHandler, {"ref_object" : wks, "scene_object": LOOKPUP_SHEET }),
			(r'/script\.js', ScriptHandler, {"ref_object" : wks, "scene_object": LOOKPUP_SHEET }),
			(r"/ws", SocketHandler),
			(r'/(favicon\.ico)', web.StaticFileHandler, {'path': '/static/favicon.ico'}),
		],
		cookie_secret=COOKIE_SECRET,
		template_path=os.path.join(os.path.dirname(__file__), "templates"),
		static_path=os.path.join(os.path.dirname(__file__), "static"),
		xsrf_cookies=True,
		debug=options.debug,
	)
	app.listen(options.port)
	ioloop.IOLoop.current().start()

if __name__ == '__main__':

	main()
