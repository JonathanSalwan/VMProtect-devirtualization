#! /usr/bin/env python

#
# Copyright (C) 2004-2018 Intel Corporation.
# SPDX-License-Identifier: MIT
#


#
# python.org has useful info about the Python programming language
#
# The Python library is described here: http://docs.python.org/lib/lib.html
# An the index for the library here: http://docs.python.org/lib/genindex.html

import sys
import os
import getopt
import re
import string
import copy
import bisect

#######################################################################
# Version
#######################################################################

def Version():
    (l,v,x) = string.split('$Revision: 1.6 $')
    return v

#######################################################################
# Usage
#######################################################################

def Usage():
    print("Usage: callgraph.py [OPTION]* symbol-file edge-profile")
    print()
    print("OPTIONS:")
    print("\t-m mode       choose output format:  vcg or text")
    print("\t-u            if one routine invokes another from multiple callsites, merge them into one edge")
    print("\t-c cutoff     set cutoff, edges with smaller counts will not be shown ")
    print("\t-t threshold  set thresholds, routines with a call count above the threshold will be colored red")   
    print() 
    print("the symbol file is compatible to the readelf symbol dump, it should be generated as follows:")
    print("\t readelf -s img | grep FUNC >sym.txt")
    print() 
    print("the edge-profile must be generated using the edgcnt Pin tool")
    return -1

#######################################################################
# Messages 
#######################################################################

def Info(str):
    print(str, file=sys.stderr)
    return


def Warning(str):
    print(str, file=sys.stderr)
    return

def Error(str):
    print(str, file=sys.stderr)
    sys.exit(-1)


#######################################################################
ALL_RTN = {}
        
class RTN:
    def __init__(self,start,name,external=0):
        self._start = start
        self._name = name
        self._in = []
        self._out = []
        ALL_RTN[start] = self
        return
    

    def add_out_edg(self, edg ):
        self._out.append(edg)
        return
                             
    def add_in_edg(self, edg ):
        self._in.append(edg)
        return

    def count_in(self):
        count = 0
        for e in  self._in: count += e.count()
        return count

    def count_out(self):
        count = 0
        for e in  self._out: count += e.count()
        return count


    def start(self):
        return self._start

    def is_external(self):
        return  self._external

    def name(self):
        return  self._name

    def find_target(self,dst):
        for e in self._out:
            if e._dst == dst: return e
        return None

    def vcg_name(self):
        return hex(self.start())

    def String(self,cutoff):
        s = "RTN  %s [%x]  icnt %d  ocnt: %d)\n" % (self._name, self._start,  self.count_in(), self.count_out())

        s += "callers:\n"
        for edg in self._in:
            if edg.count() < cutoff: continue
            r = edg._src
            s += "\t%s %s [%x] %d\n" % (edg._type,r.name(),r.start(),edg._count)

        s += "callees:\n"
        for edg in self._out:
            if edg.count() < cutoff: continue
            r = edg._dst
            s += "\t%s %s [%x] %d\n" % (edg._type,r.name(),r.start(),edg._count)
        return s


    def StringVCG(self,threshold):
        s = "\t" + "node:\n"
        s += "\t" + "{\n"
        if self.count_in() > threshold or self.count_out() > threshold:
            s += "\t\t" + "color: red\n"            
        s += "\t\t" + "title: \"" + self.vcg_name() + "\"\n"
        s += "\t\t" + "label: \"" + self.name() + ": " + hex(self._start) + "\\n"
        s += "icnt: %d  ocnt: %d\"" % (self.count_in(),self.count_out())
        s += "\t" + "}\n"
        return s
    
#######################################################################
# 
#######################################################################
ALL_EDG = []

class EDG:
    def __init__(self,src,dst,type,count):
        global ALL_EDG
        self._src = src
        self._dst = dst
        self._type = type
        self._count = count
        src.add_out_edg(self )
        dst.add_in_edg(self )
        ALL_EDG.append(self)
        return

    def count(self):
        return self._count
    
    def add_count(self,count):
        self._count += count
        return
    
    def StringVCG(self, threshold = 100000000000):
        s = ""
        if self._count > threshold:
            s += "\t" + "nearedge:\n"
        else:
            s += "\t" + "edge:\n"

        s += "\t{\n"    
        s += "\t\t" + "sourcename: \"" + self._src.vcg_name() + "\"\n"
        s += "\t\t" + "targetname: \"" + self._dst.vcg_name() + "\"\n"
        s += "\t\t" + "thickness: 2\n"
        s += "\t\t" + "label: \"%s(%d)\"\n" % (self._type,self._count)

#        s += "\t\t" + "priority: %d\n" % self._count                        
        s += "\t}\n"    
        return s
        
#######################################################################
# 
#######################################################################


def DumpVCG(cutoff, threshold):
    print("// ###################################################################################")
    print("// VCG Callgraph: cutoff %d,  threshold %d" %(cutoff, threshold))
    print("// ###################################################################################")

    print("graph:")
    print("{");

    print("title: \"Call Graph\"")
    print("label: \"Call Graph\"")
    print("display_edge_labels: yes")
    print("layout_downfactor: 100")
    print("layout_nearfactor: 10")
    print("layout_upfactor: 1")
