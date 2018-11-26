import matplotlib.pyplot as plt
import matplotlib.style as style
import numpy as np


def geo_mean(one_array):
	ret_value = 1
	i = 0
	for number in one_array:
		if number != 0:
			i = i+1
			ret_value = ret_value * number
	return ret_value ** (1 / i)

def plot_overhead_figure(output_file, name_list, native_list, native_err_list, monitored_list, monitored_err_list):
	width = 0.3	

	## normalization
	native_err_list = [ float(a)/b for a,b in zip(native_err_list, native_list)]
	monitored_list = [ float(a)/b for a,b in zip(monitored_list, native_list)]
	monitored_err_list = [ float(a)/b for a,b in zip(monitored_err_list, native_list)]
	native_list = [ 1.0 for _ in native_list]

	
	style.use('seaborn-white')
	fig = plt.figure()

	ax = fig.add_subplot(1,1,1)

	x_row = np.arange(len(name_list)+2)
	plot_list = []


	native_list.append(0)
	native_err_list.append(0)
	monitored_list.append(0)
	monitored_err_list.append(0)

	native_list.append(1)
	native_err_list.append(0)
	monitored_list.append(geo_mean(monitored_list))
	monitored_err_list.append(0)

	plot_list.append(plt.bar(x_row + 0*width, native_list, width, align='edge', yerr=native_err_list, ecolor='red', edgecolor='black', color='#545948', hatch="|"))
	plot_list.append(plt.bar(x_row + 1*width, monitored_list, width, align='edge', yerr=monitored_err_list, ecolor='blue', edgecolor='black', color='white', hatch="/"))

	#print(native_list)
	#print(monitored_list)

	ax.set_xlim(-0.1, len(x_row)-0.2)
	ax.set_xticks(x_row+ width)
	ax.set_xticklabels(name_list+["","geomean"], rotation=40,fontsize=12)
	ax.set_ylim(0.7, 1.3)
	plt.yticks(np.arange(0.7, 1.3, 0.1))
	ax.tick_params(axis='y', labelsize=14)
	plt.legend([p[0] for p in plot_list], ["native", "with sampling"], loc="upper center", ncol=2, fontsize=16)
		
	fig_size = plt.rcParams["figure.figsize"]
	fig_size[0] *= max(0.03*len(x_row), 1)
	fig_size[1] *= (1 - 0.3)
	fig.set_size_inches(fig_size[0], fig_size[1], forward = True)
	plt.subplots_adjust(bottom=0.25)
	fig.savefig(output_file+".pdf", format='pdf')

