#! /usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import division
import os
import sys
import re

# Assumes SolidPython is in site-packages or elsewhwere in sys.path
from solid import *
from solid.utils import *

SEGMENTS = 48

card_pocket_w = 56
card_pocket_h = 50
card_pocket_d = 1.5

fob_inside_dia = 35
fob_inside_h = 6

def fob_hole():
    m1 = [ [ fob_inside_dia, 0, 0, 0 ],
           [ 0, fob_inside_dia, 0, 0 ],
           [ 0, 0, fob_inside_h, 0 ],
           [ 0, 0, 0, 1 ] ]
    return multmatrix(m1)(sphere(r=0.5))

def fob_pocket():
    d = 2
    m2 = [ [ fob_inside_dia+2*d, 0, 0, 0 ],
           [ 0, fob_inside_dia+2*d, 0, 0 ],
           [ 0, 0, fob_inside_h+2*d, 0 ],
           [ 0, 0, 0, 1 ] ]

    fob_cover = multmatrix(m2)(sphere(r=0.5))
    fob_cut1 = translate([-(fob_inside_dia+4*d)/2, -fob_inside_dia, -(fob_inside_h*2+2*d+2)/2])(cube([fob_inside_dia+4*d, fob_inside_dia, fob_inside_h*2+2*d+2]))
    fob_cut2 = translate([-(fob_inside_dia+4*d)/2, -fob_inside_dia*.25, -(fob_inside_h*2+2*d+2)])(cube([fob_inside_dia+4*d, fob_inside_dia, fob_inside_h*2+2*d+2]))
    return fob_cover-fob_cut1-fob_cut2

def card_pocket():
    return down(1)(back(1)(left(card_pocket_w/2)(cube([card_pocket_w, card_pocket_h+1, card_pocket_d+1]))))

def half_sphere(r):
    return rotate([-90, 0, 0])(difference()(sphere(r), down(r)(left(r)(cube([r*2,r*2,r*2])))))

def half_cylinder(r):
    return rotate([-90, 0, 0])(difference()(cylinder(r,r), down(r-1)(left(r)(cube([r*2,r*2,r*2])))))

def card_cover():
    d = 2
    r = 2
    card_cover_w = card_pocket_w+2*d
    card_cover_d = card_pocket_d+d
    
    left_s = translate([card_cover_w/2-r, card_pocket_h, 0])(half_sphere(r))
    right_s = translate([-(card_cover_w/2-r), card_pocket_h, 0])(half_sphere(r))

    left_c = translate([card_cover_w/2-r, 0, 0])(half_cylinder(r))
    right_c = translate([-(card_cover_w/2-r), 0, 0])(half_cylinder(r))
    return hull()(left_c+right_c+left_s+right_s)
    #return left(card_cover_w/2)(cube([card_cover_w, card_pocket_h, card_cover_d]))
    

def assembly():
    fp = fob_pocket()
    fh = fob_hole()
    cp = card_pocket()
    cc = card_cover()

    return fp+cc-fh-cp

if __name__ == '__main__':
    a = assembly()
    scad_render_to_file(a, file_header='$fn = %s;' % SEGMENTS, include_orig_code=True)
