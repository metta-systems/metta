package com.exquance.metta.mockups;

import processing.core.PApplet;
import processing.core.PFont;

public class Sketch extends PApplet {
    
    private PFont cur_font; 
    
    public Sketch() {
        
    }
    
    public void stretch() {
        size(screen.width - 100, screen.height - 100);        
    }
    
    public void set_cur_font(PFont font) {
        this.cur_font = font;
    }
    
    public PFont get_cur_font() {
        return cur_font;        
    }
    
    public static int mcolor(String col_spec) {
        return 0;
    } 
    
    public static int mcolor(int color) {
        return 0xff000000 | color;
    }
    
    
}
