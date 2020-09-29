#!/bin/bash

set -e

function scheme() {
    expected_string=$1
    echo "$expected_string" | ./../build/src/scheme
}

function test_scheme() {
    expected_string=$1
    expected_error=${2:-heh}
    if scheme "$expected_string" |& grep -q "> $expected_string"; then
        echo $expected_string passed
    elif scheme "$expected_string" |& grep -q "> $expected_error"; then
        echo $expected_string passed
    else
        echo $expected_string failed
        exit 1
    fi
}

test_scheme "-123"
test_scheme "123"
test_scheme "#t"
test_scheme "#f"
test_scheme "fail" "Bad input!"
test_scheme "121aa" "Number not followed by delimiter"
