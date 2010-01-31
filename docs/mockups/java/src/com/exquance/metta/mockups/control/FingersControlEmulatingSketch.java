package com.exquance.metta.mockups.control;

import java.util.HashMap;
import java.util.Map;

import com.exquance.metta.mockups.M;
import com.exquance.metta.mockups.Sketch;
import com.exquance.metta.mockups._sketch_controls.Fingers;
import com.exquance.metta.mockups._sketch_controls.FingersEmulation;
import com.exquance.metta.mockups._sketch_controls.FingersState;

@SuppressWarnings("serial")
public class FingersControlEmulatingSketch extends Sketch {
    
    private final Map<Integer, FingerRepresenter> leftHand = new HashMap<Integer, FingerRepresenter>();    
    private final Map<Integer, FingerRepresenter> rightHand = new HashMap<Integer, FingerRepresenter>();    

    public FingersControlEmulatingSketch() {
        leftHand.put(Fingers.LITTLE, new FingerRepresenter(
                Fingers.LITTLE, FingerRepresenter.LEFT_HAND_ID, 
                String.valueOf(FingersEmulation.LITTLE_KEY), M.wcolor(0x660000)));
        leftHand.put(Fingers.RING, new FingerRepresenter(
                Fingers.RING, FingerRepresenter.LEFT_HAND_ID, 
                String.valueOf(FingersEmulation.RING_KEY), M.wcolor(0x006600)));
        leftHand.put(Fingers.MIDDLE, new FingerRepresenter(
                Fingers.MIDDLE, FingerRepresenter.LEFT_HAND_ID, 
                String.valueOf(FingersEmulation.MIDDLE_KEY), M.wcolor(0x000066)));
        leftHand.put(Fingers.INDEX, new FingerRepresenter(
                Fingers.INDEX, FingerRepresenter.LEFT_HAND_ID, 
                String.valueOf(FingersEmulation.INDEX_KEY), M.wcolor(0x666600)));
        leftHand.put(Fingers.THUMB, new FingerRepresenter(
                Fingers.THUMB, FingerRepresenter.LEFT_HAND_ID, 
                String.valueOf("_"), M.wcolor(0x006666)));  
        
        rightHand.put(Fingers.LITTLE, new FingerRepresenter(
                Fingers.LITTLE, FingerRepresenter.RIGHT_HAND_ID, 
                String.valueOf(FingersEmulation.LITTLE_KEY), M.wcolor(0x660000)));
        rightHand.put(Fingers.RING, new FingerRepresenter(
                Fingers.RING, FingerRepresenter.RIGHT_HAND_ID, 
                String.valueOf(FingersEmulation.RING_KEY), M.wcolor(0x006600)));
        rightHand.put(Fingers.MIDDLE, new FingerRepresenter(
                Fingers.MIDDLE, FingerRepresenter.RIGHT_HAND_ID, 
                String.valueOf(FingersEmulation.MIDDLE_KEY), M.wcolor(0x000066)));
        rightHand.put(Fingers.INDEX, new FingerRepresenter(
                Fingers.INDEX, FingerRepresenter.RIGHT_HAND_ID, 
                String.valueOf(FingersEmulation.INDEX_KEY), M.wcolor(0x666600)));
        rightHand.put(Fingers.THUMB, new FingerRepresenter(
                Fingers.THUMB, FingerRepresenter.RIGHT_HAND_ID, 
                String.valueOf("_"), M.wcolor(0x006666)));
        
        for (FingerRepresenter finger: leftHand.values()) {
            add_component(finger);
        }
        
        for (FingerRepresenter finger: rightHand.values()) {
            add_component(finger);
        }

    }
    
    @Override
    public void setup() {
        noCursor();
        
        use_client_size();
        
        move_fingers_to(center_x, center_y);
        
        smooth();
        frameRate(30);
        
        set_cur_font(loadFont("DejaVuSans-40.vlw"));
      
        super.setup();      
    }
    
    @Override
    public void draw() {
        background(0x999999);
     
        track_fingers_move(mouseX, mouseY);
        track_fingers_tap(mouseX, mouseY);
        
        super.draw();
    }
    
    @Override
    public void mousePressed() {
        FingersEmulation.applyState(key, mouseButton);
    }
    
    @Override
    public void mouseReleased() {
        FingersEmulation.releaseState(mouseButton);
    }    
    
    @Override
    public void keyPressed() {
        FingersEmulation.applyState(key, mouseButton);
    }
    
    @Override    
    public void keyReleased() {
        FingersEmulation.releaseState(key, mouseButton);
    }    
    
    private void move_fingers_to(float center_x, float center_y) {
        for (Map.Entry<Integer, FingerRepresenter> entry: leftHand.entrySet()) {
            entry.getValue().set_pos(center_x, center_y);
        }
        for (Map.Entry<Integer, FingerRepresenter> entry: rightHand.entrySet()) {
            entry.getValue().set_pos(center_x, center_y);
        }
    }
    
    private void track_fingers_move(float mouseX, float mouseY) {
        for (Map.Entry<Integer, FingerRepresenter> entry: leftHand.entrySet()) {
            if (FingersState.is_finger_down(entry.getKey(), FingerRepresenter.LEFT_HAND_ID)) {
                entry.getValue().set_pos(mouseX, mouseY);
            }
        }
        for (Map.Entry<Integer, FingerRepresenter> entry: rightHand.entrySet()) {
            if (FingersState.is_finger_down(entry.getKey(), FingerRepresenter.RIGHT_HAND_ID)) {
                entry.getValue().set_pos(mouseX, mouseY);
            }
        }
    }
    
    private void track_fingers_tap(float mouseX, float mouseY) {
        for (Map.Entry<Integer, FingerRepresenter> entry: leftHand.entrySet()) {
            entry.getValue().fingers_tap(FingersState.LEFT_HAND, FingersState.RIGHT_HAND, 
                                         mouseX, mouseY);
        }
        for (Map.Entry<Integer, FingerRepresenter> entry: rightHand.entrySet()) {
            entry.getValue().fingers_tap(FingersState.LEFT_HAND, FingersState.RIGHT_HAND, 
                                         mouseX, mouseY);
        }
    }    
    
}
