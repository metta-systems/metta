package com.exquance.metta.mockups;

import java.util.ArrayList;
import java.util.List;

import processing.core.PApplet;
import processing.core.PFont;

public class Sketch extends PApplet/*, Component*/ {
    
    private PFont cur_font; 
    
    private List<Component> components;
    
    public Sketch() {
        components = new ArrayList<Component>();
    }
    
    protected void add_component(Component component) {
        components.add(component);
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
    
    @Override
    public void setup() {
        for (Component component: components) {
             component.prepare(this);
        }
    }
    
    @Override
    public void draw() {
        for (Component component: components) {
            if (component.shown()) {
                component.mouse_move(mouseX, mouseY);
                component.update(this);
            }
        }        
    }
    
    @Override
    public void mousePressed() {
        for (Component component: components) {
            if (component.shown()) {
                component.mouse_click(mouseX, mouseY);            
            }
        }
    }    
    
    
}
