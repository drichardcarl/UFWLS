PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE contacts(idx integer primary key, contactName text not null, contactNumber text not null);
COMMIT;
