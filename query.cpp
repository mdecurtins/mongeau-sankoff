/* query.cpp
 * Main program which allows running the M-S algorithm. The program 
 * retrieves a collection of documents from a database and applies
 * the algorithm on this collection, for a given query. It is also 
 * possible to compare the query to a single document, if specified.
 * 
 *
 * History:
 * 2008-07-19 Added code to normalize scores to the [0, 100] range.
 *
 * Copyright (C) 2006 Carlos Gómez
 * Universidad Simón Bolívar
 */

#include "MS.h"
#include "JCS.h"
#include "parsemusic.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include <list>
#include <libpq-fe.h>

#define ALG_MS 0
#define ALG_JCS 1

using namespace std;

int v_level; // program verbosity level

// which information is in which column of the SELECT result relation
const int ColComposer = 0;
const int ColTitle = 1;
const int ColFilename = 2;
const int ColContent = 3;
const int ColNumber = 4;

/* Exception that is thrown when an unexpected 
 * value is found parsing the program options.
 */
class InvalidProgramArgument {
public:
    string desc;
    InvalidProgramArgument(string desc_) : desc(desc_) {};
};

/* Parses an integer from a C string. */
long strl(char *s) throw(InvalidProgramArgument) {
    istringstream is(s);
    long x;
    char c;
    if (!(is >> x) || is.get(c)) {
        string str(s);
        throw InvalidProgramArgument(str + ": expected integer value");
    }
    return x;
}

/* Parses a float from a C string. */
float strf(char *s) throw(InvalidProgramArgument) {
    istringstream is(s);
    float f;
    char c;
    if (!(is >> f) || is.get(c)) {
        string str(s);
        throw InvalidProgramArgument(str + ": expected numeric value");
    }
    return f;
}

/* Converts a float to a string. */
string floatStr(float f) {
    stringstream ss;
    ss.precision(3);
    ss << f;
    return ss.str();
}

/* Removes the characters of the string after the n position. */
string trim(string s, int n) {
    if ((unsigned int)n < s.length()) {
        return s.substr(0, n);
    } else {
        return s;
    }
}

/* Prints a textual representation of the alignment between two sequences. */
string printAlignment(Sequence& q, Sequence& d, Result& result) {
    stringstream s_query, s_document, s_alignment;
    s_query.setf(ios::left);
    s_document.setf(ios::left);
    int i = result.queryStart + 1;
    int j = result.documentStart + 1;
    list<char>::iterator iter = result.solution.begin();
    int column = 0;
    for (; iter != result.solution.end(); iter++) {
        char op = *iter;
        char *space = "         ";
        //cout << i << "\n";
        //cout.flush();
        if (op == Insertion) {
            s_query.width(8);
            s_query << q(i).print();
            s_document << space;
            s_query << " ";
            i++;
        }
        else if (op == Deletion) {
            s_query << space;
            s_document.width(8);
            s_document << d(j).print();
            s_document << " ";
            j++;
        }
        else if (op == Replacement) {
            s_query.width(8);
            s_query << q(i).print();
            s_document.width(8);
            s_document << d(j).print();
            s_query << " ";
            s_document << " ";
            i++;
            j++;
        }
        else if ((op >= Fragmentation) && (op < Consolidation)) {
            int k = op - Fragmentation;
            cout << "k " << k << endl;
            for (int l = 0; l < k; l++) {
                s_query.width(8);
                s_query << q(i + l).print();
                s_query << " ";
            }
            s_document.width(8);
            s_document << d(j).print();
            s_document << " ";
            for (int l = 0; l < k - 1; l++) {
                s_document << space;
            }
            i += k;
            j++;
        }
        else if ((op >= Consolidation) && (op < 60)) {
            int k = op - Consolidation;
            s_query.width(8);
            s_query << q(i).print();
            s_query << " ";
            for (int l = 0; l < k - 1; l++) {
                s_query << space;
            }
            for (int l = 0; l < k; l++) {
                s_document.width(8);
                s_document << d(j + l).print();
                s_document << " ";
            }
            i++;
            j += k;
        }
        else {
            assert(false);
        }
        if (column == 7) {
            s_alignment << s_query.str() << "\n" << s_document.str() << "\n\n";
            s_query.str("");
            s_document.str("");
            column = 0;
        }
        column++;
    }
    s_alignment << s_query.str() << "\n" << s_document.str();
    return s_alignment.str();
}

