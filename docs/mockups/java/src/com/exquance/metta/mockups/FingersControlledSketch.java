package com.exquance.metta.mockups;

import com.exquance.metta.mockups._sketch_controls.FingersState;

@SuppressWarnings("serial")
public class FingersControlledSketch extends Sketch {

    @Override
    public void draw() {
        for (Component component: components) {
            if (component.shown()) {
                component.fingers_move(FingersState.LEFT_HAND, FingersState.RIGHT_HAND, mouseX, mouseY);
            }
        }        
    }    
    
    @Override
    public void mousePressed() {  
        for (Component component: components) {
            if (component.shown()) {
                // FIXME: implement            
            }
        }
    }
    
    
}
