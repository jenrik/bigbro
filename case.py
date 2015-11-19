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
case_d = 20
case_th = 1.5
coil_sup_l = 3
coil_sup_d = 3
# Inner diameter of coil
coil_sup_w = 43
coil_sup_h = 31

def coil_sup():
    return cylinder(coil_sup_d/2, coil_sup_l)

def coil_sups():
    s1 = translate([-(coil_sup_w-coil_sup_d/2)/2, -(coil_sup_h-coil_sup_d/2)/2, 0])(coil_sup())
    s2 = translate([(coil_sup_w-coil_sup_d/2)/2, -(coil_sup_h-coil_sup_d/2)/2, 0])(coil_sup())
    s3 = translate([(coil_sup_w-coil_sup_d/2)/2, (coil_sup_h-coil_sup_d/2)/2, 0])(coil_sup())
    s4 = translate([-(coil_sup_w-coil_sup_d/2)/2, (coil_sup_h-coil_sup_d/2)/2, 0])(coil_sup())
    return s1+s2+s3+s4

def bottom():
    return translate([-case_w/2, -case_h/2, 0])(cube([case_w, case_h, case_th]))

def frame():
    outer = translate([-case_w/2, -case_h/2, case_th])(cube([case_w, case_h, case_d]))
    iw = case_w-2*case_th
    ih = case_h-2*case_th
    inner = translate([-iw/2, -ih/2, case_th])(cube([iw, ih, case_d+1]))
    return outer-inner

def assembly():
    bt = bottom()
    cs = up(case_th)(coil_sups())
    fr = frame()

    return cs+fr+bt

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=False)
