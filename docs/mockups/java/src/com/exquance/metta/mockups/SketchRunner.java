package com.exquance.metta.mockups;

import processing.core.PApplet;

public class SketchRunner {
	
	  public static void main(String args[]) {
		    PApplet.main(new String[] { "--present", "com.exquance.metta.mockups." + args[0] });
	  } 	

}