string printSolution(list<char>& sol) {
    stringstream s;
    list<char>::iterator iter = sol.begin();
    for (; iter != sol.end(); iter++) {
        char op = *iter;
        if (op == Insertion) {
            s << "ins";
        } else if (op == Deletion) {
            s << "del";
        } else if (op == Replacement) {
            s << "repl";
        } else if ((op >= Fragmentation) && (op < Consolidation)) {
            int k = op - Fragmentation;
            s << "frag(" << k << ")";
        } else if ((op >= Consolidation) && (op < 60)) {
            int k = op - Consolidation;
            s << "cons(" << k << ")";
        }
        s << " ";
    }
    return s.str();
}

inline float normalize(float score, int algorithm, float min_score, 
                       float max_score, Alignment alignment) {
    if (algorithm != ALG_MS)
        return score;
    if (alignment == Global) {
        return 100 + score / 100;  // HACK Couldn't come up with a more logical 
                                   // formula
    } else {
        if (score < 0) 
            return 0;
        else
            return (score * 100) / max_score;
    }
}

/* Main function. This reads the program arguments, retrieves a collection 
 * of documents from a database server, creates an instance of the algorithm 
 * and invokes the algorithm. 
 */
int main(int argc, char *argv[]) {
    char *usage =
        "Usage: query [options] <query>\n" \
        "       query [options] <query> <document>\n\n" \
        "This program finds the given query in the database. If a document is also given,\n" \
        "the program only compares the query to the document and shows the resulting \n" \
        "score.\n\n"
        "General options:\n"
        "-a <algorithm>      algorithm, either ms or jcs. ms is used by default.\n"\
        "-align <align type> this should be one of global, start, varlenstart, \n"
        "                    alltosome, overlap or local. The default is local.\n"\
        "-mirex              Output results in MIREX Symbolic Similarity format, that is,\n" \
        "                    only show the matching file name.\n" \
        "-n <n>              Number of results to return. The default is 10.\n" \
        "-output <file>      Output the results to a file instead of standard output.\n" \
        "-qbsh               Output results in MIREX QBSH format (that is, in one line).\n" \
        "-q                  Quiet. Only print the score (if comparing two documents).\n" \
        "-v <level>          verbosity level from 0 to 3. The default is 0.\n" \
        "\n" \
        "Mongeau-Sankoff options:\n" \
        "-k1 <value>         k1 constant. The default is 0.36.\n" \
        "-k2 <value>         k2 constant. The default is 0.24.\n" \
        "-k3 <value>         k3 constant. If not specified, k3 = k1.\n" \
        "-f <F>              F constant (maximum length of a fragmentation).\n" \
        "-c <C>              C constant (maximum length of a consolidation).\n" \
        "-modulo12           Use modulo-12 representation.\n" \
        "-shortest           When using modulo-12 representation, take the smallest \n" \
        "                    difference between pitches.\n" \
        "-noinsdel           Do not consider insertion and deletion operations.\n" \
        "\n" \
        "Database connection options:\n" \
        "-user <user>        Database username. By default, connect as the user with the\n" \
        "                    same name as the shell current user.\n" \
        "-password <pwd>     Connection password (normally not needed).\n" \
        "-database <db>      Database name. By default, connect to the database with the\n" \
        "                    same name as the user.\n" \
        "-host <name>        Host name where the database server is running. Defaults to\n" \
        "                    the current machine.\n" \
        "\n";
    if (argc < 2) {
        cout << usage;
        return 0;
    }
    v_level = 0;
    // process program arguments
    int i_algorithm = ALG_MS;
    int k = 10;
    Alignment alignment = Local;
    bool b_alignment = false;
    bool extended = true; // show extended results 
    bool quiet = false;
    bool qbsh = false;
    bool ins_del = true;
    float k1 = 0.36;
    float k2 = 0.24;
    float k3 = -1; // THIS LINE SHOULD NOT BE CHANGED
    int F = 2;
    int C = 2;
    bool modulo12 = false;
    bool shortest_distance = false;
    string collection = "MIREX2005";
    float sigma_init_s = 1.5;
    float sigma_k = 0.28;
    float sigma_s = 0.263;
    float sigma_pitch_e = 0.422;
    float sigma_rhythm_e = 0.55;
    float p_noedit = 0.95;
    float p_join = 0.03;
    char *output_file = NULL;
    char *s_user = NULL;
    char *s_password = NULL;
    char *s_host = NULL;
    char *s_database = NULL;
    int i;
    for (i = 1; i < argc; i++) {
        try {
            if (strcmp(argv[i], "-v") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                long l = strl(argv[i + 1]);
                if ((l < 0) || (l > 3)) {
                    cout << "\n" << "-v must be between 0 and 3\n\n";
                    return 0;
                }
                v_level = (int)l;
                i++;
            } else if (strcmp(argv[i], "-a") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                char *s_algorithm = argv[i + 1];
                if (strcasecmp(s_algorithm, "ms") == 0) {
                    i_algorithm = ALG_MS;
                } else if (strcasecmp(s_algorithm, "jcs") == 0) {
                    i_algorithm = ALG_JCS;
                } else {
                    cout << "\nthe algorithm must be either ms or jcs\n\n";
                    return 0;
                }
                i++;
            } else if (strcmp(argv[i], "-align") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                char *s_align = argv[i + 1];
                if (strcasecmp(s_align, "global") == 0) {
                    b_alignment = true;
                    alignment = Global;
                } else if (strcasecmp(s_align, "start") == 0) {
                    b_alignment = true;
                    alignment = Start;
                } else if (strcasecmp(s_align, "varlenstart") == 0) {
                    b_alignment = true;
                    alignment = VarLenStart;
                } else if (strcasecmp(s_align, "alltosome") == 0) {
                    b_alignment = true;
                    alignment = AllToSome;
                } else if (strcasecmp(s_align, "overlap") == 0) {
                    b_alignment = true;
                    alignment = Overlap;
                } else if (strcasecmp(s_align, "local") == 0) {
                    b_alignment = true;
                    alignment = Local;
                } else {
                    cout << "\n" <<
                        "alignment must be one of global, start, varlenstart, " \
                        "alltosome, overlap or local\n\n";
                    return 0;
                }
                i++;
            } else if (strcmp(argv[i], "-mirex") == 0) {
                extended = false;
            } else if (strcmp(argv[i], "-eval") == 0) {
                collection = "MIREX2005EVAL";
            } else if (strcmp(argv[i], "-n") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                long l = strl(argv[i + 1]);
                if (l < 1) {
                    cout << "\n" << "-n must be positive\n\n";
                    return 0;
                }
                k = l;
                i++;
            } else if (strcmp(argv[i], "-k1") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                float f = strf(argv[i + 1]);
                if (f < 0) {
                    cout << "\n" << "-k1 must be non-negative\n\n";
                    return 0;
                }
                k1 = f;
                i++;
            } else if (strcmp(argv[i], "-k2") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                float f = strf(argv[i + 1]);
                if (f < 0) {
                    cout << "\n" << "-k2 must be non-negative\n\n";
                    return 0;
                }
                k2 = f;
                i++;
            } else if (strcmp(argv[i], "-k3") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                float f = strf(argv[i + 1]);
                if (f < 0) {
                    cout << "\n" << "-k3 must be non-negative\n\n";
                    return 0;
                }
                k3 = f;
                i++;
            } else if (strcmp(argv[i], "-f") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                long l = strl(argv[i + 1]);
                if ((l < 1) || (l > 20)) {
                    cout << "\n" << "-f must be between 1 and 20\n\n";
                    return 0;
                }
                F = l;
                i++;
            } else if (strcmp(argv[i], "-c") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                long l = strl(argv[i + 1]);
                if ((l < 1) || (l > 20)) {
                    cout << "\n" << "-c must be between 1 and 20\n\n";
                    return 0;
                }
                C = l;
                i++;
            } else if (strcmp(argv[i], "-noinsdel") == 0) {
                ins_del = false;
            } else if (strcmp(argv[i], "-modulo12") == 0) {
                modulo12 = true;
            } else if (strcmp(argv[i], "-shortest") == 0) {
                shortest_distance = true;
            } else if (strcmp(argv[i], "-sigmak") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                float f = strf(argv[i + 1]);
                if (f < 0) {
                    cout << "\n" << "-sigmak must be non-negative\n\n";
                    return 0;
                }
                sigma_k = f;
                i++;
            } else if (strcmp(argv[i], "-sigmas") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                float f = strf(argv[i + 1]);
                if (f < 0) {
                    cout << "\n" << "-sigmas must be non-negative\n\n";
                    return 0;
                }
                sigma_s = f;
                i++;
            } else if (strcmp(argv[i], "-sigmainits") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                float f = strf(argv[i + 1]);
                if (f < 0) {
                    cout << "\n" << "-sigmainits must be non-negative\n\n";
                    return 0;
                }
                sigma_init_s = f;
                i++;
            } else if (strcmp(argv[i], "-sigmape") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                float f = strf(argv[i + 1]);
                if (f < 0) {
                    cout << "\n" << "-sigmape must be non-negative\n\n";
                    return 0;
                }
                sigma_pitch_e = f;
                i++;
            } else if (strcmp(argv[i], "-sigmare") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                float f = strf(argv[i + 1]);
                if (f < 0) {
                    cout << "\n" << "-sigmare must be non-negative\n\n";
                    return 0;
                }
                sigma_rhythm_e = f;
                i++;
            } else if (strcmp(argv[i], "-psame") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                float f = strf(argv[i + 1]);
                if ((f < 0) || (f > 1)) {
                    cout << "\n" << "-psame must be between 0 and 1\n\n";
                    return 0;
                }
                p_noedit = f;
                i++;
            } else if (strcmp(argv[i], "-pjoin") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                float f = strf(argv[i + 1]);
                if ((f < 0) || (f > 1)) {
                    cout << "\n" << "-pjoin must be between 0 and 1\n\n";
                    return 0;
                }
                p_join = f;
                i++;
            } else if (strcmp(argv[i], "-output") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                output_file = argv[i + 1];
                i++;
            } else if (strcmp(argv[i], "-user") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                s_user = argv[i + 1];
                i++;
            } else if (strcmp(argv[i], "-password") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                s_password = argv[i + 1];
                i++;
            } else if (strcmp(argv[i], "-database") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                s_database = argv[i + 1];
                i++;
            } else if (strcmp(argv[i], "-host") == 0) {
                if (i == argc - 1) {
                    cout << usage;
                    return 0;
                }
                s_host = argv[i + 1];
                i++;
            } else if (strcmp(argv[i], "-qbsh") == 0) {
                qbsh = true;
            } else if (strcmp(argv[i], "-q") == 0) {
                quiet = true;
            } else {
                if (strncmp(argv[i], "-", 1) == 0) {
                    cout << "\n" << argv[i] << ": invalid option\n\n";
                    return 0;
                }
                break;
            }
        } catch(InvalidProgramArgument e) {
            cout << "\nInvalid program argument\n" << e.desc << "\n\n";
            return 0;
        }
    }
    if (i == argc) {
        cout << usage;
        return 0;
    }
    if (b_alignment == false) {
        if (i_algorithm == ALG_MS) {
            alignment = Local;
        } else { // ALG_JCS
            alignment = Start;
        }
    }
    if (alignment == Global) {
        k2 = 0;
    }
    if (k3 == -1) {
        k3 = k1;
    }
    if (p_noedit + p_join > 1) {
        cout << "\npsame and pjoin must sum at most 1\n\n";
        return 0;
    }
    unsigned int *query1 = (unsigned int*)parse(argv[i]);
    if (query1 == 0) {
        cout << "\n" << argv[i] << ": error parsing the query\n\n";
        return 0;
    }
    Sequence query(query1, *query1);
    if (verb(1)) cout << "query: " << query.print() << "\n";
    if (query.length() == 0) {
        return 0;
    }
    // create an instance of the algorithm
    Algorithm* algorithm;
    if (i_algorithm == ALG_MS) {
        algorithm = new MS(k1, k2, k3, F, C, ins_del, alignment, modulo12,
                           shortest_distance);
    } else { // if (i_algorithm == ALG_JCS)
        algorithm = new JCS(sigma_init_s, sigma_k, sigma_s, sigma_pitch_e, 
                            sigma_rhythm_e, p_noedit, p_join, alignment);
        query.mod12();
        query.quantize();
    }
    // Normalize alignment score
    // TODO Normalization as it's done only has meaning for M-S
    float min_score = k3 * query.totalDuration();
    Result r1 = algorithm->compare(query, query);
    float max_score = r1.score;
    if (i + 1 < argc) {
        // compare to a single document and exit
        unsigned int *document1 = (unsigned int*)parse(argv[i + 1]);
        if (document1 == 0) {
            cout << "\n" << argv[i + 1] << ": error parsing the document\n\n";
            return 0;
        }
        Sequence document(document1, *document1);
        if (verb(1)) cout << "document: " << document.print() << "\n\n";
        if (i_algorithm == ALG_JCS) {
            document.mod12();
            document.quantize();
        }
        if (document.length() != 0) {
            min_score += k3 * document.totalDuration();
            Result result = algorithm->compare(query, document);
            result.score = normalize(result.score, i_algorithm, min_score, 
                                     max_score, alignment);
            if (!quiet) {
                cout << "path: " << printSolution(result.solution) << "\n\n";
                cout << printAlignment(query, document, result) << "\n\n";
            }
            cout << result.score << "\n";
        }
        return 0;
    }
    // else, compare to every document in a collection
    PGconn* conn;
    string conn_string;
    if (s_user != NULL) {
        conn_string += "user=" + string(s_user) + " ";
    }
    if (s_password != NULL) {
        conn_string += "password=" + string(s_password) + " ";
    }
    if (s_database != NULL) {
        conn_string += "dbname=" + string(s_database) + " ";
    }
    if (s_host != NULL) {
        conn_string += "host=" + string(s_host);
    }
    conn = PQconnectdb(conn_string.c_str());
    if (PQstatus(conn) != CONNECTION_OK) {
        cout << "Error connecting to the database server\n";
        cout << PQerrorMessage(conn) << "\n";
        return 0;
    }
    //string sql_query = "SELECT composer,title,filename,content," 
    //    "to_char(number,'9999999') FROM document WHERE collection = '";
    //sql_query += collection + "'";
    string sql_query = "SELECT composer,title,filename,content," \
        "to_char(number,'9999999') FROM document";
    PGresult *documents;
    documents = PQexecParams(conn, sql_query.c_str(), 0, NULL, NULL, NULL, 0, 1);
    // (request tuples in binary format)
    ExecStatusType status = PQresultStatus(documents);
    if (status != PGRES_TUPLES_OK) {
        cout << "Error fetching the documents from the database: " << 
            PQresStatus(status) << "\n" ;
        cout << PQresultErrorMessage(documents) << "\n";
        return 0;
    }
    int n = PQntuples(documents);
    vector<Result> results; // matching documents
    results.reserve(k);
    for (int i = 0; i < n; i++) {
        // compare to every document
        unsigned int *document1;
        document1 = (unsigned int*)PQgetvalue(documents, i, ColContent);
        int size = PQgetlength(documents, i, ColContent);
        Sequence document(document1, size);
        if (document.length() == 0) {
            continue;
        }
        if (i_algorithm == ALG_JCS) {
            document.mod12();
            document.quantize();
        }
        min_score += (k2 - k3) * document.totalDuration();
        Result result = algorithm->compare(query, document);
        result.score = normalize(result.score, i_algorithm, min_score, 
                                 max_score, alignment);
        if ((i >= k) && (result.score <= results[k-1].score)) {
            // dismiss the result
            continue;
        }
        // else, save the result, keeping results ordered
        string filename = string(PQgetvalue(documents, i, ColFilename));
        string composer = string(PQgetvalue(documents, i, ColComposer));
        string title = string(PQgetvalue(documents, i, ColTitle));
        int number = strl(PQgetvalue(documents, i, ColNumber));
        result.filename = filename;
        result.id = number;
        result.composer = composer;
        result.title = title;
        vector<Result>::iterator pos = upper_bound(results.begin(), 
                                                   results.end(), result);
        //cout << "pos " << pos->score << " score " << result.score << "\n";
        if (pos >= results.end() && i >= k) {
            // dismiss the result
            continue;
        }
        if (i >= k) {
            results.pop_back();
        }
        results.insert(pos, result);
    }
    PQclear(documents);
    PQfinish(conn);
    // print the results
    stringstream s_results;
    for (vector<Result>::iterator iter = results.begin(); iter != results.end();
         iter++) {
        Result result = *iter;
        if (qbsh) {
            if (iter != results.begin()) {
                s_results << " ";
            }
            s_results << result.filename;
        } else if (extended) {
            s_results.width(40);
            s_results << left << trim(result.composer, 40) << "  ";
            s_results.width(40);
            s_results << left << trim(result.title, 40) << "  ";
            s_results.width(6);
            s_results << left << result.score << " ";
            s_results.width(0);
            s_results << result.filename << "  ";
            s_results << result.id << "\n";
        } else {
            s_results << result.filename << "\n";
        }
    }
    if (qbsh) {
        s_results << "\n";
    }
    if (output_file == NULL) {
        cout << s_results.str();
    } else {
        ofstream outfile(output_file);
        if (outfile) {
            outfile << s_results.str();
        } else {
            cout << output_file << ": the file could not be opened for writing\n";
        }
        outfile.close();
    }
    if (verb(1)) {
        cout << "\n";
        for (vector<Result>::iterator iter = results.begin(); 
             iter != results.end(); iter++) {
            cout << "path: " << printSolution((*iter).solution) << "\n";
        }
    }
    return 0;
}

void warning(char *msg) {
    cout << "Warning: " << msg << "\n";
}
