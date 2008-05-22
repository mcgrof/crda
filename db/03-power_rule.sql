use regulatory;
/*        environment_cap         varchar(255) not null,
        max_antenna_gain_mbi   int(11) UNSIGNED not null default '0',
        max_ir_ptmp_mbm        int(11) UNSIGNED not null default '0',
        max_ir_ptp_mbm         int(11) UNSIGNED not null default '0',
        max_eirp_pmtp_mbm      int(11) UNSIGNED not null default '0',
        max_eirp_ptp_mbm       int(11) UNSIGNED not null default '0'
*/
insert into power_rule (environment_cap, max_antenna_gain_mbi, max_ir_ptmp_mbm, max_ir_ptp_mbm, max_eirp_pmtp_mbm, max_eirp_ptp_mbm) values (" ", 6 * 1000, 30 * 1000, 30 * 1000, 36 * 1000, 255 * 1000);
