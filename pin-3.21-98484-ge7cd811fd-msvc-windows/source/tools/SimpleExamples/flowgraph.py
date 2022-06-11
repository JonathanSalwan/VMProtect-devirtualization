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

#######################################################################
# Version
#######################################################################

def Version():
    
    (l,v,x) = string.split('$Revision: 1.5 $')
    return v

#######################################################################
# Usage
#######################################################################

def Usage():
    print("Usage: flowgraph.py [OPTION]+ assembler-listing edge-profile")
    print()
    print("flowgraph converts a disassembled routine into a flowgraph which can be rendered using vcg") 
    print()
    print("assembler-listing is a textual disassembler listing generated with")
    print("objdump-routine.csh or directly with objdump")
    print()
    print("edge-profile is a profile generated with the edgcnt Pin tool")

    
    return -1

#######################################################################
# Messages
#######################################################################

def Info(str):
    print("I:",str, file=sys.stderr)
    return

def Warning(str):
    print("W:", str, file=sys.stderr)
    return

def Error(str):
    print("E:",str, file=sys.stderr)
    sys.exit(-1)


#######################################################################
# 
#######################################################################
#  402d05:       41 56                   push   %r14

PatternNoFallthrough = re.compile(r'call|ret|jmp')
PatternCall = re.compile(r'call')

class INS:
    def __init__(self, addr, opcode ):
        self._addr = addr
        self._opcode = opcode
        self._next = None
        self._leader = 0
        self._bbl = None
        return

    def get_opcode(self):
        return self._opcode
 
    def set_next(self,next):
        self._next = next
        return

    def get_next(self):
        return self._next
                   
    def get_addr(self):
        return self._addr

    def get_leader(self):
        return self._leader
                   
    def set_leader(self,leader):
        self._leader = leader

    def get_bbl(self):
        return self._bbl
                   
    def set_bbl(self,bbl):
        self._bbl = bbl

    def has_no_fallthru(self):
        return PatternNoFallthrough.search(self._opcode)

    def is_call(self):
        return PatternCall.search(self._opcode)
                   
#######################################################################
##
#######################################################################

ALL_INS = {}

PatternAssemler = re.compile(r'^\s*([0-9a-fA-F]+):\s*(?:[0-9a-fA-F][0-9a-fA-F] )+\s*(.+)$')


def ProcessAssemblerListing(lines):
    last_ins = None

    for l in lines:
        match =  PatternAssemler.match(l)
        if not match:
#            print "bad line ",l
            continue
        addr = int(match.group(1),16)
        ins = INS( addr,  match.group(2) )
        ALL_INS[addr] = ins
        if last_ins:
           last_ins.set_next(ins)
        last_ins = ins  
    return

#######################################################################
#  0x0000000000400366 0x0000000000402300         2182

PatternEdge2 = re.compile(r'^\s*0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+([0-9]+)\s*$')
PatternEdge3 = re.compile(r'^\s*0x([0-9a-fA-F]+)\s+0x([0-9a-fA-F]+)\s+([a-zA-Z])\s+([0-9]+)\s*$')

def ProcessEdgProfile(lines):

    
    version = string.split(lines[0])

    if version[0] != "EDGCOUNT":
        Error("files is not an edge profile")

    if version[1] == "2.0":
        v = 2
    elif version[1] == "3.0":
        v = 3
    else:
        Error("unsupported  edge profile version")
        
    edg_list = []

    for l in lines[1:]:
        if v == 2:
            match = PatternEdge2.match(l)
        elif v==3:
            match = PatternEdge3.match(l)

        if not match: continue

        if v == 2:
            src = int(match.group(1),16)
            dst = int(match.group(2),16)
            count = int(match.group(3))
            type = "u"
        elif v == 3:
            src = int(match.group(1),16)
            dst = int(match.group(2),16)
            type = match.group(3)
            count = int(match.group(4))

        if src in ALL_INS:
            next = ALL_INS[src].get_next()
            if next: next.set_leader(1)

        if dst in ALL_INS:    
            ins = ALL_INS[dst]
            ins.set_leader(1)

        if src in ALL_INS or dst in ALL_INS:
            edg_list.append( (src,dst,count,type) )            
            
    return edg_list

