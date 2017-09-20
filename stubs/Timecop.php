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
 * @param float $scale
 */
function timecop_scale($scale) {}

/**
 * Return from time travel to current time
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
     * @param float $scale
     */
    public static function scale($scale) {}

    /**
     * Return from time travel to current time
     */
    public static function return() {}
}

class TimecopDateTime extends DateTime {}

class TimecopOrigDateTime extends DateTime {}

class TimecopDateTimeImmutable extends DateTimeImmutable {}

class TimecopOrigDateTimeImmutable extends DateTimeImmutable {}
