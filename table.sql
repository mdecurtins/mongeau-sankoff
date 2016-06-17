CREATE TABLE document (
    composer text,
    title text,
    filename text PRIMARY KEY,
    collection text,
    content musicseq NOT NULL,
    number SERIAL UNIQUE
);