#######################################################################
# 
#######################################################################
class EDG:
    def __init__(self,src,dst,count, type):
        self._src = src
        self._dst = dst
        self._count = count
        self._type = type
        return

    def is_fallthru(self):
        return self._fallthru
    
    def StringVCG(self, threshold = 100000000000):
        s = ""
        if self._count > threshold:
            s += "\t" + "nearedge:\n"
        else:
            s += "\t" + "edge:\n"
        
        s += "\t{\n"    
        s += "\t\t" + "sourcename: \"" + hex(self._src._start) + "\"\n"
        s += "\t\t" + "targetname: \"" + hex(self._dst._start) + "\"\n"
        if self._type == "F" or self._type == "L":
            s += "\t\t" + "thickness: 4\n"
        else:
            s += "\t\t" + "thickness: 2\n"

        s += "\t\t" + "label: \"%s(%d)\"\n" % (self._type,self._count)
            
#        s += "\t\t" + "priority: %d\n" % self._count                        
        s += "\t}\n"    
        return s
        
#######################################################################



class BBL:
    def __init__(self,start):
        self._start = start
        self._ins = []
        self._in = []
        self._out = []
        self._count = 0
        self._in_count = 0
        self._out_count = 0
        self._next = None
        return
    
    def add_ins(self,ins):
        self._ins.append(ins)
        self._end = ins.get_addr()
        return

    def set_count(self,count):
        assert( self._count == 0 )
        self._count = count
        return

    def add_out_edg(self, edg ):
        self._out.append(edg)
        return
                             
    def add_in_edg(self, edg ):
        self._in.append(edg)
        return

    def add_in_count(self, count ):
        self._in_count += count
        return

    def add_out_count(self, count ):
        self._out_count += count
        return

    def count_in(self):
        count = self._in_count
        for e in  self._in: count += e._count
        return count

    def count_out(self):
        count = self._out_count
        for e in  self._out: count += e._count
        return count

    def set_next(self,next):
        self._next = next
        return

    def get_next(self):
        return self._next

    def get_start(self):
        return self._start

    def is_call(self):
        return  self._ins[-1].is_call()

    def has_no_fallthru(self):
        return  self._ins[-1].has_no_fallthru()



    def String(self):
        s = "BBL  at %x  count %d   (i: %d  o: %d)\n" % (self._start, self._count, self._in_count, self._out_count)

        s += "i:  "
        for edg in self._in:
            s += "%x (%d)  " % (edg._src.get_start(),edg._count)
        s += "\n"

        s += "o:  "
        for edg in self._out:
            s += "%x (%d)  " % (edg._dst.get_start(),edg._count)
        s += "\n"

        for ins in self._ins:
            s += "%x %s\n" % (ins.get_addr(),ins.get_opcode())
        return s


    def StringVCG(self,threshold=1000):
        s = "\t" + "node:\n"
        s += "\t" + "{\n"
        if self._count > threshold:
            s += "\t\t" + "color: red\n"            
        s += "\t\t" + "title: \"" + hex(self._start) + "\"\n"
        s += "\t\t" + "label: \"" + hex(self._start) + " (" + str(self._count) + ")\\n"
        for ins in self._ins:             s += "%x: %s\\n" % (ins.get_addr(),ins.get_opcode())
        s += "\"\n"
        s += "\t" + "}\n"
        return s

    
#######################################################################
#
#######################################################################
ALL_BBL = {}
ALL_EDG = []


#######################################################################
#
#######################################################################

