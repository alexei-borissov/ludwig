# Script for creating ASCII data file with io_sort.c
# and creating vtk-data files for visualization

# input as follows:
# size; NOTE: correct system size has to be also given in io_sort.c 
# type: 1 scalar, 2 vector
# no of files in output grouping
# selected columns for output into vtk-file starting with 1
# filelist name (default filelist)

import sys, os, re, math
Lx=128
Ly=128
Lz=32
ngroup=1

op=0
vel=0
dir=1

create_data_file=1
create_vtk_file=1

# collect and set lists for analysis
type=[]
x=[]
y=[]
z=[]
metafile=[]
filelist=[]

if op==1:
	type.append('1')
	x.append('12')
	y.append('0')
	z.append('0')
	filelist.append('filelist_op')
	os.system('ls -t1 order_velo.*.dat-0-%d > filelist_op' % ngroup)

if vel==1:
	type.append('2')
	x.append('9')
	y.append('10')
	z.append('11')
	filelist.append('filelist_vel')
	os.system('ls -t1 order_velo.*.dat-0-%d > filelist_vel' % ngroup)

if dir==1:
	type.append('3')
	x.append('4')
	y.append('5')
	z.append('6')
	filelist.append('filelist_dir')
	os.system('ls -t1 dir.*.dat-0-%d > filelist_dir' % ngroup)

os.system('gcc -o io_sort io_sort.c -lm')

# create ASCII datafile
if create_data_file==1:
	for i in range(len(type)):

		datafiles=open(filelist[i],'r') 

		print('# creating datafiles')

		while 1:
			line=datafiles.readline()
			if not line: break

			print '# processing %s' % line 

			stub=line.split('-',1)
		        filetype=stub[0].split('.',1)

			if filetype[0] == 'order_velo':
				antype=2
				cmd = './io_sort %d %d %s' % (antype,ngroup,stub[0])
				os.system(cmd)

			if filetype[0] == 'dir':
				antype=4
				cmd = './io_sort %d %d %s' % (antype,ngroup,stub[0])
				os.system(cmd)

		datafiles.close

if create_vtk_file==1:

	# create datafile list
	if op==1:
		os.system('ls -t1 order_velo.*.dat > filelist_op')
	if vel==1:
		os.system('ls -t1 order_velo.*.dat > filelist_vel')
	if dir==1:
		os.system('ls -t1 dir.*.dat > filelist_dir')

	# create vtk-header
	for i in range(len(type)):

		x[i]=int(x[i])-1
		y[i]=int(y[i])-1
		z[i]=int(z[i])-1

		headerlines=[]
		headerlines.append('# vtk DataFile Version 2.0')
		headerlines.append('Generated by create_vtk_file')
		headerlines.append('ASCII')
		headerlines.append('DATASET STRUCTURED_POINTS')
		headerlines.append('DIMENSIONS  %d %d %d' %(Lx,Ly,Lz))
		headerlines.append('ORIGIN 0 0 0')
		headerlines.append('SPACING 1 1 1')
		headerlines.append('POINT_DATA %d' %(Lx*Ly*Lz))
		if type[i]=='1':
			headerlines.append('SCALARS scalar%d float 1' %i)
			headerlines.append('LOOKUP_TABLE default')
		if type[i]=='2':
			headerlines.append('VECTORS velocity float')
		if type[i]=='3':
			headerlines.append('VECTORS director float')


		print('# creating vtk-files')

		# inputfiles
		datafilenames=open(filelist[i],'r')

		while 1:
			line=datafilenames.readline()
			if not line: break

			linestring=line.split()
			datafilename=linestring[0]

			if type[i]=='1':
				outputfilename= datafilename + '-op.vtk'

			if type[i]=='2':
				outputfilename= datafilename + '-velo.vtk'

			if type[i]=='3':
				outputfilename= datafilename + '-dir.vtk'

			print '# processing %s' % outputfilename

			file=open(datafilename,'r')
			out=open(outputfilename,'w')

			dataline=[]
			data=[]

			# write header
			for j in range(len(headerlines)):
				out.write('%s\n' % headerlines[j]) 

			while 1:

			     	line=file.readline()
				if not line: break

				datastring=line.split()

				if type[i]=='1':
					xdata=float(datastring[x[i]])
					out.write('%.5le\n' % xdata)
				if type[i]=='2':
					xdata=float(datastring[x[i]])
					ydata=float(datastring[y[i]])
					zdata=float(datastring[z[i]])
					out.write('%.5le %.5le %.5le\n' % (xdata,ydata,zdata))
				if type[i]=='3':
					xdata=float(datastring[x[i]])
					ydata=float(datastring[y[i]])
					zdata=float(datastring[z[i]])
					out.write('%.5le %.5le %.5le\n' % (xdata,ydata,zdata))

	       		out.close
			file.close

		datafilenames.close

print('# done')
