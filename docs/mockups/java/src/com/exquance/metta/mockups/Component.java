package com.exquance.metta.mockups;

import java.util.ArrayList;
import java.util.List;

import processing.core.PConstants;

public abstract class Component implements PConstants, ActionsListener {
    
    public static final float RADIUS_UNSET = Float.MIN_VALUE;
    public static final float ANGLE_UNSET = Float.MIN_VALUE;    
    public static final int COLOR_UNSET = Integer.MIN_VALUE;    

    List<ActionsListener> listeners;
    
    private boolean shown = false;
    
    public Component() { 
        listeners = new ArrayList<ActionsListener>();
        this.show();
    }
    
    public void add_listener(ActionsListener listener) {
        listeners.add(listener);
    };  
    
    public <T extends ActionsListener> void add_listeners(List<T> listeners) {
        for (T listener: listeners) {
            listeners.add(listener);
        }
    };
    
    public void mouse_move(float mouse_x, float mouse_y) {
        for (ActionsListener listener: listeners) {
            if (listener.is_mouse_in(mouse_x, mouse_y)) {
                if (!listener.on_mouse_over(mouse_x, mouse_y)) return;
            } else {
                if (!listener.on_mouse_out()) return;
            }
        }
    }      
    
    public void mouse_click(float mouse_x, float mouse_y) { 
        for (ActionsListener listener: listeners) {
            if (listener.is_mouse_in(mouse_x, mouse_y)) {
                if (!listener.on_mouse_click(mouse_x, mouse_y)) return;
            }
        }
    };
    
    public void show() {
        this.shown = true;
    }    
    
    public void hide() {
        this.shown = false;
    }    
    
    public boolean shown() {
        return this.shown;
    }
    
    public boolean is_mouse_in(float mouse_x, float mouse_y) { return false; };    
    public boolean on_mouse_over(float mouse_x, float mouse_y) { return true; };
    public boolean on_mouse_click(float mouse_x, float mouse_y) { return true; };
    public boolean on_mouse_out() { return true; };
    public abstract void prepare(Sketch p);    
    public abstract void update(Sketch p);
    
    
}
