CREATE TABLE 03246_range_literal_replacement_works (id UInt8) Engine=Memory;

INSERT INTO 03246_range_literal_replacement_works VALUES (1 BETWEEN 0 AND 2);

SELECT * FROM 03246_range_literal_replacement_works;
