#!/bin/bash

find * -type f -print0 | xargs -0 chmod +x
