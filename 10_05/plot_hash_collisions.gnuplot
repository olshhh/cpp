set datafile separator ','
set terminal pngcairo size 1600,900
set output 'hash_collisions.png'
set title 'Hash collisions vs number of hashed strings'
set xlabel 'Number of hashed strings'
set ylabel 'Collisions'
set grid
plot 'hash_collisions.csv' using 1:2 with lines lw 2 title 'RSHash', 'hash_collisions.csv' using 1:3 with lines lw 2 title 'JSHash', 'hash_collisions.csv' using 1:4 with lines lw 2 title 'PJWHash', 'hash_collisions.csv' using 1:5 with lines lw 2 title 'ELFHash', 'hash_collisions.csv' using 1:6 with lines lw 2 title 'BKDRHash', 'hash_collisions.csv' using 1:7 with lines lw 2 title 'SDBMHash', 'hash_collisions.csv' using 1:8 with lines lw 2 title 'DJBHash', 'hash_collisions.csv' using 1:9 with lines lw 2 title 'DEKHash', 'hash_collisions.csv' using 1:10 with lines lw 2 title 'APHash'
