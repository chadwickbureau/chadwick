#!/bin/bash

function process_year {
  # Call syntax: process_year year switch
  # where 'switch' is -X or -S for XML or SportsML output
  echo -n "Checking ${1}...        "
  cd ~/git/retrosheet/event/regular
  if [ ! -e TEAM${1} ]; then
    echo "[OK]     No teamfile found, skipping"
    return
  fi
  if ! ls -1 ${1}*.EV? >/dev/null 2>&1; then
    echo "[OK]     No event files found, skipping"
    return
  fi
  output=$(cwbox -q -y ${1} ${2} ${1}*.EV? 2>&1 >/dev/null)
  status=$?
  if [ ${status} -ne 0 ]; then
    echo "[FAIL]   Return code of call was not zero"
    echo "Contents of standard error from call:"
    echo ${output}
  else
    echo "[OK]     Output successful"
  fi
}

for year in `seq 2012 -1 1871`
do
process_year ${year} -S
done
