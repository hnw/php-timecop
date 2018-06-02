<?php

/**
 * Time travel to specified timestamp and freeze
 *
 * @param DateTimeInterface|int $time
 *
 * @return bool
 */
function timecop_freeze($time) {}

/**
 * Time travel to specified timestamp
 *
 * @param DateTimeInterface|int $time
 *
 * @return bool
 */
function timecop_travel($time) {}

/**
 * Accelerate time with specified scale
 *
 * @param float $scale
 *
 * @return bool
 */
function timecop_scale($scale) {}

/**
 * Return from time travel to current time
 *
 * @return bool
 */
function timecop_return() {}

class Timecop
{
    /**
     * Time travel to specified timestamp and freeze
     *
     * @param DateTimeInterface|int $time
     *
     * @return bool
     */
    public static function freeze($time) {}

    /**
     * Time travel to specified timestamp
     *
     * @param DateTimeInterface|int $time
     *
     * @return bool
     */
    public static function travel($time) {}

    /**
     * Accelerate time with specified scale
     *
     * @param float $scale
     *
     * @return bool
     */
    public static function scale($scale) {}

    /**
     * Return from time travel to current time
     *
     * @return bool
     */
    public static function return() {}
}

class TimecopDateTime extends DateTime {}

class TimecopOrigDateTime extends DateTime {}

class TimecopDateTimeImmutable extends DateTimeImmutable {}

class TimecopOrigDateTimeImmutable extends DateTimeImmutable {}
