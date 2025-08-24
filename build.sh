#!/bin/bash

g++ -std=c++20 shm_test.cpp -lpthread -lssl -lcrypto -o shm_test
