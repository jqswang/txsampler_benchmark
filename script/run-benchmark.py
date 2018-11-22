#!/usr/bin/env python2
import ConfigParser
import os
import shlex
import subprocess
import sys
from optparse import OptionParser

g_vars = {}

g_vars["event_code_map"] = {"1":("cycles:precise=2", 1000000),"2": ("RTM_RETIRED:ABORTED",10000),"3": ("RTM_RETIRED:COMMIT",3000 ),"4": ("MEM_UOPS_RETIRED:ALL_STORES", 100000)}

def log_print(s):
	if g_vars["verbose"]:
		print(s)

def execute_command(command, shell_flag=False):
        log_print("EXECUTE: {}".format(command))
	if g_vars["dry_run"]:
		return "", "", 0
	out, err = subprocess.Popen(shlex.split(command), shell=shell_flag,stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()
	return out, err

def time_execute(prefix_cmd, main_cmd, shell_flag = False):
	time_file_name = "/tmp/measured_time.tmp"
	time_cmd = "/usr/bin/time -f %e -o "+time_file_name
	if g_vars["use_script"]:
		os.environ["TXSAMPLER_CMD"] = time_cmd + " " + prefix_cmd
		cmd = main_cmd
	else:
		cmd = " ".join([time_cmd,prefix_cmd,main_cmd])
	try:
		os.remove(time_file_name)
	except OSError:
		pass
	out,err = execute_command(cmd, shell_flag)
	with open(time_file_name) as f:
		time = f.readlines()[0].strip().rstrip()
	#os.remove(time_file_name)
	print(time)
	return out, err, float(time)

def parse_arguments():
        global g_vars
        usage = "usage: %prog [options] [run_file,default.run]"
        parser = OptionParser(usage)
        parser.add_option("-i", type="int", dest="iteration", default=1, help="The number of iterations.")
        parser.add_option("-r", type="string", dest="run", default="none", help="Available option native|txsampler")
        parser.add_option("-e", type="string", dest="events", default="all", help="What events to sample. 1(cycles),2(aborts),3(commits),4(stores), all(all-above). E.g. 1,2,3,4")
        parser.add_option("--show-output", action="store_true", dest="show_output", default=False, help="Whether to show the launched program's output")
        parser.add_option("--verbose", action="store_true", dest="verbose", default=False, help="Print every command executed")
	parser.add_option("-t", type="int", dest="num_threads", default=1, help="The number of threads to launch the program if configurable (not applicable to PARSEC)")
	parser.add_option("-c", type="string", dest="cpu_list", default="0", help="The CPUs the program is bound to. It is similar to --cpu-list in taskset")
	parser.add_option("--dry-run", action="store_true", dest="dry_run", default=False, help="Force dry run")

	(options, args) =parser.parse_args()
	if len(args) < 1:
		g_vars["run_file"] = "default.run"
	else:
		g_vars["run_file"] = args[0]

	g_vars["iterations"] = options.iteration
	g_vars["run_type"] = options.run
	g_vars["event_code"] = options.events
	g_vars["show_output"] = options.show_output
	g_vars["verbose"] = options.verbose or options.dry_run
	g_vars["num_threads"] = options.num_threads
	g_vars["cpu_list"] = options.cpu_list
	g_vars["dry_run"] = options.dry_run

def strip_quotes(s):
	if s.startswith('"') and s.endswith('"'):
		s = s[1:-1]
	return s

def fill_environment_variables(s):
	'''Replace the variables within $$'''
	segments = s.split("$")
	assert len(segments)%2 == 1
	for i in [ 2*x+1 for x in range(0, (len(segments)-1)/2)]:
		assert segments[i] in os.environ
		segments[i] = os.environ[segments[i]]
	return "".join(segments)

def parse_run_file():
	config = ConfigParser.ConfigParser()
	config.optionxform = str
	config.read(g_vars["run_file"])

	if config.has_section("environment"):
		g_vars["environ_list"] = config.items('environment')
	
	g_vars["exe"] = fill_environment_variables(config.get("information", "exe"))
	if config.has_option("information", "launch_script"):
		g_vars["launch_script"] = fill_environment_variables(config.get("information", "launch_script"))
		g_vars["launch_script"] = strip_quotes(g_vars["launch_script"])
		g_vars["use_script"] = True
	else:
		g_vars["use_script"] = False

	g_vars["app_shown_name"] = config.get("information", "shown_name")
	g_vars["args"] = fill_environment_variables(strip_quotes(config.get("information", "arguments")))

	g_vars["sampling_dict"] = {}
	if config.has_section("sampling"):
		for (key, value) in config.items("sampling"):
			key = key.replace("?",":")
			key = key.replace("--", "=")
			g_vars["sampling_dict"][key] = value
	
def native_run(prefix_cmd, main_cmd):
	for i in range(1, g_vars["iterations"]+1):
		out, err, time = time_execute(prefix_cmd, main_cmd)
		if g_vars["show_output"]:
			print(out)
			print(err)
		print("##MEASUREMENT: native {} {} {}".format(g_vars["app_shown_name"], i, round(time,2)))
			
def get_event_list(event_code):
	ret_list = []
	event_code = event_code.replace("all","1,2,3,4")
	for e in event_code.split(","):
		assert e in g_vars["event_code_map"]
		ret_list.append(g_vars["event_code_map"][e])
	return ret_list	
			

def txsampler_run(prefix_cmd, main_cmd):
	event_list = get_event_list(g_vars["event_code"])
	if "sampling_dict" in g_vars:
		new_event_list = []
		for name,interval in event_list:
			if name in g_vars["sampling_dict"]:
				interval = int(g_vars["sampling_dict"][name])
			new_event_list.append((name, interval))
		event_list = new_event_list
	event_str = " ".join(["-e " + name + "@" + str(interval) for name,interval in event_list])

	for i in range(1, g_vars["iterations"]+1):
		exe = g_vars["exe"]
		exe_base = os.path.basename(exe)
		log_print("Cleaning *.hpcstruct hpctoolkit-*")
		subprocess.Popen("%s %s" % ("rm", "-rf *.hpcstruct hpctoolkit-*"),shell=True).wait()
		out, err = execute_command("hpcstruct "+ exe)
		if out and g_vars["show_output"]:
			print(out)
		if err and g_vars["show_output"]:
			print(err)
		measurement_folder = "hpctoolkit-"+ exe_base+"-measurements"
		out, err, time = time_execute(" ".join([prefix_cmd, "hpcrun", event_str]), main_cmd)
		if out and g_vars["show_output"]:
			print(out)
		if err and g_vars["show_output"]:
			print(err)
		out, err = execute_command("hpcprof-mpi -o hpctoolkit-"+ exe_base+"-database -S ./"+exe_base+".hpcstruct -I ../ "+ measurement_folder)
                if out and g_vars["show_output"]:
                        print(out)
                if err and g_vars["show_output"]:
                        print(err)
		print("##MEASUREMENT: txsampler {} {} {}".format(g_vars["app_shown_name"], i, round(time,2)))

def main():
	parse_arguments()

	# set environmental variables
	os.environ["THREADS"] = str(g_vars["num_threads"])
	os.environ['OMP_WAIT_POLICY'] = "ACTIVE"
	os.environ['OMP_NUM_THREADS'] = str(g_vars["num_threads"])

	os.environ["HTM_TRETRY"] = "5"
	os.environ["HTM_PRETRY"] = "1"
	os.environ["HTM_GRETRY"] = "5"

	parse_run_file()

	if "environ_list" in g_vars:
		for (key, value) in g_vars["environ_list"]:
			os.environ[key] = fill_environment_variables(value)

	# construct running cmd
        if g_vars["use_script"]:
		main_cmd = g_vars["launch_script"]
        else:
                main_cmd = g_vars["exe"]

        if not os.path.isabs(main_cmd):
		main_cmd = os.path.join(os.getcwd(),main_cmd)
	if not g_vars["use_script"]:
		main_cmd = main_cmd+ " "+g_vars["args"]

	# construct prefix
	prefix_cmd = "taskset -c " + g_vars["cpu_list"]

	# native run
	if g_vars["run_type"] == "native":
		native_run(prefix_cmd, main_cmd)
	# txsampler run
	if g_vars["run_type"] == "txsampler":
		txsampler_run(prefix_cmd, main_cmd)


main()
