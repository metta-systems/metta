package com.exquance.metta.mockups.menu;

import java.util.ArrayList;
import java.util.List;

import processing.core.PConstants;

import com.exquance.metta.mockups.Sketch;

public abstract class Component implements PConstants {
    
    public static final float RADIUS_UNSET = Float.MIN_VALUE;
    public static final float ANGLE_UNSET = Float.MIN_VALUE;    
    public static final int COLOR_UNSET = Integer.MAX_VALUE;    

    public Sketch p;
    
    ArrayList<Component> listeners = new ArrayList<Component>(); // TODO: static?
    
    public Component(Sketch p) {
        this.p = p;
    }
    
    public void add_listener(Component comp) {
        listeners.add(comp);
    };  
    
    public void add_listeners(List<? extends Component> comps) {
        for (int i = 0; i < comps.size(); i++) {
            listeners.add(comps.get(i));
        }
    };
    
    public void mouse_move(float mouse_x, float mouse_y) {
        for (int i = 0; i < listeners.size(); i++) {
            if (listeners.get(i).is_mouse_in(mouse_x, mouse_y)) {
                if (!listeners.get(i).on_mouse_over(mouse_x, mouse_y)) return;
            } else {
                if (!listeners.get(i).on_mouse_out()) return;
            }
        }
    }      
    
    public void mouse_click(float mouse_x, float mouse_y) { 
        for (int i = 0; i < listeners.size(); i++) {
            if (listeners.get(i).is_mouse_in(mouse_x, mouse_y)) {
                if (!listeners.get(i).on_mouse_click(mouse_x, mouse_y)) return;
            }
        }
    };
    
    public boolean is_mouse_in(float mouse_x, float mouse_y) { return false; };    
    public boolean on_mouse_over(float mouse_x, float mouse_y) { return true; };
    public boolean on_mouse_click(float mouse_x, float mouse_y) { return true; };
    public boolean on_mouse_out() { return true; };    
    public abstract void update();    
    
    
}