def CreateCFG(edg_list):
    no_interproc_edges = 1

    ins_list = list(ALL_INS.items())
    ins_list.sort() # by addr

    bbl_list = []

    Info("BBL create")

    last = None
    for (a,ins) in ins_list:

        if ins.get_leader():
           start = ins.get_addr()
           bbl = BBL(start)               
           bbl_list.append(bbl)
           ALL_BBL[start] = bbl
           if last: last.set_next( bbl )
           last = bbl
               
        last.add_ins( ins )
        ins.set_bbl( last )

        if ins.has_no_fallthru():
           next = ins.get_next()
           if next: next.set_leader(1)

    Info( "Created %d bbls" % len(bbl_list))     
#    for bbl in bbl_list: print bbl.String()

    Info( "EDG create")
    
    for (src,dst,count,type) in edg_list:

        if src in ALL_INS:
            bbl_src = ALL_INS[src].get_bbl()
        else:
            assert( dst in ALL_BBL )
            if no_interproc_edges:
                ALL_BBL[dst].add_in_count(count)
                continue             
            bbl_src = BBL(src)
            ALL_BBL[src] = bbl_src

        if dst in ALL_BBL:
            bbl_dst = ALL_BBL[dst]
        else:
            if no_interproc_edges:
                bbl_src.add_out_count(count)
                continue             
        
            bbl_dst = BBL(dst)
            ALL_BBL[dst] = bbl_dst

                
        edg = EDG( bbl_src, bbl_dst, count, type)
        ALL_EDG.append( edg )
        bbl_src.add_out_edg( edg )
        bbl_dst.add_in_edg( edg )


    Info("propagate counts and add fallthrus")

    for bbl in bbl_list:
        count = bbl.count_in()
        bbl.set_count(count)
        count -= bbl.count_out()        
        if count < 0:
            Warning("negative fallthru count")
            count = 0        

        next = bbl.get_next() 
                        
        if count > 0:
           if bbl.has_no_fallthru():
              Info("losing flow %d\n" % count)
           elif next:               
                edg = EDG(bbl,next,count,"F")
                ALL_EDG.append( edg )
                bbl.add_out_edg( edg )
                next.add_in_edg( edg )

        if bbl.is_call() and next:
            edg = EDG(bbl,next, 0,"L")
            ALL_EDG.append( edg )
            bbl.add_out_edg( edg )
            next.add_in_edg( edg )

#    for bbl in bbl_list: print bbl.String()
    return bbl_list

def DumpVCG():
    start = 0
    end = 0 
    print("// ###################################################################################")
    print("// VCG Flowgraph for %x - %x" % (start,end))
    print("// ###################################################################################")
 
    print("graph:")
    print("{");

    print("title: \"Control Flow Graph for rtn %x - %x \"" % (start,end));
    print("label: \"Control Flow Graph for rtn %x - %x \"" % (start,end));
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
        print(e.StringVCG())

    bbl_list = list(ALL_BBL.items())
    bbl_list.sort()
    for (x,b) in bbl_list: 
        print(b.StringVCG())

    print("}");
    print("// eof")
    return
#######################################################################
# Main
#######################################################################

def Main(argv):

    if len(argv) != 2:
        Usage()
        return -1
    
    Info( "Reading listing")

    filename = argv[0]
    try:
        input = open(filename, "r")
        lines = input.readlines()
        input.close()
    except:
        Error("cannot read data " + filename)

    ProcessAssemblerListing(lines)

    Info( "Reading edges")
    
    filename = argv[1]
    try:
        input = open(filename, "r")
        lines = input.readlines()
        input.close()
    except:
        Error("cannot read data " + filename)

    edg_list = ProcessEdgProfile(lines)

    Info("Read %d edges" % len(edg_list))

    bbl_list = CreateCFG( edg_list)

    Info("Dump VCG to stdout")    
        
    DumpVCG()
        
    return 0

#######################################################################
# 
#######################################################################

if __name__ == "__main__":
    sys.exit( Main( sys.argv[1:]) )

#######################################################################
# eof
#######################################################################
