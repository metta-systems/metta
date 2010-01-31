package com.exquance.metta.mockups.control;

import com.exquance.metta.mockups.M;
import com.exquance.metta.mockups.Sketch;

import com.exquance.metta.mockups._components_lib.RadialComponent;
import com.exquance.metta.mockups._sketch_controls.Fingers;

public class FingerRepresenter extends RadialComponent {
    
    public static int RIGHT_HAND_ID = 1; // fingers state
    public static int LEFT_HAND_ID  = 2; // fingers state
    
    private static final float r = 30f; // radius 
    
    private final int finger;
    private final int hand;
    private final String label;
    private final int color;
    
    FingerRepresenter(int finger, int hand, String label, int color) {
        this.finger = finger;
        this.hand = hand;
        this.label = label;
        this.color = color;        
    }

    @Override
    public void prepare(Sketch p) {
        // TODO Auto-generated method stub

    }

    @Override
    public void update(Sketch p) {
        p.fill(color);
        p.ellipse(center_x, center_y, r * 2, r * 2);
        p.fill(M.lerpColor(color, M.wcolor(0xffffff), .3f));
        // p.textFont(p.get_cur_font());        
        // p.text(label, center_x - 20, center_y - 20); 
        p.noFill();
    }
    
    @Override
    public void set_pos(float center_x, float center_y) {
        switch (finger) {
            case Fingers.LITTLE: 
                super.set_pos((hand == LEFT_HAND_ID) ? (center_x - (r * 11)) : (center_x + (r * 11)),
                              center_y + r);
                break;
            case Fingers.RING: 
                super.set_pos((hand == LEFT_HAND_ID) ? (center_x - (r * 8)) : (center_x + (r * 8)),
                              center_y - r);
                break;
            case Fingers.MIDDLE: 
                super.set_pos((hand == LEFT_HAND_ID) ? (center_x - (r * 5)) : (center_x + (r * 5)),
                              center_y - r);
                break;
            case Fingers.INDEX: 
                super.set_pos((hand == LEFT_HAND_ID) ? (center_x - (r * 2)) : (center_x + (r * 2)),
                              center_y);
                break;
            case Fingers.THUMB: 
                super.set_pos((hand == LEFT_HAND_ID) ? (center_x - (r * 3)) : (center_x + (r * 3)),
                              center_y + (r * 5));
                break;                
        
        }
        
    }

}
