package com.exquance.metta.mockups;

import processing.core.PApplet;

public final class M {
    
    public static int mcolor(String col_spec) {
        return 0;
    } 
    
    public static int wcolor(int color) {
        return 0xff000000 | color;
    }   
    
    public static float abs(float val) {
        return PApplet.abs(val);
    }
    
    public static float atan2(float y, float x) {
        return PApplet.atan2(y, x);
    }
    
    public static float cos(float angle) {
        return PApplet.cos(angle);
    }
    
    public static float sin(float angle) {
        return PApplet.sin(angle);        
    }    
    
    public static int lerpColor(int c1, int c2, float amt) {
        return PApplet.lerpColor(c1, c2, amt, PApplet.RGB);
    }

}
