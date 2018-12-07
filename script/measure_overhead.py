#!/usr/bin/env python2
import argparse
import ConfigParser
import collections
import csv
import math
import numpy as np
import os
import plot
import shlex
import subprocess

g_vars = {}
native_list = []
native_err_list = []
txsampler_list = []
txsampler_err_list = []


def execute_command(command):
	if g_vars["verbose"]:
		print("EXECUTE: {}".format(command))
	child = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
	out, err = child.communicate()
	return out,err

def read_info_file(file_path):
	if g_vars["verbose"]:
		print("Start to read {}".format(file_path))
	apps = collections.OrderedDict()
	with open(file_path) as f:
		reader = csv.DictReader(f)
		for row in reader:
			name = row["name"].lower()
			apps[name] = {}
			config = apps[name]
			for field in ["compile_path", "compile_command", "run_path", "run_file"]:
				config[field] = row[field]
	return apps

def average_std(values):
	values = filter(lambda x: not math.isnan(x), values)
	if len(values) >= 7:
		values.sort()
		values = values[1:-1]
	return np.mean(values), np.std(values)

def measure_time(config, iterations, is_native):
	time_list = []
	if is_native:
		cmd = "run-benchmark.py -r native -t {} -c {} {} --show-output".format(g_vars["num_threads"], g_vars["cpu_list"], config["run_file"])
	else:
		cmd = "run-benchmark.py -r txsampler -t {} -c {} {} --show-output".format(g_vars["num_threads"], g_vars["cpu_list"], config["run_file"])
	for _ in range(iterations):
		out, err = execute_command(cmd)
		try:
			#print(out)
			#print(err)
			time = float(filter(lambda x:x.find("##MEASUREMENT:")>=0, out.split("\n"))[0].split()[-1])
		except ValueError:
			time = float('nan')
		time_list.append(time)
	return time_list

def measure_app(name, config, iterations):
	time_list = []

	# compile
	working_dir = os.path.join(os.environ["TSX_ROOT"], "benchmark", config["compile_path"])
	if g_vars["verbose"]:
		print("Change working directory to {}".format(working_dir))
	os.chdir(working_dir)
	print(config["compile_command"])
	execute_command(config["compile_command"])
	
	# run
	working_dir = os.path.join(os.environ["TSX_ROOT"], "benchmark", config["run_path"])
	if g_vars["verbose"]:
		print("Change working directory to {}".format(working_dir))
	os.chdir(working_dir)

	native_times = measure_time(config, iterations, True)
	txsampler_times = measure_time(config, iterations, False)

	native_avg, native_std = average_std(native_times)
	txsampler_avg, txsampler_std = average_std(txsampler_times)
	overhead = txsampler_avg / native_avg

	print("{}  native(average:{}, std:{})  txsampler(average:{}, std:{})  overhead:{}X".format(name, round(native_avg,2), round(native_std,2), round(txsampler_avg,2), round(txsampler_std,2), round(overhead,2)))

	global native_list
	global native_err_list
	global txsampler_list
	global txsampler_err_list

	native_list.append(native_avg)
	native_err_list.append(native_std)
	txsampler_list.append(txsampler_avg)
	txsampler_err_list.append(txsampler_std)

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
	apps = read_info_file(os.path.join(os.path.dirname(os.path.realpath(__file__)), "overhead-list.csv"))

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
		measure_app(name, apps[name], args.iterations)
		
	assert(len(names) == len(native_list))
	assert(len(names) == len(native_err_list))
	assert(len(names) == len(txsampler_list))
	assert(len(names) == len(txsampler_err_list))
	plot.plot_overhead_figure("overhead", names, native_list, native_err_list, txsampler_list, txsampler_err_list)

		
main()
