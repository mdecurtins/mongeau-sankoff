MIREX 2007 SYMBOLIC MELODIC SIMILARITY SUBMISSION

Authors: Carlos Gómez, Edna Ruckhaus, Soraya Abad-Mota
Universidad Simón Bolívar
Venezuela

Contact: Carlos Gómez (armarpc at gmail dot com)

0. REQUISITES

The submitted system is intended to run on GNU/Linux. In addition, the
following programs and libraries are required:

- PostgreSQL
- postgresql-libs (they are included in the full installation of 
PostgreSQL)
- Java JDK
- gcc
- bash

A series of preparatory steps need to be done before running the
program. The program is distributed as source, so it must first be
compiled. Also, the program assumes that the collection to be searched
is stored in a PostgreSQL database; therefore, a table must be created
in PostgreSQL to store the collection prior to running the program. In
addition, this table uses an user-defined data type based on compiled C
functions, so this data type must be defined in the database before
creating the table.

Once the table is created, the MIDI files must be read and inserted into
the database. After this the program will be ready to run. In short, the
necessary steps are:

  1. Compile the program.
  2. Create the database.
  3. Create the user-defined data type.
  4. Create the table.
  5. Insert the documents into the table.
  6. Final preparations.
  7. Do some tests.
  8. Run the program.

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

You will probably want to create a database to be used with this
submission. If you want, you can also create a new database user for
this database.

To create a new user in PostgreSQL, type (in a shell)

  $ createuser -U postgres carlos

This will create an user named 'carlos'. 'postgres' is the name of the
Postgres superuser. The user creation script will ask you a series of
questions, you can answer no to all of them.

Now that an user exists who will own the database, the database can be
created. Type

 $ createdb -U postgres -O carlos carlos

The -O option specifies the owner of the database, and the last argument
is the name for the database. It's convenient that the database and
owner have the same name.


3. CREATE THE USER-DEFINED DATA TYPE

A user-defined data type designed to store musical sequences has to be
defined into the database, prior to creating the table that will store
the documents.

This data type is based in C functions. These functions were compiled in
the compilation step, but the resulting library must be moved to a
directory where Postgres can find it. Postgres looks for extension
libraries in the pkglibdir directory, whose path was printed by the
configure script in the first step. Say that the Postgres pkglibdir
directory is /usr/lib/postgresql. Then you would type

  $ mv secuencia/secuencia.so /usr/lib/postgresql

You will likely need root privileges for this. If you don't have root
privileges or do not want to place the file in a system directory, you
can put it in another directory and modify the Postgres configuration
variable dynamic_library_path (through postgresql.conf) to point to this
directory. See section 33.9.1 of the PostgreSQL documentation for more
details.

Now that the library is found in an appropriate directory, we can
actually create the data type. As we'll be defining C functions, this
has to be done as a superuser. Type

  $ psql carlos -U postgres

to connect to database 'carlos' as the user 'postgres'. Then type

  # \i musicseq.sql

This script will create the data type. You will get some warnings saying
that the type definition is only a shell, this is normal.


4. CREATE THE TABLE

After the last step, this one is really easy. Go back to the shell and type

 $ psql carlos -U carlos -f table.sql

This runs a script that creates the table where the documents will be
stored.


5. INSERT THE DOCUMENTS INTO THE DATABASE

A program is included that parses the MIDI files in a specified
directory with the end of inserting them into the database. Type

 $ ./insert <directory>

Where <directory> is the path of the directory that contains the MIDI
files. The program will run for about five minutes (for the Essen
collection).

The program generates a file in the current directory called insert.sql.
This is a script that contains the necessary commands to insert all the
documents into the database. To execute it, type

  $ psql carlos -U carlos -f insert.sql

IMPORTANT NOTE: Please note that this script adds the documents to the
existing table, and that all the documents present in the table will be
searched by the algorithm. Therefore, if you wanted to first test the
algorithm with a small collection, and then do the actual evaluation
with another collection, you should delete the documents that are
present before inserting the second collection. To delete the documents
at any time, type

  $ psql carlos -U carlos
  # delete from document;

This will delete all the documents from the table.


6. FINAL PREPARATIONS

The program is almost ready to run, but first, it might be necessary
that you indicate some parameters to the program on how to connect to
the database server. Open the 'ms_original' file in a text editor. At
the top of the file, you will see the parameters that can be set.
Normally you won't need to specify a password, and the hostname only
needs to be specified if the database server is running on a different
machine.

Please repeat this step for the 'ms_variation' file.


7. DO SOME TESTS

A small test program is included that poses a query to the system and
checks if the results match what is expected. To run it, type

  $ ./testsimple

If everything is working as it should, the program will say 'Test ok'.


8. RUN THE PROGRAM

This submission comprises two algorithms, or rather, two variations of
the same algorithm. To run the first one, type

  $ ./ms_original <query> <output file>

Where <query> is the name of a MIDI file containing the query and
<output file> is where the result list will be written.

To run the second program, type

  $ ./ms_variation <query> <output file>

The running time is in the order of 40 seconds to 3 minutes per query.
