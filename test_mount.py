#!/usr/bin/python

from time import sleep
from subprocess import Popen, PIPE
import sys
import os

file_name = 'dir_test'
root_path = '{}'.format(file_name)

def print_err(out, err):
    if out:
        print 'out: {}'.format(out)
    if err:
        print 'err: {}'.format(err)
        print 'len(err): {}'.format(len(err))
    print '____________________'

def run_cmds(cmd, expected_out, check_exp=True):
    child = Popen(cmd.split(' '), stdout=PIPE, stderr=PIPE)
    out, err = child.communicate()
    if check_exp:
        if expected_out not in out or len(err) > 0:
            print_err(out, err)
            return err, False
        print_err(out, False)
    return out, True

expected_fs_type = None
def set_expected():
    global expected_fs_type
    if expected_fs_type is None:
        out, status = run_cmds('stat -f -c %T /home', '', False)
        if not status:
            print 'ERROR Determining default FS Type'
            sys.exit(1)
        expected_fs_type = out.rstrip('\n')
        print 'Init Config: {}'.format(expected_fs_type)
    return expected_fs_type


def run_stat(opt):
    if opt == 'mount':
        expected = 'UNKNOWN (0x69ff69ff)'
    elif opt == 'umount':
        expected = set_expected()
    else:
        return False
    cmd = 'stat -f -c %T {}'.format( root_path )
    return run_cmds(cmd, expected)[1]


def run_mount():
    expected = '/dev/ram5 on {}/{} type thunderfs (rw)'.format( os.getcwd(), file_name)
    cmd = 'mount -v -t thunderfs /dev/ram5 {}'.format( root_path )
    run_cmds(cmd, expected)


def run_umount():
    expected = '/dev/ram5 has been unmounted'
    cmd = 'umount -v {}'.format( root_path )
    run_cmds(cmd, expected)


def main():
    if run_stat('umount') is False:
        print 'HAULT BEFORE TEST START.'
        print 'stat -f -c %T {} != {}'.format( file_name, set_expected() )
        sys.exit(1)
    iterations = 100
    time = 1
    sequence = [run_mount, run_stat, run_umount, run_stat]
    test_success = True
    for cmd in range(iterations):
        if run_mount() is False:
            test_success = False
            print 'Fault mount'
            print 'On Loop: {}'.format(cmd)
            break
        sleep(1)
        if run_stat('mount') is False:
            test_success = False
            print 'Fault stat mount'
            print 'On Loop: {}'.format(cmd)
            break
        sleep(1)
        if run_umount() is False:
            test_success = False
            print 'Fault umount'
            print 'On Loop: {}'.format(cmd)
            break
        sleep(1)
        if run_stat('umount') is False:
            test_success = False
            print 'Fault stat umount'
            print 'On Loop: {}'.format(cmd)
            break
        sleep(1)
    if test_success:
        print '\n############ TEST SUCCESS ############'
    else:
        print '\n############ TEST FAILURE ############'

if __name__ == '__main__':
    interrupt = False
    try:
        main()
    except EOFError:
        interrupt = True
    except KeyboardInterrupt:
        interrupt = True

    if interrupt and run_stat('umount') is False:
        print '\n############ WARNING ############'
        print 'STUCK ON thunderfs'
        print 'USER INTERVENTION REQUIRED'
        print '############ WARNING ############'





