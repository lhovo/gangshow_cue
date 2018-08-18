import gspread
from oauth2client import file, client, tools

# If modifying these scopes, delete the file token.json.
SCOPES = 'https://www.googleapis.com/auth/spreadsheets.readonly'
SAMPLE_SPREADSHEET_ID = '1eZnpg01GLe36Y5XCI1ceR8PjJPWazy0hflXuHH7JEEQ'
SCENE_LOOKUP = "scene_lookup"

def main():
	"""Shows basic usage of the Sheets API.
	Prints values from a sample spreadsheet.
	"""
	store = file.Storage('token.json')
	creds = store.get()
	if not creds or creds.invalid:
		flow = client.flow_from_clientsecrets('credentials.json', SCOPES)
		creds = tools.run_flow(flow, store)

	gc = gspread.authorize(creds)

	# Open a worksheet from spreadsheet with one shot
	wks = gc.open_by_key(SAMPLE_SPREADSHEET_ID)
	lookup = wks.worksheet(SCENE_LOOKUP).get_all_records()

	for i in wks.worksheets():
		if not SCENE_LOOKUP in i.title:
			print(i.title)
			print(i.get_all_records())

if __name__ == '__main__':
	main()