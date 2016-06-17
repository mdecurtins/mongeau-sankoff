-- musicseq.sql
-- Creates the musicseq user defined datatype to store sequences
-- of musical notes in PostgreSQL.
-- The functions referenced in this file can be found in secuencia.c.
-- author: Carlos Gómez

CREATE FUNCTION secuencia_in(cstring)
    RETURNS musicseq
    AS '$libdir/secuencia.so'
    LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION secuencia_out(musicseq)
    RETURNS cstring
    AS '$libdir/secuencia.so'
    LANGUAGE C IMMUTABLE STRICT;

CREATE FUNCTION secuencia_send(musicseq)
    RETURNS bytea
    AS '$libdir/secuencia.so'
    LANGUAGE C IMMUTABLE STRICT;

CREATE TYPE musicseq (
    internallength = VARIABLE,
    input = secuencia_in,
    output = secuencia_out,
    send = secuencia_send,
    alignment = int4,
    storage = plain
);

