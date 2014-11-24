#!/usr/bin/python

from time import sleep
from subprocess import Popen, PIPE
import sys
import os

file_name = 'dir_test'
root_path = '{}'.format(file_name)


def run_cmds(cmd):
    child = Popen(cmd.split(' '), stdout=PIPE, stderr=PIPE)
    out, err = child.communicate()
    if err > 0:
        print err
        return False
    return True

def touch_file(num):
    cmd = 'touch ./{}/file{}'.format(file_name, num)
    print 'Making: file{}'.format(file_name)
    print '________________________________'
    run_cmds(cmd)

def get_file(num, expected, removed):
    path = os.path.join(os.getcwd(), file_name)
    lsdir = os.listdir(path)
    print './{}/* : {}'.format( file_name, repr(lsdir) )
    if removed and expected not in lsdir:
        #print 'File has been removed'
        return True         # File has been removed
    elif not removed and expected in lsdir:
        #print 'File has been created'
        return True         # File exists
    else:
        print "########## CRITICAL ERROR ##########"
        print repr(lsdir)
        sys.exit(1)
    return False

def rm_file(num):
    cmd = 'rm ./{}/file{}'.format(file_name, num)
    print 'Removing: file{}'.format(file_name)
    print '________________________________'
    run_cmds(cmd)


def main():
    print 'main fun'
    iterations = 100
    delay = 1
    for x in range(iterations):
        num = x % 100
        fi = 'file{}'.format(num)
        touch_file(num)
        sleep(delay)
        success_mk = get_file(num, fi, False)
        if not success_mk:
            print 'fail mk'
            return False
        sleep(delay)
        rm_file(num)
        sleep(delay)
        success_rm = get_file(num, fi, True)
        if not success_rm:
            print 'fail rm'
            return False
        sleep(1)
    return True

if __name__ == '__main__':
    try:
        success = main()
    except KeyboardInterrupt:
        print "\n############## KEYBOARD INTERRUPT ##############"
        sys.exit(1)

    if success:
        print '\n############## SUCCESS ##############'
    else:
        print '\n############## FAILURE ##############'

