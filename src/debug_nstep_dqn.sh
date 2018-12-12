#!/bin/bash

g_type=barabasi_albert

result_root=results/dqn-$g_type

# max belief propagation iteration
max_bp_iter=5

# embedding size
embed_dim=64

# gpu card id
dev_id=0

# max batch size for training/testing
batch_size=64

net_type=QNet

# set reg_hidden=0 to make a linear regression
reg_hidden=64

# learning rate
learning_rate=0.0001

# init weights with rand normal(0, w_scale)
w_scale=0.01

# nstep
n_step=2

min_n=15
max_n=20

min_w=1
max_w=100

num_env=1
mem_size=500000

max_iter=10000

use_randvc=1

# folder to save the trained model
save_dir=$result_root/embed-$embed_dim-nbp-$max_bp_iter-rh-$reg_hidden

if [ ! -e $save_dir ];
then
    mkdir -p $save_dir
fi

lldb -o r -- python2.7 main.py \
    -n_step $n_step \
    -dev_id $dev_id \
    -min_n $min_n \
    -max_n $max_n \
    -min_w $min_w \
    -max_w $max_w \
    -num_env $num_env \
    -max_iter $max_iter \
    -mem_size $mem_size \
    -g_type $g_type \
    -learning_rate $learning_rate \
    -max_bp_iter $max_bp_iter \
    -net_type $net_type \
    -max_iter $max_iter \
    -save_dir $save_dir \
    -embed_dim $embed_dim \
    -batch_size $batch_size \
    -reg_hidden $reg_hidden \
    -momentum 0.9 \
    -l2 0.00 \
    -w_scale $w_scale \
    -use_randvc $use_randvc\
    2>&1 | tee $save_dir/log-$min_n-${max_n}.txt
#    -load_model $save_dir/iter_5.model \
