
from flask import Flask, render_template, make_response
from flask import request
from datetime import datetime
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_agg import FigureCanvasAgg as FigureCanvas
import io
import sqlite3 as sl

app = Flask(__name__)
conn = sl.connect('sleep_data.db')

conn.execute("""CREATE TABLE IF NOT EXISTS DATA (month INTEGER, day INTEGER, time TEXT, humidity INTEGER, temp INTEGER, sound_val INTEGER, light_val INTEGER);""")
#conn.execute("""CREATE TABLE IF NOT EXISTS AVG (avgHumidity INTEGER, avgTemp INTEGER, avgSound INTEGER, avgLight INTEGER);""")
#cur.execute("INSERT INTO AVG VALUES (0, 0, 0, 0);")


def get_data():
	time = []
	humidity =[]
	temp = []
	sound_values =[]
	light_values =[]

	query = "SELECT * FROM DATA ORDER BY time DESC LIMIT 600;"
	conn =  sl.connect("sleep_data.db")
	data = conn.execute(query)
	for row in data:
		print(row)
		time.append(row[2])
		humidity.append(row[3])
		temp.append(row[4])
		sound_values.append(row[5])
		light_values.append(row[6])
	return [time[::-1], humidity[::-1], temp[::-1], sound_values[::-1], light_values[::-1]]


@app.route("/send")
def push_db():


	conn = sl.connect('sleep_data.db')

	now = datetime.now()
	month = int(now.strftime("%m"))
	day = int(now.strftime("%d"))
	time = now.strftime("%H:%M")

	humidity = int(request.args.get("humidity"))
	temp = int(request.args.get("temp"))
	sound_val = int(request.args.get("soundValue"))
	light_val = int(request.args.get("lightValue"))

	#avgs = cur.execute("SELECT * FROM AVG;")
	query = "INSERT INTO DATA VALUES (?, ?, ?, ?, ?, ?, ?);"
	#query1 = "INSERT INTO AVG VALUES (?, ?, ?, ?);"
	
	conn.execute(query, (month, day, time, humidity, temp, sound_val, light_val))
	#cur.execute(query1, (humidity, temp, sound_val, light_val))
	conn.commit()
	return "OK"




@app.route("/dashboard")
def dashboard():
	return render_template("index.html")



@app.route('/plot/temp')
def plot_temp():
	times, temps = get_data()[0], get_data()[2]
	#times = [datetime.strptime(time,"%H:%M") for time in times]
	#print(times[0].type)

	fig, ax = plt.subplots(1, 1, figsize=(8, 6))
	ax.set_title("Temperature [°F]")
	ax.set_xlabel("Times")
	ax.legend()
	ax.plot(times, temps)

	plt.xticks(rotation=90)
	ax.set_xticks(times[::80])
	canvas = FigureCanvas(fig)
	output = io.BytesIO()
	canvas.print_png(output)
	response = make_response(output.getvalue())
	response.mimetype = 'image/png'
	return response


@app.route('/plot/hum')
def plot_hum():
	times, humidity = get_data()[0], get_data()[1]
	#times = [datetime.strptime(time,"%H:%M") for time in times]
	#print(times[0].type)

	fig, ax = plt.subplots(1, 1, figsize=(8, 6))
	ax.set_title("Humidity (%)")
	ax.set_xlabel("Times")
	ax.legend()
	ax.plot(times, humidity)

	plt.xticks(rotation=90)
	ax.set_xticks(times[::80])
	canvas = FigureCanvas(fig)
	output = io.BytesIO()
	canvas.print_png(output)
	response = make_response(output.getvalue())
	response.mimetype = 'image/png'
	return response


@app.route('/plot/sound')
def plot_sound():
	times, sounds = get_data()[0], get_data()[3]
	#times = [datetime.strptime(time,"%H:%M") for time in times]
	#print(times[0].type)

	fig, ax = plt.subplots(1, 1, figsize=(8, 6))
	ax.set_title("Sound")
	ax.set_xlabel("Times")
	ax.legend()
	ax.plot(times, sounds)

	plt.xticks(rotation=90)
	ax.set_xticks(times[::80])
	canvas = FigureCanvas(fig)
	output = io.BytesIO()
	canvas.print_png(output)
	response = make_response(output.getvalue())
	response.mimetype = 'image/png'
	return response


@app.route('/plot/light')
def light():
	times, lights = get_data()[0], get_data()[4]
	#times = [datetime.strptime(time,"%H:%M") for time in times]
	#print(times[0].type)

	fig, ax = plt.subplots(1, 1, figsize=(8, 6))
	ax.set_title("Light")
	ax.set_xlabel("Times")
	ax.legend()
	ax.plot(times, lights)

	plt.xticks(rotation=90)
	ax.set_xticks(times[::80])
	canvas = FigureCanvas(fig)
	output = io.BytesIO()
	canvas.print_png(output)
	response = make_response(output.getvalue())
	response.mimetype = 'image/png'
	return response
