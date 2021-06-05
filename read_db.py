import sqlite3 as sl

con = sl.connect('sleep_data.db')
cur = con.cursor()
data = cur.execute("SELECT * FROM DATA;")
for row in data:
	print(row)
