All the compat check utils should be placed here.
These will be invoked by upgrade manager during START phase

Upgrade manager provides the below inputs
1. running_top_dir (-r <>)
2. new_topdir (-n <>)
3. pipeline (-p <>)
Everything else should be derived by the script/application

It should exit with 0 on success or 1 in failure
