package com.exquance.metta.mockups._sketch_controls;

import processing.core.PApplet;

public class FingersEmulation {
    
    public static final char THUMB_KEY = ' ';
    public static final char INDEX_KEY = 'f';
    public static final char MIDDLE_KEY = 'e';
    public static final char RING_KEY = 'w';
    public static final char LITTLE_KEY = 'a';
    
    public static void applyState(char key, int mouseBtn) {
        switch (key) {
            case THUMB_KEY:  applyByMouse(Fingers.THUMB,  mouseBtn); break;
            case INDEX_KEY:  applyByMouse(Fingers.INDEX,  mouseBtn); break;
            case MIDDLE_KEY: applyByMouse(Fingers.MIDDLE, mouseBtn); break;
            case RING_KEY:   applyByMouse(Fingers.RING,   mouseBtn); break;
            case LITTLE_KEY: applyByMouse(Fingers.LITTLE, mouseBtn); break;
        }
    }
    
    public static void releaseState(char key, int mouseBtn) {
        switch (key) {
            case THUMB_KEY:  releaseByMouse(Fingers.THUMB,  mouseBtn); break;
            case INDEX_KEY:  releaseByMouse(Fingers.INDEX,  mouseBtn); break;
            case MIDDLE_KEY: releaseByMouse(Fingers.MIDDLE, mouseBtn); break;
            case RING_KEY:   releaseByMouse(Fingers.RING,   mouseBtn); break;
            case LITTLE_KEY: releaseByMouse(Fingers.LITTLE, mouseBtn); break;
        }        
    }
    
    public static void releaseState(int mouseBtn) {
        if (mouseBtn == PApplet.RIGHT) {
            FingersState.RIGHT_HAND = 0;
        } else if (mouseBtn == PApplet.LEFT) {
            FingersState.LEFT_HAND = 0;
        } else if (mouseBtn == PApplet.CENTER) {
            FingersState.LEFT_HAND = 0;
            FingersState.RIGHT_HAND = 0;
        }
    }    
    
    private static void applyByMouse(int finger, int mouseBtn) {
        if (mouseBtn == PApplet.RIGHT) {
            FingersState.RIGHT_HAND |= finger;
        } else if (mouseBtn == PApplet.LEFT) {
            FingersState.LEFT_HAND |= finger;
        } else if (mouseBtn == PApplet.CENTER) {
            FingersState.LEFT_HAND |= finger;
            FingersState.RIGHT_HAND |= finger;
        }
    }
    
    private static void releaseByMouse(int finger, int mouseBtn) {
        if (mouseBtn == PApplet.RIGHT) {
            if ((FingersState.RIGHT_HAND & finger) > 0) FingersState.RIGHT_HAND ^= finger;
        } else if (mouseBtn == PApplet.LEFT) {
            if ((FingersState.LEFT_HAND  & finger) > 0) FingersState.LEFT_HAND  ^= finger;
        } else if (mouseBtn == PApplet.CENTER) {
            if ((FingersState.RIGHT_HAND & finger) > 0) FingersState.RIGHT_HAND ^= finger;
            if ((FingersState.LEFT_HAND  & finger) > 0) FingersState.LEFT_HAND  ^= finger;
        }
    }    

}
