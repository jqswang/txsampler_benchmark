#!/usr/bin/env python2
# It is adopted from measure_benchmark.py, but it is used to try the best value of TXN_GRAN_1 and TXN_GRAN_2

import sys, os
import ConfigParser
import shlex, subprocess
import numpy as np
import copy
from optparse import OptionParser

def get_global_variables():
	ret_dict = dict()
	ret_dict["number_of_time_measurement"] = 7
	#ret_dict["core_setting_same"] = {1: "3", 2: "3,7", 4: "3,7,11,15", 8: "3,7,11,15,19,23,27,31", 12:"3,7,11,15,19,23,27,31,35,39", 14:"3,7,11,15,19,23,27,31,35,39,43,47"}
	ret_dict["core_setting_same"] = {14:"3,7,11,15,19,23,27,31,35,39,43,47,51,55", 12:"3,7,11,15,19,23,27,31,35,39,43,47", 1:"47"}
	ret_dict["core_setting_diff"] = { 2: "24,36", 4: "24,25,36,37", 8: "24,25,26,27,36,37,38,39", 12:"24-29,36-41", 16:"24-31,36-43", 20:"24-33,36-45", 24:"24-47"}
	ret_dict["core_setting_smt"] = {2:"12,36", 4:"12-13,36-37", 8:"12-15,36-39", 12:"12-17,36-41", 16:"12-19,36-43", 20:"12-21,36-45",24:"12-23,36-47"}
	ret_dict["time_measure_tool"] = "measure_time_once.sh"
	ret_dict["input_root"] = "/home/qwang/tsx/tsx_input"

	#ret_dict["environ_list"] = {"HTM_TRETRY":"5", "HTM_PRETRY":"1","HTM_GRETRY":"5"}
	#ret_dict["environ_list"] = {"HTM_TRETRY":"16", "HTM_PRETRY":"1","HTM_GRETRY":"16"}
	return ret_dict
g_vars = get_global_variables()


#####You can tune something here
g_vars["num_threads"] = 14
g_vars["current_core_setting"] = g_vars["core_setting_same"][g_vars["num_threads"]]
g_vars["event_code_map"] = {"1":("cycles:precise=2", 1000000),"2": ("RTM_RETIRED:ABORTED",10000),"3": ("RTM_RETIRED:COMMIT",3000 ),"4": ("MEM_UOPS_RETIRED:ALL_STORES", 100000)}
###################################################

def MYPRINT(input_str):
	if g_vars["verbose_flag"]:
		print(input_str)

