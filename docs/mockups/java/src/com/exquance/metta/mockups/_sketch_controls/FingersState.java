package com.exquance.metta.mockups._sketch_controls;

public class FingersState {
    
    public static byte RIGHT_HAND = 0; // fingers state
    public static byte LEFT_HAND = 0; // fingers state
    
    public static boolean is_finger_down(int finger, int hand_id) {
        if (hand_id == Hands.LEFT_HAND_ID) {
            return (finger & LEFT_HAND) > 0;
        } else if (hand_id == Hands.RIGHT_HAND_ID) {
            return (finger & RIGHT_HAND) > 0;            
        }
        return false;
    }
    
    public static void set_finger_down(int finger, int hand_id) {
        if (hand_id == Hands.LEFT_HAND_ID) {
            LEFT_HAND |= finger;
        } else if (hand_id == Hands.RIGHT_HAND_ID) {
            RIGHT_HAND |= finger;            
        }
    }

}
