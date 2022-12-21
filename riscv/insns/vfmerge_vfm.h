// vfmerge_vf vd, vs2, vs1, vm
VI_VF_MERGE_LOOP({
  vd = use_first ? rs1 : vs2;
})
P.get_state()->mhpmcounter[10]->bump(1);
