from tornado.ioloop import IOLoop, PeriodicCallback
from tornado import gen
from tornado.websocket import websocket_connect
from tornado.queues import Queue

messageQue = Queue(maxsize=2)

class Client(object):
	def __init__(self, url, timeout, ioloop):
		self.url = url
		self.timeout = timeout
		self.ioloop = ioloop
		self.ws = None
		self.connect()
		PeriodicCallback(self.keep_alive, 20000).start()

	@gen.coroutine
	def connect(self):
		print("trying to connect")
		try:
			self.ws = yield websocket_connect(self.url)
		except Exception as e:
			print("connection error", e)
		else:
			print("connected")
			self.run()

	@gen.coroutine
	def run(self):
		while True:
			if "hardware" in self.url:
				item = yield messageQue.get()
				self.ws.write_message(item)
				messageQue.task_done()
			else:
				msg = yield self.ws.read_message()
				if msg is None:
					print("connection closed")
					self.ws = None
					break
				else:
					try:
						messageQue.put_nowait(msg)
					except:
						print("unable to put msg on que",msg)
						pass

	def keep_alive(self):
		if self.ws is None:
			self.connect()
		else:
			self.ws.write_message('{"keep alive":1}')

if __name__ == "__main__":
	ioloop = IOLoop.instance()
	clientHardware = Client("wss://xx/hardware", 5, ioloop)
	clientClient = Client("ws://127.0.0.1:8888/ws", 5, ioloop)
	ioloop.start()