#    print "dirty_edge_labels: yes"
    print("layout_algorithm: mindepth")
    print("manhatten_edges: yes")
    print("edge.arrowsize: 15")
    print("late_edge_labels: yes")    

    for e in ALL_EDG:
        if e._count < cutoff: continue 
        print(e.StringVCG())

    list = list(ALL_RTN.items())
    list.sort()
    for (x,b) in list:
        if b.count_out() < cutoff and b.count_in() < cutoff: continue 
        print(b.StringVCG(threshold))

    print("}");
    print("// eof")
    return

#######################################################################
# 
#######################################################################

def DumpText(cutoff):

    print("# ###################################################################################")
    print("# Callgraph: cutoff %d" % cutoff)
    print("# ###################################################################################")


    list = list(ALL_RTN.items())
    list.sort()
    for (x,b) in list:
        if b.count_out() < cutoff and b.count_in() < cutoff: continue 
        print(b.String(cutoff))

    print("# eof")
    return

#######################################################################
# SYMBOL PROCESSING
#######################################################################
#   Num:    Value          Size Type    Bind   Vis      Ndx Name
# 24: 0000000000000000     0 FILE    LOCAL  DEFAULT  ABS crtstuff.c


def ProcessSymbols(lines):

    for l in lines:
        ll = string.split(l)
        (num,value,size,type,bind,vis,ndx,name) = ll[:8]
        start = int(value,16)
        if start in ALL_RTN:
            name2 = ALL_RTN[start].name()
            if string.find(name2,name) == -1 and   string.find(name,name2) == -1:
                Warning("two routines with same address %x: %s vs %s" %(start,name2,name))
        else:        
            RTN(start,name)
    return    

RTN_MAP = []


MAX_ADDR = 0

PatternSyscall = re.compile(r'^[fF]+$')

def FindRtnByAddress(addr, s):
    global RTN_MAP
    global ALL_RTN
    index = bisect.bisect_left(RTN_MAP,addr)

    if 0 < index and  index < len(RTN_MAP):
        a = RTN_MAP[index]
        if a == addr:
            r = ALL_RTN[a]
#            print "found match rtn", r._name," ", hex(a)," for address ",hex(addr)            
            return r
        else:
            assert( a > addr)
            a = RTN_MAP[index-1]
            assert( a < addr)
            r =  ALL_RTN[a]
            return r
    else:
        if addr == MAX_ADDR:
            Warning("found syscall")
            return RTN(addr,"syscall",1 )

        if addr not in ALL_RTN:
            RTN(addr,"unknown_%s_0x%x" % (s,addr),1 )
        return ALL_RTN[addr]



#######################################################################
#  0x0000000000400366 0x0000000000402300         2182

PatternEdge = re.compile(r'^\s*0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+([a-zA-Z])\s+([0-9]+)\s+.*$')

def ProcessEdgProfile(lines, unique_edgs_only):
    global MAX_ADDR
    
    version = string.split(lines[0])
    if version[0] != "EDGCOUNT":
        Error("input file  is not an edgcnt profile")

    if version[1] != "4.0":
        Error("unsupported  profile version %s" % version[1])
        
    for l in lines[1:]:
        match = PatternEdge.match(l)
        if not match:
            continue

        src = int(match.group(1),16)
        dst = int(match.group(2),16)
        type = match.group(3)
        count = int(match.group(4))

        if dst > MAX_ADDR: MAX_ADDR = dst
        if src > MAX_ADDR: MAX_ADDR = src

        rtn_src = FindRtnByAddress(src,"call")
        rtn_dst = FindRtnByAddress(dst,"rtn")

        if type =='r': continue
        
        if type != 'c' and type != 'C' and rtn_src == rtn_dst: continue

        if unique_edgs_only:
            e = rtn_src.find_target(rtn_dst)
            if e:
                e.add_count(count)
                continue
                
#        print "found edg ",rtn_src._name," -> ", rtn_dst._name
        e = EDG(rtn_src,rtn_dst,type,count)

    return

#######################################################################
# Main
#######################################################################

def Main(argv):
    global RTN_MAP
    global ALL_RTN
    global ALL_EDG


    try:
        opts, args = getopt.getopt(argv, "m:c:t:u")
    except getopt.error:
        Usage()
        return -1
    
    mode = "vcg"
    cutoff = 1
    unique_edgs_only = 0
    threshold = 1000
    
    for o, a in opts:
        if o == "-m":
            mode = a
        elif o == "-c":
            cutoff = int(a)
        elif o == "-t":
            threshold = int(a)
        elif o == "-u":
            unique_edgs_only = 1
        else:
            Usage()
            return -1

    if len(args) != 2:
        Usage()
        return
    
    Info( "Reading Symbols")


    filename = args[0]
    try:
        input = open(filename, "r")
        lines = input.readlines()
        input.close()
    except:
        raise Error("cannot read data " + filename)

    ProcessSymbols(lines)


    RTN_MAP = [x._start for x in list(ALL_RTN.values())]
    RTN_MAP.sort()
    Info("found %d symbols" % len(RTN_MAP))

    Info( "Reading edges")
    
    filename = args[1]
    try:
        input = open(filename, "r")
        lines = input.readlines()
        input.close()
    except:
        raise Error("cannot read data " + filename)

    ProcessEdgProfile(lines,unique_edgs_only)

    Info("Read %d edges" % len(ALL_EDG))

    if mode == "vcg":
        DumpVCG(cutoff, threshold)
    elif mode == "text":
        DumpText(cutoff)
    else:
        Usage()
        return -1
        
    
    return 0

#######################################################################
# 
#######################################################################

if __name__ == "__main__":
    sys.exit( Main( sys.argv[1:]) )

#######################################################################
# eof
#######################################################################
