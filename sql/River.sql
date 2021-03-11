-- Database River
DROP DATABASE IF EXISTS River;
CREATE DATABASE River DEFAULT CHARACTER SET utf8 COLLATE utf8_general_ci;

USE River;
-- user table
DROP TABLE IF EXISTS user;
CREATE TABLE IF NOT EXISTS `user`(
   `ID` INT UNSIGNED AUTO_INCREMENT PRIMARY KEY COMMENT 'id increment',
   `GameID` INT UNSIGNED NOT NULL,
   `Account` VARCHAR(32) NOT NULL,
   `Pwd` VARCHAR(32) NOT NULL,
   `Nickname` VARCHAR(32) NOT NULL,
   `Gold` BIGINT NOT NULL DEFAULT 0 COMMENT 'user gold',
   `Diamond` INT NOT NULL DEFAULT 0,
   `RoomCard` INT NOT NULL DEFAULT 0,
   `Coupon` INT NOT NULL DEFAULT 0,
   `Experience` INT NOT NULL DEFAULT 0,
   `ImageID` TINYINT NOT NULL DEFAULT 0,
   `Sex` TINYINT NOT NULL DEFAULT 0 COMMENT '0 man:1 woman', 
   `RealName` VARCHAR(16),
   `Phone` VARCHAR(16),
   `Address` VARCHAR(64),
   `Postcode` VARCHAR(7),
   `QQ` VARCHAR(13),
   `Wechat` VARCHAR(20),
   `EMail` VARCHAR(20),
   `RegisterDate` DATE,
   `RegisterIP` VARCHAR(20),
   `RegisterMachine` VARCHAR(32),
   `LoginDate` DATE,
   `LoginIP` VARCHAR(20),
   `LoginMachine` VARCHAR(32)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT 'use basic info';