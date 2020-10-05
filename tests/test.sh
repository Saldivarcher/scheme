#!/bin/bash

set -e

RED=`tput setaf 1`
GREEN=`tput setaf 2`

function scheme() {
    expected_string=$1
    echo "$expected_string" | ./../build/src/scheme
}

function test_scheme() {
    expected_string=$1
    expected_error=${2:-heh}
    if scheme "$expected_string" |& grep -q "> $expected_string"; then
        echo ${GREEN}$expected_string passed
    elif scheme "$expected_string" |& grep -q "> $expected_error"; then
        echo ${GREEN}$expected_string passed
    else
        echo ${RED}$expected_string failed
        exit 1
    fi
}

test_scheme "123"
test_scheme "-123"
test_scheme "#t"
test_scheme "#f"
test_scheme "#\\space" "#\\\\space"
test_scheme "#\\newline" "#\\\\newline"
test_scheme "#\\a" "#\\\\a"
test_scheme "fail" "Bad input!"
test_scheme "121aa" "Number not followed by delimiter"
test_scheme "#\\asdfsa" "Invalid character!"
