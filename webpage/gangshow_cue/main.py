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

class MessageBuffer(object):
	def __init__(self):
		# cond is notified whenever the message cache is updated
		self.cond = locks.Condition()
		self.cache = []
		self.cache_size = 200

	def get_messages_since(self, cursor):
		"""Returns a list of messages newer than the given cursor.
		``cursor`` should be the ``id`` of the last message received.
		"""
		results = []
		for msg in reversed(self.cache):
			if msg["id"] == cursor:
				break
			results.append(msg)
		results.reverse()
		return results

	def add_message(self, message):
		self.cache.append(message)
		if len(self.cache) > self.cache_size:
			self.cache = self.cache[-self.cache_size:]
		self.cond.notify_all()


# Making this a non-singleton is left as an exercise for the reader.
global_message_buffer = MessageBuffer()


class IndexHandler(web.RequestHandler):
	def get(self):
		# for i in wks.worksheets():
		# 	if not SCENE_LOOKUP in i.title:
		# 		print(i.title)
		# 		print(i.get_all_records())
		self.render("index.html", messages=global_message_buffer.cache)

class CueHandler(web.RequestHandler):
	def get(self):
		# for i in wks.worksheets():
		# 	if not SCENE_LOOKUP in i.title:
		# 		print(i.title)
		# 		print(i.get_all_records())
		self.render("cue.html", messages=global_message_buffer.cache)

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

	app = web.Application(
		[
			(r"/", IndexHandler),
			(r"/cue", CueHandler),
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
