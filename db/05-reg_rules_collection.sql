use regulatory;
/*
        entry_id                int(11) UNSIGNED NOT NULL AUTO_INCREMENT,
        reg_collection_id       int(11) UNSIGNED not null default '0',
        reg_rule_id             int(11) UNSIGNED not null default '0',
*/

/* Global collection of rules */

/* DMN_NULL_WORLD */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (1, 1);

/* DMN_NULL_DEBUG */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (2, 8);

/* DMN_DEBUG_NULL */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (3, 34);

/* DMN_DEBUG_DEBUG */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (4, 8);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (4, 34);

/* DMN_NULL_ETSIC */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (5, 2);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (5, 3);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (5, 5);

/* DMN_FCC1_WORLD */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (6, 19);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (6, 20);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (6, 21);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (6, 22);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (6, 23);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (6, 24);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (6, 25);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (6, 26);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (6, 27);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (6, 28);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (6, 29);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (6, 30);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (6, 1);

/* DMN_FCC2_WORLD */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (7, 15);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (7, 14);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (7, 1);

/* DMN_FCC2_FCCA */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (8, 2);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (8, 3);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (8, 4);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (8, 15);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (8, 14);

/* DMN_FCC2_ETSIC */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (9, 2);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (9, 3);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (9, 4);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (9, 2);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (9, 3);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (9, 5);

/* DMN_FCC3_FCCA */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (10, 19);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (10, 20);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (10, 21);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (10, 22);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (10, 23);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (10, 24);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (10, 25);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (10, 26);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (10, 27);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (10, 28);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (10, 29);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (10, 30);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (10, 31);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (10, 15);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (10, 14);

/* DMN_ETSI1_WORLD */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (11, 15);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (11, 16);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (11, 1);

/* DMN_ETSI2_WORLD */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (12, 13);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (12, 1);

/* DMN_ETSI3_WORLD */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (13, 15);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (13, 1);

/* DMN_ETSI6_WORLD */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (14, 17);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (14, 18);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (14, 1);

/* DMN_MKK1_MKKA */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (15, 32);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (15, 6);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (15, 7);

/* DMN_MKK1_FCCA */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (16, 32);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (16, 2);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (16, 3);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (16, 4);

/* DMN_MKK2_MKKA */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (18, 32);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (18, 33);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (18, 6);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (18, 7);

/* DMN_APL1_WORLD */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (19, 9);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (19, 1);

/* DMN_APL1_ETSIC */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (20, 9);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (20, 2);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (20, 3);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (20, 5);

/* DMN_APL2_WORLD */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (21, 10);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (21, 1);

/* DMN_APL2_ETSIC */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (22, 10);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (22, 2);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (22, 3);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (22, 5);

/* DMN_APL3_WORLD */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (23, 11);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (23, 12);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (23, 1);

/* DMN_APL4_WORLD */
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (24, 13);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (24, 14);
insert into reg_rules_collection (reg_collection_id, reg_rule_id) values (24, 1);
