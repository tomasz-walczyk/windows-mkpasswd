#
# Copyright (C) 2019 Tomasz Walczyk
#
# This file is subject to the terms and conditions defined
# in 'LICENSE' file which is part of this source code package.
#
###########################################################

readonly min_rounds=1000
readonly max_rounds=10000
readonly rounds_step=1000

readonly min_salt_len=8
readonly max_salt_len=16
readonly salt_len_step=1

readonly min_password_len=1
readonly max_password_len=127
readonly password_len_step=1

readonly output_file="test-data.txt"

###########################################################

[[ -e "${output_file}" ]] && rm "${output_file}"

{
	samples=$(( (${max_rounds} - ${min_rounds}) / ${rounds_step} ))
	samples=$(( ${samples} * (${max_salt_len} - ${min_salt_len}) / ${salt_len_step} ))
	samples=$(( ${samples} * (${max_password_len} - ${min_password_len}) / ${password_len_step} ))
	
	counter=0
	for (( rounds=${min_rounds}; rounds<=${max_rounds}; rounds+=${rounds_step} )); do
	  for (( salt_len=${min_salt_len}; salt_len<=${max_salt_len}; salt_len+=${salt_len_step} )); do
		for (( password_len=${min_password_len}; password_len<=${max_password_len}; password_len+=${password_len_step} )); do
		  [[ ${salt_len} -ne 0 ]] && salt=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w ${salt_len} | head -n 1) || salt=""
		  [[ ${password_len} -ne 0 ]] && password=$(cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w ${password_len} | head -n 1) || password=""
		  hash=$(echo "${password}" | mkpasswd --stdin --method=sha-512 --rounds=${rounds} --salt="${salt}" 2> /dev/null)
		  [[ $? -eq 0 ]] || hash=""
		  echo "${password};${rounds};${salt};${hash}" >> "${output_file}"
		  counter=$(( ${counter} + 1 ))
		  progress=$(( ${counter} * 100  / ${samples} ))
		  echo "${progress}"
		done
	  done
	done
} | whiptail --gauge "Generating test data..." 6 60 0
