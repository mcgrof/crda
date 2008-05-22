use regulatory;
/* DMN_WORLD */
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (2432 * 1000, 2442 * 1000, 20 * 1000, 3);

/* DMN_FCCA */
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (2412 * 1000, 2462 * 1000, 20 * 1000, 3);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (2437 * 1000, 2437 * 1000, 40 * 1000, 3);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (2442 * 1000, 2462 * 1000, 20 * 1000, 3);

/* DMN_ETSIC */
/* first two freq ranges of DMN_FCCA and: */
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (2442 * 1000, 2472 * 1000, 20 * 1000, 3);

/* DMN_MKKA -- 1 freq ranges! */
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (2412 * 1000, 2472 * 1000, 20 * 1000, 3);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (2484 * 1000, 2484 * 1000, 20 * 1000, 1);

/* DMN_DEBUG */
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (2412 * 1000, 2732 * 1000, 40 * 1000, 3);
/* There's nother 5 GHz debug below */

/************     5 GHz freq ranges now    ********************/

/* DMN_APL1 */
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5745 * 1000, 5825 * 1000, 20 * 1000, 2);

/* DMN_APL2 */
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5745 * 1000, 5805 * 1000, 20 * 1000, 2);

/* DMN_APL3 */
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5280 * 1000, 5320 * 1000, 20 * 1000, 2);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5745 * 1000, 5805 * 1000, 20 * 1000, 2);

/* DMN_APL4 --- DMN_ETSI2 uses only the first frequency range */
/* DMN_ETS5 uses the same freq range as DMN_ETSI2 so we can combine these two */
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5180 * 1000, 5240 * 1000, 20 * 1000, 2);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5745 * 1000, 5825 * 1000, 20 * 1000, 2);

/* DMN_ETSI1 -- DMN_ETSI3 and DMN_ETSI4 uses the first freq range of DMN_ETSI1 */
/* DMN_ETSI4 overlaps with ETSI3 so we deleted it */
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5180 * 1000, 5320 * 1000, 20 * 1000, 2);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5500 * 1000, 5700 * 1000, 20 * 1000, 2);

/* DMN_ETSI6 */
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5180 * 1000, 5280 * 1000, 20 * 1000, 2);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5500 * 1000, 5700 * 1000, 20 * 1000, 2);

/* DMN_FCC1 */
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5180 * 1000, 5200 * 1000, 20 * 1000, 2);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5210 * 1000, 5210 * 1000, 40 * 1000, 2);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5220 * 1000, 5240 * 1000, 20 * 1000, 2);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5250 * 1000, 5250 * 1000, 40 * 1000, 2);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5260 * 1000, 5280 * 1000, 20 * 1000, 2);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5290 * 1000, 5290 * 1000, 40 * 1000, 2);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5300 * 1000, 5320 * 1000, 20 * 1000, 2);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5745 * 1000, 5745 * 1000, 20 * 1000, 2);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5760 * 1000, 5760 * 1000, 40 * 1000, 2);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5765 * 1000, 5785 * 1000, 20 * 1000, 2);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5800 * 1000, 5800 * 1000, 40 * 1000, 2);
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5805 * 1000, 5825 * 1000, 20 * 1000, 2);

/* DMN_FCC2 -- the first freq is from ETSI1 (5180 - 5320), the other one is the second one from DMN_APL4 (5745 - 5825) */

/* DMN_FCC3 is DMN_FCC1 + the freqs below */
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5500 * 1000, 5700 * 1000, 20 * 1000, 2);

/* DMN_MKK1 */
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5170 * 1000, 5230 * 1000, 20 * 1000, 2);

/* DMN_MKK2 is DMN_MKK1 + the freqs below */
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5040 * 1000, 5080 * 1000, 20 * 1000, 2);

/* DMN_DEBUG */
insert into freq_range (start_freq_khz, end_freq_khz, max_bandwidth_khz, modulation_cap) values (5005 * 1000, 6100 * 1000, 40 * 1000, 3);
