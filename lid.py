#! /usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import division
import os
import sys
import re

# Assumes SolidPython is in site-packages or elsewhwere in sys.path
from solid import *
from solid.utils import *

SEGMENTS = 16

card_pocket_w = 56
card_pocket_h = 50

case_w = card_pocket_w+5
case_h = card_pocket_h+5
case_th = 2

epsilon = 0.5

def lid_top():
    return cube([case_w, case_h, case_th], center=True)

def lid_bottom():
    return cube([case_w-2*case_th-epsilon, case_h-2*case_th-epsilon, case_th], center=True)

def post():
    post_w = 6
    post_h = 6
    pcb_h = 2
    nub_d = 1
    offset = 1
    p = translate([-post_w/2, -post_w/2, 0])(cube([post_w, post_w, post_h+offset]))
    slot = translate([0, 0, pcb_h])(cube([post_w, post_w, post_h]))
    nub = translate([0, post_w/2, 2*pcb_h])(rotate([90, 0, 0])(cylinder(nub_d/2, post_w/2)))
    return p-slot+nub

def rfid_sups():
    return pcb_sup(18, 39)

def ardu_sups():
    return pcb_sup(18, 43.5)

def pcb_sup(w, l):
    s1 = translate([-w/2, -l/2, 0])(post())
    s2 = translate([w/2, -l/2, 0])(mirror([1, 0, 0])(post()))
    s3 = translate([-w/2, l/2, 0])(mirror([0, 0, 0])(rotate([0, 0, 270])(post())))
    s4 = translate([w/2, l/2, 0])(mirror([1, 0, 0])(rotate([0, 0, 270])(post())))
    return s1+s2+s3+s4

def side():
    h = case_h-(case_th+epsilon)*2
    return translate([0, -h/2, case_th-epsilon])(cube([2, h, 10]))

def assembly():
    lt = lid_top()
    lb = up(case_th)(lid_bottom())
    rfs = translate([case_w/4, 0, case_th*2-1])(rfid_sups())
    r4s = translate([-case_w/4, 0, case_th*2-1])(ardu_sups())
    s1 = translate([-case_w/2+case_th+epsilon/2, 0, 0])(side())
    s2 = translate([case_w/2-case_th-epsilon/2-2, 0, 0])(side())
    return lt+lb+rfs+r4s+s1+s2

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=False)
