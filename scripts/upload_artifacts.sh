#!/bin/sh

tar -czf /tmp/artifacts.tgz ~/
artifacts upload --target-paths "${TRAVIS_JOB_NUMBER}"  /tmp/artifacts.tgz

