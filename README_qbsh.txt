MIREX 2007 QUERY BY SINGING/HUMMING SUBMISSION

Authors: Carlos Gómez, Edna Ruckhaus, Soraya Abad-Mota
Universidad Simón Bolívar
Venezuela

Copyright (C) 2007 Carlos Gómez

Contact: Carlos Gómez (cgomez@ldc.usb.ve, armarpc@gmail.com)

This submission is very similar to the Symbolic Similarity submission that was sent. However, some programs have been added or modified, therefore it's necessary (or at least recommended) to follow the compilation and installation steps present in this file. The setup procedure in this file is simpler, because it assumes that some steps have already been made (for example, the database doesn't need to be created again).

This submission encompasses only a note matcher (and the necessary program for indexing the collection).

0. REQUISITES

The submitted system is intended to run on GNU/Linux. In addition, the
following programs and libraries are required:

- PostgreSQL
- postgresql-libs (they are included in the full installation of 
PostgreSQL)
- Java JDK
- gcc
- bash

The setup steps are:

  1. Compile the program.
  2. Create the database.
  3. Create the user-defined data type.
  4. Create the table.
  5. Final preparations.
  6. Run the program.

In the sections that follow, each step is explained in detail.


1. COMPILE THE PROGRAM

Run the following commands in a bash shell:

  $ tar -xvzf smsCarlosGomez.tar.gz
  $ cd smsCarlosGomez
  $ ./configure
  $ make

The configure script is very simple; it determines the location of the
PostgreSQL libraries in your system. At the end of the configure output,
the location of Postgres's pkglibdir will be printed -- you will need
this information in the third step.

Some warnings will show up during the compilation complaining about
uninitialized variables, this is normal.


2. CREATE THE DATABASE

It's not necessary to create a new database; the database used for the Symbolic Similarity submission can be used. 


3. CREATE THE USER-DEFINED DATA TYPE

As the code of the user-defined data type was updated since the Symbolic Similarity submission (to work with larger MIDI files), it's necessary to copy the updated library to the Postgres pkglibdir directory. Say that the pkglibdir directory is /usr/lib/postgresql, you would type

  $ mv secuencia/secuencia.so /usr/lib/postgresql

You will likely need root privileges for this. If you don't have root
privileges or do not want to place the file in a system directory, you
can put it in another directory and modify the Postgres configuration
variable dynamic_library_path (through postgresql.conf) to point to this
directory. See section 33.9.1 of the PostgreSQL documentation for more
details.


4. CREATE THE TABLE

The table is the same that for the Symbolic Similarity submission, so it's not necessary to repeat this step.


5. FINAL PREPARATIONS

The program is almost ready to run, but first, it might be necessary
that you indicate some parameters to the program on how to connect to
the database server. Open the 'note_matcher' file in a text editor. At
the top of the file, you will see the parameters that can be set.
Normally you won't need to specify a password, and the hostname only
needs to be specified if the database server is running on a different
machine.


6. RUN THE PROGRAM

- Indexing:

A program is included that parses the MIDI files in a specified
directory with the end of inserting them into the database. Type

 $ ./indexing <directory>

Where <directory> is the path of the directory that contains the MIDI
files. 

The program generates a file in the current directory called insert.sql.
This is a script that contains the necessary commands to insert all the
documents into the database. To execute it, type

  $ psql carlos -U carlos -f insert.sql

The script takes care of deleting the existing documents in the database prior to inserting. 

- Note matcher:

  $ ./note_matcher <format> <queries directory> <output_file>

<format> is either "note" or "midi", and indicates whether the queries are .note files or MIDI files, respectively.
<queries directory> is a directory containing the queries. It doesn't matter if the directory contains other files, only the files with the corresponding extension will be read.
<output_file> is where the results will be written.
