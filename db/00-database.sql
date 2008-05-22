/* create database */

create database if not exists regulatory;

/* add permissions */
use regulatory;
grant all privileges on regulatory.* to admin@localhost identified by '802.11r';
