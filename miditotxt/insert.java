/* insert.java
 *
 * This program reads the MIDI files in the specified directory and 
 * generates a file called insert.sql which contains the necessary 
 * commands to insert all the documents read into a database.
 */

import javax.sound.midi.*;
import java.io.*;

class MyFilter implements FilenameFilter {

    public boolean accept(File dir, String name) {
        return name.endsWith(".mid");
    }
}

class insert {
    
    public static void main(String[] args) throws 
        InvalidMidiDataException, IOException {
        int argc = args.length;
        if (argc != 1) {
            System.out.println("\nUsage: insert <directory>\n");
            return;
        }
        File directory = new File(args[0]);
        if (!(directory.isDirectory())) {
            System.out.println(args[0] + ": not a directory\n");
            return;
        }
        File[] files = directory.listFiles(new MyFilter());
        String output = "DELETE from document;\n";
        System.out.println("Parsing MIDI files...");
        for (int i = 0; i < files.length; i++) {
            Sequence seq = MidiSystem.getSequence(files[i]);
            String file_name = files[i].getName();
            output += "INSERT INTO document (filename, content) " + 
                "VALUES ('" + file_name + "', '"  + 
                miditxt.convertir(seq) + "');\n";
        }
        System.out.println("Writing insert.sql...");
        BufferedWriter writer = new BufferedWriter(new FileWriter("insert.sql"));
        writer.write(output);
        writer.close();
    }
}
