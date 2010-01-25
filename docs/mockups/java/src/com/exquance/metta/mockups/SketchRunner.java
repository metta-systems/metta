package com.exquance.metta.mockups;

import processing.core.PApplet;

public class SketchRunner {
	
	  public static void main(String args[]) {
	        if (args.length > 0) {
	            System.out.println("arguments passed: " + args.length);
    	        if ((args.length > 1) && (args[1] != null)) {
    	            Sketch.OPENGL_ON = (args[1].equals("true")); 
                    System.out.println("OpenGL enabled: " + Sketch.OPENGL_ON);     	            
    	        }
                if ((args.length > 2) && (args[2] != null)) {
                    Sketch.FONTS_ON = (args[2].equals("true"));
                    System.out.println("fonts enabled: " + Sketch.FONTS_ON);                    
                }                
                if ((args.length > 3) && (args[3] != null)) {
                    Sketch.WIDTH = Integer.parseInt(args[3]);
                    System.out.println("width set to: " + Sketch.WIDTH);                    
                }
                if ((args.length > 4) && (args[4] != null)) {
                    Sketch.HEIGHT = Integer.parseInt(args[4]);
                    System.out.println("width set to: " + Sketch.HEIGHT);                    
                }
    		    PApplet.main(new String[] { "--present", "com.exquance.metta.mockups." + args[0] });
	        } else {
	            System.out.println("arguments: sketch [opengl.on] [fonts.on] [width] [height]");
	        }
	  } 	

}
