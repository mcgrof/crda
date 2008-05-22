use regulatory;
/*
        reg_country_id          int(11) UNSIGNED NOT NULL AUTO_INCREMENT,
        alpha2                  varchar(2) not null,
        reg_collection_id       int(11) UNSIGNED not null default '0',
        PRIMARY KEY(reg_country_id)
*/

/* DMN_NULL_WORLD */
insert into reg_country (alpha2, reg_collection_id) values ("00", 1);
/* DMN_NULL_DEBUG*/
insert into reg_country (alpha2, reg_collection_id) values ("01", 2);
/* DMN_DEBUG_NULL */
insert into reg_country (alpha2, reg_collection_id) values ("02", 3);
/* DMN_DEBUG_DEBUG */
insert into reg_country (alpha2, reg_collection_id) values ("03", 4);

/* DMN_NULL_ETSIC */
insert into reg_country (alpha2, reg_collection_id) values ("BR", 5);
insert into reg_country (alpha2, reg_collection_id) values ("BZ", 5);

/* DMN_FCC1_WORLD */
insert into reg_country (alpha2, reg_collection_id) values ("PH", 6);

/* DMN_FCC1_FCCA */
insert into reg_country (alpha2, reg_collection_id) values ("CO", 7);
insert into reg_country (alpha2, reg_collection_id) values ("DO", 7);
insert into reg_country (alpha2, reg_collection_id) values ("GT", 7);
insert into reg_country (alpha2, reg_collection_id) values ("MX", 7);
insert into reg_country (alpha2, reg_collection_id) values ("PA", 7);
insert into reg_country (alpha2, reg_collection_id) values ("PR", 7);
insert into reg_country (alpha2, reg_collection_id) values ("US", 7);

/* DMN_FCC2_WORLD */
insert into reg_country (alpha2, reg_collection_id) values ("AU", 8);
insert into reg_country (alpha2, reg_collection_id) values ("HK", 8);
insert into reg_country (alpha2, reg_collection_id) values ("MO", 8);

/* DMN_FCC2_FCCA */
insert into reg_country (alpha2, reg_collection_id) values ("CA", 9);

/* DMN_FCC2_ETSIC */
insert into reg_country (alpha2, reg_collection_id) values ("NZ", 10);

/* DMN_FCC3_FCCA */
insert into reg_country (alpha2, reg_collection_id) values ("UZ", 11);

/* DMN_ETSI1_WORLD */
insert into reg_country (alpha2, reg_collection_id) values ("CY", 12);
insert into reg_country (alpha2, reg_collection_id) values ("DE", 12);
insert into reg_country (alpha2, reg_collection_id) values ("DK", 12);
insert into reg_country (alpha2, reg_collection_id) values ("EE", 12);
insert into reg_country (alpha2, reg_collection_id) values ("ES", 12);
insert into reg_country (alpha2, reg_collection_id) values ("FI", 12);
insert into reg_country (alpha2, reg_collection_id) values ("GB", 12);
insert into reg_country (alpha2, reg_collection_id) values ("IE", 12);
insert into reg_country (alpha2, reg_collection_id) values ("IS", 12);
insert into reg_country (alpha2, reg_collection_id) values ("IT", 12);
insert into reg_country (alpha2, reg_collection_id) values ("LT", 12);
insert into reg_country (alpha2, reg_collection_id) values ("LU", 12);
insert into reg_country (alpha2, reg_collection_id) values ("LN", 12);
insert into reg_country (alpha2, reg_collection_id) values ("NO", 12);
insert into reg_country (alpha2, reg_collection_id) values ("PL", 12);
insert into reg_country (alpha2, reg_collection_id) values ("PT", 12);
insert into reg_country (alpha2, reg_collection_id) values ("SE", 12);
insert into reg_country (alpha2, reg_collection_id) values ("SI", 12);
insert into reg_country (alpha2, reg_collection_id) values ("UK", 12);
insert into reg_country (alpha2, reg_collection_id) values ("ZA", 12);

/* DMN_ETSI2_WORLD */
insert into reg_country (alpha2, reg_collection_id) values ("CH", 13);
insert into reg_country (alpha2, reg_collection_id) values ("HU", 13);
insert into reg_country (alpha2, reg_collection_id) values ("LI", 13);
insert into reg_country (alpha2, reg_collection_id) values ("AT", 13);

/* DMN_ETSI3_WORLD */
insert into reg_country (alpha2, reg_collection_id) values ("CZ", 14);
insert into reg_country (alpha2, reg_collection_id) values ("F2", 14); /* ? */
insert into reg_country (alpha2, reg_collection_id) values ("HR", 14);
insert into reg_country (alpha2, reg_collection_id) values ("SK", 14);
insert into reg_country (alpha2, reg_collection_id) values ("TN", 14);
insert into reg_country (alpha2, reg_collection_id) values ("TR", 14);
insert into reg_country (alpha2, reg_collection_id) values ("AM", 14);
insert into reg_country (alpha2, reg_collection_id) values ("AZ", 14);
insert into reg_country (alpha2, reg_collection_id) values ("BE", 14);
insert into reg_country (alpha2, reg_collection_id) values ("GE", 14);
insert into reg_country (alpha2, reg_collection_id) values ("MC", 14);
insert into reg_country (alpha2, reg_collection_id) values ("TT", 14);

/* DMN_ETSI6_WORLD */
insert into reg_country (alpha2, reg_collection_id) values ("BG", 15);

/* DMN_MKK1_MKKA */
insert into reg_country (alpha2, reg_collection_id) values ("JP", 16);
/* DMN_MKK1_FCCA */
insert into reg_country (alpha2, reg_collection_id) values ("J1", 17);
/* DMN_MKK2_MKKA */
insert into reg_country (alpha2, reg_collection_id) values ("J2", 18);

/* DMN_APL1_WORLD */
insert into reg_country (alpha2, reg_collection_id) values ("BN", 19);
insert into reg_country (alpha2, reg_collection_id) values ("CH", 19);
insert into reg_country (alpha2, reg_collection_id) values ("IR", 19);
insert into reg_country (alpha2, reg_collection_id) values ("CL", 19);

/* DMN_APL1_ETSIC */
insert into reg_country (alpha2, reg_collection_id) values ("BO", 20);

/* DMN_APL2_WORLD */
insert into reg_country (alpha2, reg_collection_id) values ("KP", 21);
insert into reg_country (alpha2, reg_collection_id) values ("KR", 21);
insert into reg_country (alpha2, reg_collection_id) values ("TH", 21);
insert into reg_country (alpha2, reg_collection_id) values ("UY", 21);

/* DMN_APL2_ETSIC */
insert into reg_country (alpha2, reg_collection_id) values ("VE", 22);

/* DMN_APL3_WORLD */
insert into reg_country (alpha2, reg_collection_id) values ("AR", 23);
insert into reg_country (alpha2, reg_collection_id) values ("TW", 23);

/* DMN_APL4_WORLD */
insert into reg_country (alpha2, reg_collection_id) values ("SG", 24);


