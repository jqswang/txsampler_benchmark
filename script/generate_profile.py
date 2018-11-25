#!/usr/bin/env python2
import argparse
import ConfigParser
import csv
import os
import shlex
import subprocess

g_vars = {}

def execute_command(command):
	if g_vars["verbose"]:
		print("EXECUTE: {}".format(command))
	out, err = subprocess.Popen(shlex.split(command), stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
	return out,err


def read_info_file(file_path):
	if g_vars["verbose"]:
		print("Start to read {}".format(file_path))
	apps = {}
	with open(file_path) as f:
		reader = csv.DictReader(f)
		for row in reader:
			name = row["name"].lower()
			apps[name] = {}
			config = apps[name]
			for field in ["compile_path", "compile_command", "run_path", "run_file"]:
				config[field] = row[field]
	return apps

def run_application(name, config):
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
	out, err = execute_command("run-benchmark.py -r txsampler --show-output --verbose -t {} -c {} {}".format(g_vars["num_threads"], g_vars["cpu_list"], config["run_file"]))

	#print(out)
	#print(err)	
	lines = out.split("\n")
	try:
		time = float(filter(lambda x:x.find("##MEASUREMENT:")>=0, out.split("\n"))[0].split()[-1])
	except ValueError:
		time = float('nan')
	
	print("Running {} takes {}s".format(name, round(time,2)))
	try:
		database_dir = filter(lambda x:x.find("msg: Populating Experiment database:")>=0, lines)[0].split()[-1]
		print("Profile database of {} generated: {}".format(name, database_dir))
	except IndexError:
		print("Failed to generate the profile database of {}".format(name))

def main():
	global g_vars

	parser = argparse.ArgumentParser()
	parser.add_argument("applications", nargs='?', default=None, help="the name list of the applications (separated by ,), or \"all\" for all the available applications")
	parser.add_argument("-l", "--list", action='store_true', help="list all available applications")
	parser.add_argument("--verbose", action='store_true', help="List every step")
	args = parser.parse_args()
	

	g_vars["verbose"] = args.verbose

	# read info file
	apps = read_info_file(os.path.join(os.path.dirname(os.path.realpath(__file__)), "casestudy-list.csv"))
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
		run_application(name, apps[name])
	
main()
