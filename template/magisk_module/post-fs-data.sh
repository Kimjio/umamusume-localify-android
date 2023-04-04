#!/system/bin/sh
while read -r line; do echo "${line}" | grep jp.co.cygames.umamusume | awk '{print $2}' | xargs umount -l; done </proc/mounts
while read -r line; do echo "${line}" | grep com.kakaogames.umamusume | awk '{print $2}' | xargs umount -l; done </proc/mounts
while read -r line; do echo "${line}" | grep com.komoe.kmumamusumegp | awk '{print $2}' | xargs umount -l; done </proc/mounts
while read -r line; do echo "${line}" | grep com.komoe.kmumamusumemc | awk '{print $2}' | xargs umount -l; done </proc/mounts
