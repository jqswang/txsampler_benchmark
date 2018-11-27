#!/usr/bin/env python2
import argparse
import ConfigParser
import csv
import os
import shlex
import subprocess

g_vars = {}
g_vars["throughput_applications"] = ["linkedlist"]


def execute_command(command):
	if g_vars["verbose"]:
		print("EXECUTE: {}".format(command))
	child = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	out, err = child.communicate()
	return out,err

def read_info_file(file_path):
	if g_vars["verbose"]:
		print("Start to read {}".format(file_path))
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

def measure_app(name, config, iterations):
	time_list = []

	# compile
	working_dir = os.path.join(os.environ["TSX_ROOT"], "benchmark", config["compile_path"])
	if g_vars["verbose"]:
		print("Change working directory to {}".format(working_dir))
	os.chdir(working_dir)
	execute_command(config["compile_command"])
	
	# run
	working_dir = os.path.join(os.environ["TSX_ROOT"], "benchmark", config["run_path"])
	if g_vars["verbose"]:
		print("Change working directory to {}".format(working_dir))
	os.chdir(working_dir)
	for _ in range(iterations):
		out, _ = execute_command("run-benchmark.py -r native -t {} -c {} {} --show-output".format(g_vars["num_threads"], g_vars["cpu_list"], config["run_file"]))
		try:
			#print(out)
			if name in g_vars["throughput_applications"]:
				if name == "linkedlist":
					time = float(filter(lambda x:x.startswith("#txs"), out.split("\n"))[0].strip().rstrip().split()[2])
				else:
					assert False
			else:
				time = float(filter(lambda x:x.find("##MEASUREMENT:")>=0, out.split("\n"))[0].split()[-1])
		except ValueError:
			time = float('nan')
		time_list.append(time)
	return time_list

def main():
	global g_vars

	parser = argparse.ArgumentParser()
	parser.add_argument("applications", nargs='?', default=None, help="the name list of the applications (separated by ,), or \"all\" for all the available applications")
	parser.add_argument("-l", "--list", action='store_true', help="list all available applications")
	parser.add_argument("-i", "--iterations", type=int, default=7, help="the number of times of running before averaging")
	parser.add_argument("--verbose", action='store_true', help="List every step")
	args = parser.parse_args()

	g_vars["verbose"] = args.verbose

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

	# resolve other config information
	
	config = ConfigParser.ConfigParser()
	config.optionxform = str
	config.read(os.path.join(os.environ["TSX_ROOT"], "run.conf"))
	if not config.has_section("running"):
		print("Error: The file run.conf is not correctly configured.")
		return

	if not config.has_option("running", "num_threads"):
		print("Error: No num_threads in run.conf")
		return
	g_vars["num_threads"] = config.get("running", "num_threads")
	if not config.has_option("running", "cpu_list"):
		print("Error: No cpu_list in run.conf")
		return
	g_vars["cpu_list"] = config.get("running", "cpu_list")


	for name in names:
		origin_time = measure_app(name, apps[name]["original"], args.iterations)
		
		opt_time = measure_app(name, apps[name]["optimized"], args.iterations) 
		
		origin_avg = average(origin_time)
		opt_avg = average(opt_time)
		
		if name in g_vars["throughput_applications"]:
			speedup = opt_avg / origin_avg
			print("{}  origin:{}(throughput)  opt:{}(throughput)  speedup:{}X".format(name, round(origin_avg,2), round(opt_avg,2), round(speedup,2)))
		else:
			speedup = origin_avg / opt_avg
			print("{}  origin:{}s  opt:{}s  speedup:{}X".format(name, round(origin_avg,2), round(opt_avg,2), round(speedup,2)))
	

main()