def execute_commands(command, shell_flag=False):
	MYPRINT(command)
	command_splits = shlex.split(command)
	return subprocess.Popen(command_splits, shell=shell_flag,stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()

class Output_T:
	def __init__(self, name):
		self._name = name
		self._iterations = []
		self._event_list = []
		self._log = []
		self._time = []
	def _formatEventList(self,event_list,opt=1):
		ret_list  = []
		if opt == 1:
			for event,interval in event_list:
				ret_list.append(event)
				ret_list.append(str(interval))
			return ",".join(ret_list)
		if opt == 2:
			for event,interval in event_list:
				ret_list.append(str(interval))
			return " ".join(ret_list) 
	def _getNumSamples(self,log):
		for line in log.split("\n"):
			if line[0:7] == "SUMMARY":
				return line.split()[2].strip().rstrip()	
                return None	

	def feed(self, iteration, event_list, log, time):
		self._iterations.append(str(iteration))
		self._event_list.append(event_list)
		self._log.append(log)
		self._time.append(time)
	def outputall(self, level):
		lines = []
		if level == 3:
			for i in range(0, len(self._time)):
				one_line = " ".join([self._name, self._iterations[i], self._formatEventList(self._event_list[i]), self._getNumSamples(self._log[i]), self._time[i]])
				lines.append(one_line)
		if level == 2:
			for i in range(0, len(self._time)):
				one_line = " ".join([self._name, self._iterations[i], self._formatEventList(self._event_list[i],2), self._getNumSamples(self._log[i]), self._time[i]])
				lines.append(one_line)
		lines.append(" ".join([self._name]+ self._time))
		return "\n".join(lines)


def parse_arguments():
	global g_vars
	usage = "usage: %prog [options] [run_file,default.run]"
	parser = OptionParser(usage)
	parser.add_option("-i", type="int", dest="iteration", default=1, help="The number of iterations.")
	parser.add_option("-r", type="string", dest="run", default="none", help="Available option none|hpcrun")
	parser.add_option("-e", type="string", dest="events", default="all", help="What events to sample. 1(cycles),2(aborts),3(commits),4(stores), all(all-above). E.g. [1,2],3,4")
	parser.add_option("-p", type="int", dest="print_level", default="1", help="How much information it should print. 1(time), 2(time+sampling_rate), 3(time+sampling_rate+events)")
	parser.add_option("--show-output", action="store_true", dest="show_output", default=False, help="Whether to show the target program's output")
	parser.add_option("--add-retcnt", action="store_true", dest="retcnt_flag", default=False, help="Added \"-e RETCNT\" for every hpcrun")
	parser.add_option("--verbose", action="store_true", dest="verbose", default=False, help="Print every command executed")
	parser.add_option("--no-hpcprof", action="store_false", dest="hpcprof_flag", default=True, help="After ruuning hpcrun, no need to run hpcprof")


	(options, args) =parser.parse_args()
	if len(args) < 1:
		g_vars["run_file"] = "default.run"
	else:
		g_vars["run_file"] = args[0]
		print(args)
	g_vars["number_of_time_measurement"] = options.iteration
	g_vars["run_type"] = options.run
	g_vars["event_code"] = options.events
	g_vars["print_level"] = options.print_level
	g_vars["show_output"] = options.show_output
	g_vars["retcnt_flag"] = options.retcnt_flag
	g_vars["verbose_flag"] = options.verbose
	g_vars["hpcprof_flag"] = options.hpcprof_flag

def strip_quotes(one_str):
	if one_str.startswith('"') and one_str.endswith('"'):
		one_str = one_str[1:-1]
	return one_str

def fill_environment_variables(one_str):
	'''Target anything between $$'''
	segments = one_str.split("$")
	assert len(segments)%2 == 1
	iterations = [ 2*x+1 for x in range(0, (len(segments)-1)/2)]
	for i in iterations:
		assert segments[i] in os.environ
		segments[i] = os.environ[segments[i]]
	return "".join(segments)

def parse_run_file():
	Config = ConfigParser.ConfigParser()
	Config.optionxform = str
	Config.read(g_vars["run_file"])

	if Config.has_section("environment"):
	        g_vars["environ_list"] = Config.items('environment')


	g_vars["exe"] = fill_environment_variables(Config.get("information", "exe"))
	if Config.has_option("information", "launch_script"):
		g_vars["launch_script"] = fill_environment_variables(Config.get("information", "launch_script"))
		g_vars["launch_script"] = strip_quotes(g_vars["launch_script"])
		g_vars["script_flag"] = True
	else:
		g_vars["script_flag"] = False

	g_vars["app_shown_name"] = Config.get("information", "shown_name")
	g_vars["args"] = fill_environment_variables(strip_quotes(Config.get("information", "arguments")))
	g_vars["parallel"] = Config.get("information", "parallel")

	g_vars["sampling_dict"] = dict()

	if Config.has_section("sampling"):
		for (key, value) in Config.items("sampling"):
			key = key.replace("?",":")
			key = key.replace("--", "=")
			if key == "PAPI_TOT_CYC":
				key = "cycles:precise=2"
			g_vars["sampling_dict"][key]=value

def time_execute(prefix, cmd, shell_flag=False):
	time_file_name = "measure_time.tmp"
	time_cmd = "/usr/bin/time -f %e -a -o "+time_file_name
	if g_vars["script_flag"]:
		os.environ["HPCRUN_CMD"] = time_cmd + " " + prefix 
	else:
		cmd = " ".join([time_cmd,prefix,cmd])
	try:
		os.remove(time_file_name)
	except OSError:
		pass
	out,err = execute_commands(cmd, shell_flag)
	#time,err = execute_commands("cat "+ time_file_name, False)
	#time = time.split("\n")[0].strip().rstrip()
	#print(time)
	with open(time_file_name) as f:
		time = f.readlines()[0].strip().rstrip()
	os.remove(time_file_name)
	return out,err,time


def none_run(prefix_cmd, main_cmd):
	time_array = []
	#for i in range(0, g_vars["number_of_time_measurement"]):
	for i in range(40, 61):
		os.environ["TXN_GRAN_1"] = "1"
		os.environ["TXN_GRAN_2"] = str(i)
		output,err,time = time_execute(prefix_cmd, main_cmd)	
		time_array.append(float(time))
		print(os.environ["TXN_GRAN_1"] + " " + os.environ["TXN_GRAN_2"]+" " + str(time))
		if g_vars["show_output"]:
			print(output)
			print(err)
	print(" ".join(["none-run",g_vars["app_shown_name"]]+map(str,time_array)))


def execute_hpcrun(event_one_time_list, prefix, cmd, measure_folder,iteration):
	event_statement = ""
	events = []
	for event, interval in event_one_time_list:
		events.append(event)
		event_statement = event_statement + " -e "+event+"@"+str(interval)
	if g_vars["retcnt_flag"]:
		event_statement = event_statement+" -e RETCNT"
	prefix = prefix + " hpcrun " + event_statement
	output,err,time = time_execute(prefix,cmd)
	if g_vars["show_output"]:
		print(output)
		print(err)
	log_output= subprocess.Popen("%s %s" % ("cat", measure_folder+"/*.log"),shell=True ,stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]
	ret_dict = {"output":output,"err":err,"time":time,"log":log_output, "events":"-".join(events)}
	with open("info.txt", "w") as f:
		f.write("###events###\n")
		f.write(str(event_one_time_list))
		f.write("\n###time###\n")
		f.write(time)
		f.write("\n###log###\n")
		f.write(log_output)
		f.write("\n###output###\n")
		f.write(output)
		f.write("\n###error###\n")
		f.write(err)
	subprocess.Popen("%s %s" % ("mv", "info.txt " + measure_folder +"/"),shell=True).wait()
	g_output.feed(iteration, event_one_time_list, log_output, time)
	print("time: "+str(time))
	return ret_dict

def execute_one_hpcprof(event_list, prefix,cmd,save_dir,iteration):
	exe = g_vars["exe"]
	exe_base = os.path.basename(exe)
	##clean
	subprocess.Popen("%s %s" % ("rm", "-rf *.hpcstruct hpctoolkit-*"),shell=True).wait()
	execute_commands("hpcstruct "+ exe)
	measure_folder = "hpctoolkit-"+ exe_base+"-measurements"
	measure_folder_list = []
	ret_list = []
	for one_time_list in event_list:
		ret_dict = execute_hpcrun(one_time_list, prefix, cmd,measure_folder, iteration)
		subprocess.Popen("%s %s" % ("mv", measure_folder+" "+measure_folder+"-"+ret_dict["events"]), shell=True).wait()
		measure_folder_list.append(measure_folder+"-"+ret_dict["events"])
		ret_list.append(ret_dict) 
	if g_vars["hpcprof_flag"]:
		execute_commands("hpcprof-mpi -o hpctoolkit-"+ exe_base+"-database -S ./"+exe_base+".hpcstruct -I ../ "+ " ".join(measure_folder_list))
	subprocess.Popen("%s %s" % ("mkdir", "-p " + save_dir),shell=True).wait()
	subprocess.Popen("%s %s" % ("mv", "hpctoolkit-* " + save_dir +"/"),shell=True).wait()
	subprocess.Popen("%s %s" % ("mv", "*.hpcstruct " + save_dir +"/"),shell=True).wait()
	subprocess.Popen("%s %s" % ("cp", exe+ " " + save_dir +"/"),shell=True).wait()	
	subprocess.Popen("%s %s" % ("cp", g_vars["run_file"]+ " " + save_dir +"/"),shell=True).wait()
	return ret_list


def get_one_time_list(code):
	ret_list = []		 
	if code.startswith("[") and code.endswith("]"):
		code = code[1:-1]
	if code == "all":
		code = ",".join(sorted(list(g_vars["event_code_map"].keys())))
	for c in code.split(","):
		ret_list.append(g_vars["event_code_map"][c])
	return ret_list
	
def get_propose_even_list():
	codes = g_vars["event_code"].split(",")	
	event_list = []
	if "all" in codes:
		codes = sorted(list(g_vars["event_code_map"].keys()))
	for c in codes:
		event_list.append(get_one_time_list(c))
	return event_list

def hpcrun_run(prefix_cmd, main_cmd):
	propose_event_list = get_propose_even_list()
	interval_str = " "
	if "sampling_dict" in g_vars:
		event_list = []
		for one_list in propose_event_list:
			internal_list = []
			for event,interval in one_list:
				if event in g_vars["sampling_dict"]:
					interval = int(g_vars["sampling_dict"][event])
				internal_list.append((event, interval))
				interval_str = interval_str + " " + str(interval)
			event_list.append(internal_list)
	else:
		event_list = propose_event_list
	#print(event_list)
	hpcdata_save_dir_root = os.path.join(os.getcwd(),g_vars["app_shown_name"]+"-hpcrun")
	subprocess.Popen("%s %s" % ("rm", "-rf " + hpcdata_save_dir_root ),shell=True).wait()
	for i in range(0,g_vars["number_of_time_measurement"]):
		##execute hpctoolkit
		hpcdata_save_dir = os.path.join(hpcdata_save_dir_root, str(i))
		ret_list = execute_one_hpcprof(event_list, prefix_cmd, main_cmd,hpcdata_save_dir,i)

	print(g_output.outputall(g_vars["print_level"]))
	return

def main():
	parse_arguments()


	##set environmental variables
	#os.environ['PATH'] =  g_vars["measurement_tool_path"] + ":"+os.environ['PATH']
	os.environ["THREADS"] = str(g_vars["num_threads"])
	os.environ["INPUT_ROOT"] = g_vars["input_root"]
        os.environ['OMP_WAIT_POLICY'] = "ACTIVE"
        os.environ['OMP_NUM_THREADS'] = str(g_vars["num_threads"])

	os.environ["HTM_TRETRY"] = "5"
	os.environ["HTM_PRETRY"] = "1"
	os.environ["HTM_GRETRY"] = "5"	


	parse_run_file()

	if "environ_list" in g_vars:
		print(g_vars["environ_list"])
		for (key, value) in g_vars["environ_list"]:
			os.environ[key] = fill_environment_variables(value)

	global g_output
	g_output = Output_T(g_vars["app_shown_name"])

	
	##construct running cmds
	if g_vars["script_flag"]:
		main_cmd = g_vars["launch_script"]	
	else:
		main_cmd = g_vars["exe"]

	if not os.path.isabs(main_cmd):
		main_cmd = os.path.join(os.getcwd(),main_cmd)
	if not g_vars["script_flag"]:
		main_cmd = main_cmd+ " "+g_vars["args"]

	##prefix	
	prefix_cmd = ""
	if g_vars["parallel"] == "omp":
		os.environ['GOMP_CPU_AFFINITY'] = g_vars["current_core_setting"]
	else:
		prefix_cmd = prefix_cmd + "numactl --physcpubind=" + g_vars["current_core_setting"]


	#none run
	if g_vars["run_type"] == "none":
		none_run(prefix_cmd, main_cmd)		


	#hpcrun run
	if g_vars["run_type"] == "hpcrun":	
		hpcrun_run(prefix_cmd, main_cmd)

	###use vtune
	#output =execute_vtune("tsx-hotspots", g_vars, args, numa_policy)


#def execute_vtune(analysis_type, g_vars, args, suffix=""):
#	exe = g_vars["exe"]
#	if suffix !="":
#		suffix= suffix+" "
#	return execute_commands(suffix+"amplxe-cl -collect "+analysis_type+" -- ./"+exe+" " + args)


main()

