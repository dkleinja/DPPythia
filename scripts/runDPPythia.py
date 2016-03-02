#!/usr/bin/env python

import os
import sys
from datetime import datetime
from optparse import OptionParser

run_enable = True

## simple function to run a command
def runCmd(cmd):
    print cmd
    if run_enable:
        os.system(cmd)

## parse the command line controls
parser = OptionParser('Usage: %prog [options]')
parser.add_option('-t', '--template', type = 'string', dest = 'template', help = 'Input for the pythia command file gen', default = 'pythia_inclusive.cmnd')
parser.add_option('-o', '--output', type = 'string', dest = 'output', help = 'output file prefix', default = 'out_%s')
parser.add_option('-w', '--workdir', type = 'string', dest = 'workdir', help = 'Working dir of all temporary/permanent files', default = '')
parser.add_option('-n', '--nJobs', type = 'int', dest = 'nJobs', help = 'Number of jobs to make', default = 2)
parser.add_option('-s', '--seed', type = 'int', dest = 'seed', help = 'Seed offset of each job', default = 1)
#parser.add_option('-a', '--addition', type = 'string', dest = 'addition', help = 'additional scan instruction', default = '')
parser.add_option('-l', '--local', action = 'store_true', dest = 'local', help = 'Start running the jobs locally', default = False)
parser.add_option('-g', '--grid', action = 'store_true', dest = 'grid', help = 'Submit the jobs to grid', default = False)
(options, args) = parser.parse_args()

if len(sys.argv) < 2:
    parser.parse_args(['--help'])

if not os.path.exists(options.template):
    print 'Template file not found!'
    sys.exit()

if options.local and options.grid:
    print 'Cannot be in both local and grid mode!'
    sys.exit()

if options.workdir == '':
    options.workdir = os.getcwd()

## initializes the executable DPPythia
DPPythia = os.path.join(os.getenv('DPPYTHIA_ROOT'), 'DPPythia')

## set up the working directories
tempdir = os.path.abspath(options.template)
workdir = os.path.abspath(options.workdir)
outputdir = os.path.join(workdir, 'output')
logdir = os.path.join(workdir, 'log')
wrapperdir = os.path.join(workdir, 'wrapper') if options.grid else workdir

for d in (wrapperdir, outputdir, logdir):
    if not os.path.exists(d):
        os.makedirs(d)

## prepare the file names
logs = [os.path.join(logdir, '%s.log' % (options.output % str(i))) for i in range(options.nJobs)]
outputs = [os.path.join(outputdir, '%s.root' % (options.output % str(i))) for i in range(options.nJobs)]
wrappers = [os.path.join(wrapperdir, '%s.sh' % (options.output % str(i))) for i in range(options.nJobs)]

#for i, conf in enumerate(confs):
#    reservedVals = [eval(formula) for formula in reservedValFormula]
#    tconf.generate(conf, seed = options.seed + i, outputName = outputs[i], reserved = dict(zip(reservedKeys, reservedVals)))

## if in local mode, make everything locally in the background
if options.local:
    for i in range(len(logs)):
        cmd = '%s -t %s -o %s -s %d > %s &' % (DPPythia, tempdir, outputs[i], options.seed+i, logs[i])
        runCmd(cmd)

## if in grid mode, assume running on gpvm machines
if options.grid:

    # write wrapper files first
    for i in range(len(logs)):
        fout = open(wrappers[i], 'w')

        fout.write('echo\n')
        fout.write('echo --------------------------------------------------------------------\n')
        fout.write('echo Wrapper script: %s\n' % wrappers[i])
        fout.write('echo Wrapper script generated by %s\n' % ' '.join(sys.argv))
        fout.write('echo Timestamp: %s\n' % datetime.now())
        fout.write('echo Working directory: %s\n' % workdir)
        fout.write('echo Log file: %s\n' % logs[i])
        fout.write('echo Site: ${GLIDEIN_ResourceName}\n')
        fout.write('echo Node: `hostname`\n')
        fout.write('echo OS: `uname -a`\n')
        fout.write('echo User: `whoami`\n')
        fout.write('echo --------------------------------------------------------------------\n')
        fout.write('echo\n')

        fout.write('source ' + os.path.join(os.getenv('DPSIM_ROOT'), 'exenv.sh') + '\n')
        fout.write('cd $_CONDOR_SCRATCH_DIR\n')
        fout.write('start_sec=$(date +%s)\n')
        fout.write('start_time=$(date +%F_%T)\n')
        fout.write('%s -t %s -o %s -s %d > %s &' % (DPPythia, tempdir, outputs[i], options.seed+i, logs[i]))
        fout.write('status=$?\n')
        fout.write('stop_time=$(date +%F_%T)\n')
        fout.write('stop_sec=$(date +%s)\n')
        fout.write('duration=$(expr $stop_sec "-" $start_sec)\n')
        fout.write('echo Executable exit code: $status\n')
        fout.write('echo Job duration in seconds: $duration\n')
        fout.write('exit $retcode\n')

        fout.close()

    # make jobsub commands and submit
    for i in range(len(logs)):
        cmd = 'jobsub_submit -g --OS=SL6 --use_gftp --resource-provides=usage_model=DEDICATED,OPPORTUNISTIC -e IFDHC_VERSION'
        cmd = cmd + ' -L %s' % logs[i]
        cmd = cmd + ' -d OUTPUT %s' % outputs[i]
        cmd = cmd + ' file://`which %s`' % wrappers[i]

        runCmd(cmd)
