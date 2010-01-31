package com.exquance.metta.mockups.control;

import com.exquance.metta.mockups.M;
import com.exquance.metta.mockups.Sketch;

import com.exquance.metta.mockups._components_lib.RadialComponent;
import com.exquance.metta.mockups._sketch_controls.Fingers;

public class FingerRepresenter extends RadialComponent {
    
    public static int RIGHT_HAND_ID = 1; // fingers state
    public static int LEFT_HAND_ID  = 2; // fingers state
    
    private static final float base_r = 30f; // base radius
    private float r = base_r; // finger radius
    
    private final int finger;
    private final int hand;
    private final String label;
    private final int color;
    
    FingerRepresenter(int finger, int hand, String label, int color) {
        this.finger = finger;
        this.hand = hand;
        this.label = label;
        this.color = (0x00ffffff & color) | 0xcc000000; // set transparency to 0xcc       
    }

    @Override
    public void prepare(Sketch p) {
        r = get_finger_radius(finger, base_r);
    }

    @Override
    public void update(Sketch p) {
        p.fill(color);
        p.ellipse(center_x, center_y, r * 2, r * 2);
        p.fill(M.lerpColor(color, M.wcolor(0xffffff), .3f));
        p.textFont(p.get_cur_font());        
        p.text(label, center_x - 10, center_y + 5); 
        p.noFill();
    }
    
    @Override
    public void set_pos(float center_x, float center_y) {
        switch (finger) {
            case Fingers.LITTLE: 
                super.set_pos((hand == LEFT_HAND_ID) ? (center_x - (base_r * 11)) : (center_x + (base_r * 11)),
                              center_y + base_r);
                break;
            case Fingers.RING: 
                super.set_pos((hand == LEFT_HAND_ID) ? (center_x - (base_r * 8)) : (center_x + (base_r * 8)),
                              center_y - base_r);
                break;
            case Fingers.MIDDLE: 
                super.set_pos((hand == LEFT_HAND_ID) ? (center_x - (base_r * 5)) : (center_x + (base_r * 5)),
                              center_y - base_r);
                break;
            case Fingers.INDEX: 
                super.set_pos((hand == LEFT_HAND_ID) ? (center_x - (base_r * 2)) : (center_x + (base_r * 2)),
                              center_y);
                break;
            case Fingers.THUMB: 
                super.set_pos((hand == LEFT_HAND_ID) ? (center_x - (base_r * 3)) : (center_x + (base_r * 3)),
                              center_y + (base_r * 5));
                break;                
        
        }
        
    }
    
    private static float get_finger_radius(int finger, float base_radius) {
        switch(finger) {
            case Fingers.LITTLE: return (0.7f * base_radius);
            case Fingers.RING:   return (0.8f * base_radius);
            case Fingers.MIDDLE: return (0.9f * base_radius);
            case Fingers.INDEX:  return (0.8f * base_radius);
            case Fingers.THUMB:  return (1.0f * base_radius);
            default: return base_radius;
        }
    }

}
