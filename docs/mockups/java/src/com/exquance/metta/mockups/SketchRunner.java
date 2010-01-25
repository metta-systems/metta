package com.exquance.metta.mockups;

import processing.core.PApplet;

public class SketchRunner {
	
	  public static void main(String args[]) {
	        if (args.length > 0) {
    	        if ((args.length > 1) && (args[1] != null)) {
    	            Sketch.OPENGL_ON = (args[1].equals("true")); 
                    System.out.println("OpenGL enabled: " + Sketch.OPENGL_ON);     	            
    	        }
                if ((args.length > 2) && (args[2] != null)) {
                    Sketch.FONTS_ON = (args[2].equals("true"));
                    System.out.println("fonts enabled: " + Sketch.FONTS_ON);                    
                }	        
                // TODO: set width and height also
    		    PApplet.main(new String[] { "--present", "com.exquance.metta.mockups." + args[0] });
	        } else {
	            System.out.println("arguments: sketch [opengl.on] [fonts.on]");
	        }
	  } 	

}
