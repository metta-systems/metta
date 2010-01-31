package com.exquance.metta.mockups;

import java.util.ArrayList;
import java.util.List;

import com.exquance.metta.mockups._sketch_controls.FingersListener;
import com.exquance.metta.mockups._sketch_controls.FingersReactor;
import com.exquance.metta.mockups._sketch_controls.MouseListener;
import com.exquance.metta.mockups._sketch_controls.MouseReactor;

import processing.core.PConstants;

public abstract class Component implements PConstants, MouseListener, MouseReactor, 
                                                       FingersListener, FingersReactor {
    
    public static final float RADIUS_UNSET = Float.MIN_VALUE;
    public static final float ANGLE_UNSET = Float.MIN_VALUE;    
    public static final int COLOR_UNSET = Integer.MIN_VALUE;
    
    /* private static final int[] fingers = new int[] {Fingers.THUMB, Fingers.INDEX, 
                                                       Fingers.MIDDLE, Fingers.RING, 
                                                       Fingers.LITTLE}; */

    List<MouseListener> mlisteners;
    List<FingersListener> flisteners;    
    
    private boolean shown = false;
    
    public Component() { 
        mlisteners = new ArrayList<MouseListener>();
        flisteners = new ArrayList<FingersListener>();         
        this.show();
    }
    
    @Override
    public void add_mouse_listener(MouseListener listener) {
        mlisteners.add(listener);
    };
    
    @Override
    public void add_fingers_listener(FingersListener listener) {
        flisteners.add(listener);        
    }    
    
    @Override
    public <T extends MouseListener> void add_mouse_listeners(List<T> listeners) {
        for (T listener: listeners) {
            listeners.add(listener);
        }
    };
    
    @Override
    public <T extends FingersListener> void add_fingers_listeners(List<T> listeners) {
        for (T listener: listeners) {
            listeners.add(listener);
        }
    };    
    
    @Override
    public void mouse_move(float mouse_x, float mouse_y) {
        for (MouseListener mlistener: mlisteners) {
            if (mlistener.is_mouse_in(mouse_x, mouse_y)) {
                if (!mlistener.on_mouse_over(mouse_x, mouse_y)) return;
            } else {
                if (!mlistener.on_mouse_out()) return;
            }
        }
    }      
    
    @Override
    public void mouse_click(float mouse_x, float mouse_y) { 
        for (MouseListener mlistener: mlisteners) {
            if (mlistener.is_mouse_in(mouse_x, mouse_y)) {
                if (!mlistener.on_mouse_click(mouse_x, mouse_y)) return;
            }
        }
    };
    
    @Override
    public void fingers_move(byte left_hand, byte right_hand, float x, float y) {
        for (FingersListener flistener: flisteners) {
            if (flistener.is_finger_in(x, y) && flistener.accepts_fingers(left_hand, right_hand)) {
                if (!flistener.on_fingers_over(left_hand, right_hand)) return;
            } else {
                if (!flistener.on_fingers_out()) return;
            }
        }
    }

    @Override
    public void fingers_tap(byte left_hand, byte right_hand, float x, float y) {
        for (FingersListener flistener: flisteners) {
            if (flistener.is_finger_in(x, y) && flistener.accepts_fingers(left_hand, right_hand)) {
                if (!flistener.on_fingers_tap(left_hand, right_hand)) return;
            }
        }         
    }
    
    
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
    
    public boolean can_be_gragged(byte leftHand, byte rightHand) { return false; }
    public boolean is_finger_in(float fingerX, float fingerY) { return false; }
    @Override
    public boolean accepts_fingers(byte leftHand, byte rightHand) { return false; }
    
    public boolean on_fingers_over(byte leftHand, byte rightHand) { return false; }
    public boolean on_fingers_out() { return false; }    
    public boolean on_fingers_tap(byte leftHand, byte rightHand) { return false; }
    // public boolean on_finger_release(int finger, int arm) { return false; }    
    // public boolean on_fingers_drag(byte leftHand, byte rightHand) { return false; }
    // public boolean on_firgers_drop() { return false; }    
    
    public abstract void prepare(Sketch p);    
    public abstract void update(Sketch p);    
    
}
