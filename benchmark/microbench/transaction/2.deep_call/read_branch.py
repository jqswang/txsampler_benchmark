#!/usr/bin/env python
import sys
import shlex, subprocess

'''
This script can locate IPs into source lines. It is originally used to intepret the debugging information from LBR
'''

g_binary_file =""

def execute_commands(command):
	command_splits = shlex.split(command)
	return subprocess.Popen(command_splits, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()

def get_src_info(hex_string):
	info = execute_commands("addr2line -f -e " + g_binary_file + " " + hex_string)[0]  
	infos = info.strip().rstrip().split("\n")
	return infos[1]+"(" + infos[0]+")"

def process_type_a(line):
	split_line = line.split(" ")
	str = "#"
	for ip in split_line[1:]:
		str += " ["+ get_src_info(ip) + "]"
	print(str)

def process_type_b(line):
	split_line = line.split(" ")
	str = split_line[0] + "[" + get_src_info(split_line[0]) +"]"
	assert split_line[1]==":"

	for s in split_line[2:]:
		s1 = s.replace(">", "?").split("?")
		str += " "
		str += s1[0] + "[" + get_src_info(s1[0]) + "]"
		str += "?"
		str += s1[1] + "[" + get_src_info(s1[1]) + "]"
		str += "?"
		str += s1[2]
		str += "?"
		str += s1[3]
	print(str)
		
def process_line(line):
	if line.startswith("="):
		process_type_a(line)
	else:
		process_type_b(line)
		
def main():
	global g_binary_file
	if len(sys.argv) < 3:
		print("Invalid Input")
		print("Usage: " + sys.argv[0] +" [input_file] [binary_file]")
		return;
	
	g_binary_file = sys.argv[2]
	
	#read input file
	with open(sys.argv[1]) as f:
		for line in f.readlines():
			process_line(line.strip().rstrip())

main()