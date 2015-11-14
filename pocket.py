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
card_pocket_d = 1.5

fob_pocket_inside_dia = 35
fob_pocket_inside_h = 5
fob_pocket_inside_radius = 1
fob_pocket_thickness = 2

def fob_hole():
    s = sphere(1)
    cyl = cylinder(fob_pocket_inside_dia/2, fob_pocket_inside_h-2*fob_pocket_inside_radius)
    rounded_cyl = minkowski()(cyl, s)
    return rounded_cyl

def fob_pocket():
    # outside rounding radius
    r = fob_pocket_inside_radius+fob_pocket_thickness
    s = sphere(r)
    cyl = cylinder(fob_pocket_inside_dia/2, 
                   fob_pocket_inside_h-2*fob_pocket_inside_radius)
    return minkowski()(cyl, s)

def fob_cuts():
    # remove top half of fob pocket
    fob_cut1 = translate([-card_pocket_w/2, 
                          -fob_pocket_inside_dia, 
                          -(fob_pocket_inside_h*2+2*fob_pocket_thickness+2)/2])(
                              cube([card_pocket_w+2,
                                    fob_pocket_inside_dia, 
                                    fob_pocket_inside_h*4]))
    
    fob_cut2 = translate([-card_pocket_w/2-1, -card_pocket_h, -10])(
        cube([card_pocket_w+2, card_pocket_h*2+2, 10]))
    return fob_cut1+fob_cut2

def card_pocket():
    return down(1)(back(1)(left(card_pocket_w/2)(cube([card_pocket_w, 
                                                       card_pocket_h+1, 
                                                       card_pocket_d+1]))))

def half_sphere(r):
    return rotate([-90, 0, 0])(difference()(sphere(r), 
                                            down(r)(left(r)(cube([r*2,r*2,r*2])))))

def half_cylinder(r):
    return rotate([-90, 0, 0])(difference()(cylinder(r,r), 
                                            down(r-1)(left(r)(cube([r*2,r*2,r*2])))))

def card_cover():
    d = 2
    r = 3
    card_cover_w = card_pocket_w+2*d
    card_cover_d = card_pocket_d+d
    
    left_s = translate([card_cover_w/2-r, card_pocket_h, 0])(half_sphere(r))
    right_s = translate([-(card_cover_w/2-r), card_pocket_h, 0])(half_sphere(r))

    left_c = translate([card_cover_w/2-r, 0, 0])(half_cylinder(r))
    right_c = translate([-(card_cover_w/2-r), 0, 0])(half_cylinder(r))
    return hull()(left_c+right_c+left_s+right_s)

def assembly():
    fp = fob_pocket()
    fh = fob_hole()
    fc = fob_cuts()
    cp = card_pocket()
    cc = card_cover()

    return fp+cc-fh-cp-fc

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=False)
