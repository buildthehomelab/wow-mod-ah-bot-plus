-- Buyer.ProgressionPricing.DailyLimit: BiS units the bot bought or bid on per house and
-- item on one UTC day (day = unix time / 86400), so the limit survives restarts.
CREATE TABLE IF NOT EXISTS `mod_ahbot_bis_daily_buys` (
  `house` INT UNSIGNED NOT NULL,
  `item` INT UNSIGNED NOT NULL,
  `day` BIGINT NOT NULL,
  `units` INT UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`house`, `item`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
