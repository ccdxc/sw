- commit() is actually progam_hw()
  - abort() can be called if something fails during this
- post_commit() is actually enable_epoch()/commit()
  - in post_commit() only stage 0 vnic related tables will be programmed
  - all other tables with latest epoch etc. are programmed in program_config()
  - post_commit() should be guaranteed to NOT fail (ever !!!), and abort() will
    not be called because some of the vnic's already would have moved to new
    epoch if this fails in the middle and there is no going back at this point
- post_commit() is when we can clear dirty flag on the objs and swap new obj
  with old obj in case of object updates (but this happens before enabling
  epoch in the h/w)
