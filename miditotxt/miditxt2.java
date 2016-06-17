// miditxt.java

import javax.sound.midi.*;
import java.io.File;
import java.io.IOException;

// TODO probar archivos con cambio de tempo
// archivos con tiempo en SMTPE

class miditxt {
    
    static String escala[] = {"do", "#do", "re", "#re", "mi", "fa", "#fa", "sol", 
                       "#sol", "la", "#la", "si"};

    static final int SET_TEMPO = 0x51;

    static final double default_tempo = 60000000.0 / 120.0;
    
    static long calcularTiempo(long duracion, int upq, float tempo) {
        return Math.round((duracion * 480.0 / upq) * tempo / default_tempo);
    }

    static int intVal(byte[] data) {
        int val = 0;
        for (int i = 0; i < data.length; i++) {
            val += data[data.length - i - 1] * Math.pow(256, i);
        }
        return val;
    }

    static String convertir(Sequence seq) throws InvalidMidiDataException {
        String s = "";
        int ok = 0;
        int ultimo_evnt = 1;
        int chan = -1;
        String not = "";
        long tiempo_ant = 0;
        int num = 0;
        long tiempo_off = 0;
        float tempo = 60000000 / 120;
        int upq = seq.getResolution();
        Track[] tracks = seq.getTracks();
        int l = tracks.length;
        if (l > 1) {
            //System.out.println("Warning: el archivo contiene varios tracks");
        } else if (l == 0) {
            return "";
        }
        Track track = null;
        int max = -1;
        for (int i = 0; i < tracks.length; i++) {
            if (tracks[i].size() > max) {
                track = tracks[i];
                max = tracks[i].size();
            }
        }
        for (int i = 0; i < track.size(); i++) {
            MidiEvent e = track.get(i);
            MidiMessage m = e.getMessage();
            if (m instanceof MetaMessage) {
                MetaMessage mm = (MetaMessage)m;
                if (mm.getType() == SET_TEMPO) {
                    tempo = intVal(mm.getData());
                }
            } else if (m instanceof ShortMessage) {
                ShortMessage sm = (ShortMessage)m;
                if (chan == -1) {
                    chan = sm.getChannel() ;
                } else {
                    if (chan != sm.getChannel()) {
                        System.out.println("Warning: el archivo contiene " +
                                           "eventos en varios canales");
                        chan = sm.getChannel();
                    }
                }
                if (sm.getCommand() == ShortMessage.NOTE_ON) {
                    ok = 1;
                    ultimo_evnt = 1;
                    if (num > 0) {
                        long duracion = e.getTick() - tiempo_ant;
                        duracion = calcularTiempo(duracion, upq, tempo);
                        s += not + duracion + " ";
                    }
                    int nota = sm.getData1();
                    int grado = nota % 12;
                    int octava = (nota / 12) - 5;
                    String c = "^";
                    if (octava < 0) {
                        c = ",";
                        octava = -octava;
                    }
                    String s_octava = "";
                    for (int j = 0; j < octava; j++) {
                        s_octava += c;
                    }
                    not = escala[grado] + s_octava;
                    tiempo_ant = e.getTick();
                    num++;
                } else if (sm.getCommand() == ShortMessage.NOTE_OFF) {
                    ultimo_evnt = 0;
                    tiempo_off = e.getTick();
                }
            }
        }
        // escribir la ultima nota
        if ((ok == 1) && (ultimo_evnt == 0)) {
            long duracion = tiempo_off - tiempo_ant;
            duracion = calcularTiempo(duracion, upq, tempo);
            s += not + duracion;
        }
        return s;
    }

    public static void main(String[] args) throws 
        InvalidMidiDataException, IOException {
        int argc = args.length;
        boolean nl = false;
        int i = 0;
        if (argc < 1) {
            System.out.println("\nUso: miditxt [-n] <archivo>\n");
            return;
        }
        if (args[0].equals("-n")) {
            nl = true;
            i = 1;
            if (argc == 1) {
                System.out.println("\nUso: miditxt [-n] <archivo>\n");
                return;
            }
        }
        for (; i < argc; i++) {
            String sarchivo = args[i];
            File archivo = new File(sarchivo);
            Sequence seq = MidiSystem.getSequence(archivo);
            System.out.print(convertir(seq));
            if (nl) System.out.print("\n");
        }
    }
}
