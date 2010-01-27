package com.exquance.metta.mockups.control;

import com.exquance.metta.mockups.Sketch;

import com.exquance.metta.mockups._components_lib.RadialComponent;

public class FingerRepresenter extends RadialComponent {
    
    public static int RIGHT_HAND_ID = 1; // fingers state
    public static int LEFT_HAND_ID  = 2; // fingers state    
    
    private final int finger;
    private final String label;
    private final int color;
    
    FingerRepresenter(int finger, int hand, String label, int color) {
        this.finger = finger;
        this.label = label;
        this.color = color;        
    }

    @Override
    public void prepare(Sketch p) {
        // TODO Auto-generated method stub

    }

    @Override
    public void update(Sketch p) {
        // TODO Auto-generated method stub

    }

}
