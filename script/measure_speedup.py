#!/usr/bin/env python2
import argparse
import csv
import os
import shlex
import subprocess


def execute_command(command):
	out, err = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
	return out,err

def read_info_file(file_path):
	apps = {}
	with open(file_path) as f:
		reader = csv.DictReader(f)
		for row in reader:
			name = row["name"].lower()
			if name not in apps:
				apps[name] = {}
			apps[name][row["version"]] = {}
			details = apps[name][row["version"]]
			for field in ["compile_path", "compile_command", "run_path", "run_file"]:
				details[field] = row[field]
	return apps

def average(values):
	if float('nan') in values:
		return float('nan')
	if len(values) >= 7:
		total = sum(values) - max(values) - min(values)
		num = len(values) - 2
	else:
		total = sum(values)
		num = len(values)
	return total / num

def measure_app(config, iterations):
	time_list = []

	# compile
	os.chdir(os.path.join(os.environ["TSX_ROOT"], "benchmark", config["compile_path"]))
	execute_command(config["compile_command"])
	
	# run
	os.chdir(os.path.join(os.environ["TSX_ROOT"], "benchmark", config["run_path"]))
	for _ in range(iterations):
		out, _ = execute_command("run-benchmark.py -r native -t 14 -c 0,4,8,12,16,20,24,28,32,36,40,44,48,52 " + config["run_file"])
		try:
			time = float(filter(lambda x:x.find("##MEASUREMENT:")>=0, out.split("\n"))[0].split()[-1])
		except ValueError:
			time = float('nan')
		time_list.append(time)
	return time_list

def main():
	parser = argparse.ArgumentParser()
	parser.add_argument("applications", nargs='?', default=None, help="the name list of the applications (separated by ,), or \"all\" for all the available applications")
	parser.add_argument("-l", "--list", action='store_true', help="list all available applications")
	parser.add_argument("-i", "--iterations", type=int, default=7, help="the number of times of running before averaging")
	args = parser.parse_args()

	# read info file
	apps = read_info_file(os.path.join(os.path.dirname(os.path.realpath(__file__)), "speedup-list.csv"))

	if args.list:
		print("Available applications:\n")
		print("\n".join(apps.keys()))
		return

	if args.applications is None:
		print("Please specify an application or use 'all'")
		return

	if args.applications.lower() == "all":
		names = apps.keys()
	else:
		names = map(lambda x: x.lower(), args.applications.split(","))
	
	if any([name not in apps.keys() for name in names]):
		print("Please specify an application or use 'all'")
		return

	for name in names:
		origin_time = measure_app(apps[name]["original"], args.iterations)
		
		opt_time = measure_app(apps[name]["optimized"], args.iterations) 
		
		origin_avg = average(origin_time)
		opt_avg = average(opt_time)
		speedup = origin_avg / opt_avg
		
		print("{}  origin:{}s  opt:{}s  speedup:{}X".format(name, round(origin_avg,2), round(opt_avg,2), round(speedup,2)))
	
	
	# compile and run origin/opt

	# show result

	

main()
