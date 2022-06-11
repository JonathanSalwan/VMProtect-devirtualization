#!/usr/bin/env python

#
# Copyright (C) 2007-2018 Intel Corporation.
# SPDX-License-Identifier: MIT
#


# -*- python -*-

# Grab the BLOCKCOUNT and MAPPADDR lines from the mix-mt output and
# attach that to a copy of the program. Run mix-mt with the -mapaddr
# pin took knob. And compile your program with -g

import os,sys,re
pcmap={}

class stat_t(object):
    def __init__(self, filename='unknown', line=0, pc=None):
        self.filename=filename
        self.line = int(line)
        self.count = 0
        self.pc = pc
        self.rank = 0

def stat_sorter(a,b):
    if a.count > b.count:
        return -1
    if a.count < b.count:
        return 1
    return 0

def stat_sorter_line(a,b):
    if a.line > b.line:
        return 1
    if a.line < b.line:
        return -1
    return 0

def combine_by_file_and_line(lines):
    """Even though the PC might be different"""
    tmap = {}
    for f in lines:
        key = "%s-%s" % (f.line, f.filename)
        if key not in tmap:
            tmap[key]=f
        else:
            tmap[key].count += f.count
    tmap['0-unknown'].pc = 'various'
    return list(tmap.values())
        
    
def summarize():
    global pcmap

    #for k,f in pcmap.iteritems():
    #    print "%9s %8s %s:%s" % ( f.count, k, f.filename, f.line)
    lines = list(pcmap.values())
    # remove the zero entries
    lines = [x for x in lines if x.count != 0]
    # merge things with the same file and line
    lines = combine_by_file_and_line(lines)
    lines.sort(cmp=stat_sorter)
    for i,f in enumerate(lines):
        f.rank = i
        print("AA%05d: %9s %8s %s:%d" % ( i, f.count, f.pc, f.filename, f.line))
    return lines
        
def store_mapaddr(p):
    global pcmap
    (dummy, pc, line, filename) = p
    #print "setting pc %s to %s : %s" % (pc, filename,line)
    if pc in pcmap:
        pcmap[pc].filename = filename
        pcmap[pc].line = int(line)
        pcmap[pc].pc=pc
    else:
        pcmap[pc]=stat_t(filename,line,pc)
    
def store_blockcount(p):
    global pcmap
    (dummy, pc, count) = p
    if pc in pcmap:
        pcmap[pc].count += int(count)
    else:
        t = stat_t()
        t.pc = pc
        t.count = int(count)
        pcmap[pc]  = t


def make_decorated_file(ifn,flines):
    """Emit a copy of f with the counts specified in the sorted list of stat_t's"""
    fnew = "%s.pcov" % (ifn)
    if os.path.exists(fnew):
        sys.stderr.write("The file %s already exists. Please remove it if you want the new profile\n" % (fnew))
        return
    try:
        f = open(fnew,"w")
    except:
        sys.stderr.write("Could not write %s -- skipping\n" % (fnew))
        return
    sys.stderr.write("Writing [%s]\n" % (fnew))
    nline = 1
    for x in  file(ifn).readlines():
        decorated = False
        while len(flines) > 0 and flines[0].line == nline:
            if decorated:
                f.write("+")
            else:
                f.write("AA%05d " % (flines[0].rank))
            f.write("%9d" % (flines[0].count))
            flines.pop(0)
            decorated = True
        if not decorated:
            f.write("%7s %9s" % ('', '')) # pad the line equivalently
        f.write(" ")
        f.write(x)
        nline += 1
    f.close()

def main(input_fn):
    mapaddr = re.compile("^MAPADDR ")
    blockcount = re.compile("^BLOCKCOUNT ")
    lines = file(input_fn).readlines()
    for line in lines:
        m = mapaddr.match(line)
        if m:
            store_mapaddr(line.strip().split())
        b = blockcount.match(line)
        if b:
            store_blockcount(line.strip().split())
    lines = summarize()
    
    filenames = {}
    for f in lines:
        if f.filename not in filenames and f.filename != 'unknown':
            filenames[f.filename]=1

    for f in list(filenames.keys()):
        flines = [x for x in lines if x.filename == f]
        flines.sort(cmp=stat_sorter_line)
        make_decorated_file(f,flines)
    
if len(sys.argv) != 2:
    sys.stderr.write("Need an input file\n")
    sys.exit(1)
input_fn = sys.argv[1]
print("Input file %s" % (input_fn))
main(input_fn)
