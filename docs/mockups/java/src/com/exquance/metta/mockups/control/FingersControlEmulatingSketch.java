package com.exquance.metta.mockups.control;

import com.exquance.metta.mockups.Component;
import com.exquance.metta.mockups.M;
import com.exquance.metta.mockups.Sketch;
import com.exquance.metta.mockups._sketch_controls.Fingers;
import com.exquance.metta.mockups._sketch_controls.FingersEmulation;
import com.exquance.metta.mockups._sketch_controls.FingersState;

public class FingersControlEmulatingSketch extends Sketch {

    FingerRepresenter leftLittleFinger;
    FingerRepresenter leftRingFinger;
    FingerRepresenter leftMiddleFinger;
    FingerRepresenter leftIndexFinger;
    FingerRepresenter leftThumb;
    
    FingerRepresenter rightThumb;
    FingerRepresenter rightIndexFinger;
    FingerRepresenter rightMiddleFinger;
    FingerRepresenter rightRingFinger;
    FingerRepresenter rightLittleFinger;
    
    public FingersControlEmulatingSketch() {
        leftLittleFinger = new FingerRepresenter(
                Fingers.LITTLE, FingerRepresenter.LEFT_HAND_ID, 
                String.valueOf(FingersEmulation.LITTLE_KEY), M.wcolor(0x660000));
        add_component(leftLittleFinger);
        leftRingFinger = new FingerRepresenter(
                Fingers.RING, FingerRepresenter.LEFT_HAND_ID, 
                String.valueOf(FingersEmulation.RING_KEY), M.wcolor(0x006600));
        add_component(leftRingFinger);
        leftMiddleFinger  = new FingerRepresenter(
                Fingers.MIDDLE, FingerRepresenter.LEFT_HAND_ID, 
                String.valueOf(FingersEmulation.MIDDLE_KEY), M.wcolor(0x000066));
        add_component(leftRingFinger);        
        leftIndexFinger  = new FingerRepresenter(
                Fingers.INDEX, FingerRepresenter.LEFT_HAND_ID, 
                String.valueOf(FingersEmulation.INDEX_KEY), M.wcolor(0x666600));
        add_component(leftRingFinger);        
        leftThumb = new FingerRepresenter(
                Fingers.THUMB, FingerRepresenter.LEFT_HAND_ID, 
                String.valueOf(FingersEmulation.THUMB_KEY), M.wcolor(0x006666));
        add_component(leftThumb);      
        
        rightLittleFinger = new FingerRepresenter(
                Fingers.LITTLE, FingerRepresenter.RIGHT_HAND_ID, 
                String.valueOf(FingersEmulation.LITTLE_KEY), M.wcolor(0x660000));
        add_component(leftLittleFinger);
        rightRingFinger = new FingerRepresenter(
                Fingers.RING, FingerRepresenter.RIGHT_HAND_ID, 
                String.valueOf(FingersEmulation.RING_KEY), M.wcolor(0x006600));
        add_component(leftRingFinger);
        rightMiddleFinger  = new FingerRepresenter(
                Fingers.MIDDLE, FingerRepresenter.RIGHT_HAND_ID, 
                String.valueOf(FingersEmulation.MIDDLE_KEY), M.wcolor(0x000066));
        add_component(leftRingFinger);        
        rightIndexFinger  = new FingerRepresenter(
                Fingers.INDEX, FingerRepresenter.RIGHT_HAND_ID, 
                String.valueOf(FingersEmulation.INDEX_KEY), M.wcolor(0x666600));
        add_component(leftRingFinger);        
        rightThumb = new FingerRepresenter(
                Fingers.THUMB, FingerRepresenter.RIGHT_HAND_ID, 
                String.valueOf(FingersEmulation.THUMB_KEY), M.wcolor(0x666600));
        add_component(rightThumb);        
    }
    
    @Override
    public void setup() {
        use_client_size();
        smooth();
        frameRate(30);
        
        set_cur_font(loadFont("DejaVuSans-40.vlw"));
      
        super.setup();      
    }
    
    @Override
    public void draw() {
        background(0x999999);
     
        if (FingersState.is_finger_down(Fingers.RING, FingerRepresenter.LEFT_HAND_ID)) {
            leftRingFinger.set_pos(mouseX, mouseY);
        }
        // FIXME: implement        
        
        for (Component component: components) {
            if (component.shown()) {
                component.fingers_move(FingersState.LEFT_HAND, FingersState.RIGHT_HAND, mouseX, mouseY);
            }
        } 
        
        super.draw();
    }
    
    @Override
    public void mousePressed() {
        FingersEmulation.applyState(key, mouseButton);
        super.mousePressed();
    }
    
    @Override
    public void mouseReleased() {
        FingersEmulation.releaseState(mouseButton);
    }    
    
    @Override
    public void keyPressed() {
        FingersEmulation.applyState(key, mouseButton);
        for (Component component: components) {
            if (component.shown()) {
                component.fingers_tap(FingersState.LEFT_HAND, FingersState.RIGHT_HAND, mouseX, mouseY);
            }
        }
        super.keyPressed();
    }
    
    @Override    
    public void keyReleased() {
        FingersEmulation.releaseState(key, mouseButton);
        super.keyReleased();        
    }    
    
}
